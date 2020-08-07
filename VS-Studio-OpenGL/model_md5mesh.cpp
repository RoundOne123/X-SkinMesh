#include "model_md5mesh.h"

using namespace xTools;

namespace xMD5Loader
{

#pragma region MD5_Model

	MD5_Model::MD5_Model()
	{

	}

	MD5_Model::~MD5_Model()
	{

	}

	// 加载模型
	bool MD5_Model::LoadModel(string &path)
	{
		// 判断文件类型
		if (path.substr(path.size() - 8, 8) != ".md5mesh")
		{
			cout << "Only Load md5mesh Model " << path << endl;
		}

		// 打开指定文件
		ifstream file(path);
		if (!file.is_open())
		{
			cout << "open file error " << path << endl;
		}

		string param;		// 关键字 字符
		string junk;		// 用不到的字符

		jointList.clear();
		meshList.clear();

		int length = GetFileLength(file);	// 获取文件的长度 字节长度吧
		file >> param;	// file 读入到 param
		// 为什么这里没有再使用 getline(file, curline) 这个方法
		// file.eof() 判断文件是否为空，或者是否读取到文件结尾了
		while (!file.eof())
		{
			//cout << "param : " << param << endl;
			// 忽略"MD5Version"和"commandline"开头的这两行
			if (param == "MD5Version" || param == "commandline")
			{
				IgnoreLine(file, length);
			}

			if (param == "numJoints")					// 骨骼数量
			{
				file >> numJoints;
				// 预留骨骼数量的空间，并没有创建对象，注意和resize的区别
				jointList.reserve(numJoints);
			}
			else if (param == "numMeshs")				// 网格数量
			{
				file >> numMeshs;
				meshList.reserve(numMeshs);
			}
			else if (param == "joints")					// 处理骨骼
			{
				M_Joint joint;
				file >> junk;	// 前面的大括号
				for (int i = 0; i < numJoints; i++)
				{
					// 分别对应
					//     "origin"	         -1               (       -0.000000       0.016430     -0.006044        )   (     0.707107       0.000000        0.707107          )
					file >> joint.name >> joint.parent_ID >> junk >> joint.pos.x >> joint.pos.y >> joint.pos.z >> junk >> junk >> joint.orient.x >> joint.orient.y >> joint.orient.z >> junk;
					IgnoreLine(file, length);		// 忽略当前行后面的位置
					RemoveNotes(joint.name);		// 去掉 "" 引号
					ComputeQuatW(joint.orient);		// 计算旋转的w分量

					//!!!!!!!!!!!!!!!!!!!!!!!
					jointList.push_back(joint);
				}
				file >> junk;	// 后面的大括号
			}
			else if (param == "mesh")					// 处理网格
			{
				M_Mesh mesh;
				int numVert, numTris, numWeight;
				file >> junk;
				file >> param;
				while (param != "}")
				{
					if (param == "shader")				// mesh使用的纹理路径
					{
						file >> mesh.shader;			// 纹理名
						RemoveNotes(mesh.shader);		// 去掉 ""
						string texPath = GetSameDirFile(path, mesh.shader);		// 最终纹理的路径
						if (texPath.substr(texPath.size() - 4, 4) != ".tga")	// 纹理
						{
							texPath += ".tga";
						}
						// 加载纹理并生成纹理缓冲对象
						mesh.texID = LoadTexture(texPath);
						IgnoreLine(file, length);
					}
					else if (param == "numverts")		// 顶点数量
					{
						file >> numVert;
						IgnoreLine(file, length);
						// 处理顶点数据
						for (int i = 0; i < numVert; i++)
						{
							M_Vertex vert;
							file >> junk >> junk >> junk >> vert.texcoord.x >> vert.texcoord.y >> junk >> vert.startWeight >> vert.weightCount;
							IgnoreLine(file, length);

							//!!!!!!!!!!!!!!!!!!!!!!!
							mesh.verts.push_back(vert);
						}
					}
					else if (param == "numtris")			// 三角形数量
					{
						file >> numTris;
						IgnoreLine(file, length);
						// 处理顶点索引数据
						for (int i = 0; i < numTris; i++)
						{
							int indices[3];
							file >> junk >> junk >> indices[0] >> indices[1] >> indices[2];
							IgnoreLine(file, length);
							
							// !!!!!!!!!!!!!!!!!!!!!!!
							mesh.indexBuffer.push_back(indices[0]);	// (GL_INT)
							mesh.indexBuffer.push_back(indices[1]);
							mesh.indexBuffer.push_back(indices[2]);
						}
					}
					else if (param == "numweights")		// 权重数量
					{
						file >> numWeight;
						IgnoreLine(file, length);
						// 处理权重数据
						for (int i = 0; i < numWeight; i++)
						{
							M_Weight weight;
							file >> junk >> junk >> weight.joint_ID >> weight.bias >> junk >> weight.pos.x >> weight.pos.y >> weight.pos.z;
							IgnoreLine(file, length);

							//!!!!!!!!!!!!!!!!!!!!!!!
							mesh.weights.push_back(weight);
						}
					}
					else								// 忽略其他情况
					{
						IgnoreLine(file, length);
					}
					file >> param;
				}
				// 创建数组缓冲对象、顶点缓冲对象和索引缓冲对象
				CreateVertBuffer(mesh);

				//!!!!!!!!!!!!!!!!!!!!!!!
				meshList.push_back(mesh);
			}
			// >> 读取到一个符合要求的值之后，再次 >> 则会读取下一个复合要求的值 
			// 空格隔开时被当作单独的一个
			file >> param;
		}
		return jointList.size() == numJoints && meshList.size() == numMeshs;
	}

	// 加载纹理并创建纹理对象
	unsigned int MD5_Model::LoadTexture(string &path)
	{
		int width, height, channel;
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &channel, NULL);
		if (!data)
		{
			cout << "Load texture fail path : " << path << endl;
			return 0;
		}

		int format;
		format = channel == 3 ? GL_RGB : GL_RGBA;
		unsigned int texture;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// 设置纹理环绕方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);

		return texture;
	}

	// 创建数组缓冲对象、顶点缓冲和索引缓冲
	void MD5_Model::CreateVertBuffer(M_Mesh &mesh)
	{
		glGenVertexArrays(1, &(mesh.VAO));
		glGenBuffers(1, &(mesh.VBO));
		glGenBuffers(1, &(mesh.EBO));
	}

	// 计算顶点在模型空间的位置 -> 放在Update函数中调用
	void MD5_Model::ComputerVertPos(M_Mesh &mesh, const MD5_Animation::X_FrameSkeleton& skeleton)
	{
		// 遍历mesh的所有顶点
		for (unsigned int i = 0; i < mesh.verts.size(); i++)
		{
			M_Vertex &vert = mesh.verts[i];
			vert.pos = glm::vec3(0);

			/*
				顶点在模型空间中的位置：
					顶点的位置由一个或者多个权重决定，
					权重在模型空间中的位置则是由与它相关联的骨骼所决定，
					也可以说每个顶点的位置由一个或者多个骨骼决定。

					权重可以看作是在某个骨骼空间坐标定义的一个虚拟点，
					该虚拟顶点会随着骨骼的坐标空间一起变换，
				顶点位置 = 一个或者多个虚拟顶点在模型空间中的位置乘以权重占比再累加得到的。
			*/

			// 遍历顶点关联的权重
			for (int j = 0; j < vert.weightCount; j++)
			{
				// 获得权重
				M_Weight &weight = mesh.weights[vert.startWeight + j];
				//// 获得权重关联得骨骼数据（默认的骨骼数据）
				//M_Joint &joint = jointList[weight.joint_ID];	// 这里是唯一的 就是指文件骨骼数据的由上到下的顺序
				// -> 使用当前时刻的骨骼数据 计算出顶点位置
				const MD5_Animation::X_SkeletonJoint& joint = skeleton.jointList[weight.joint_ID];

				// 声明矩阵
				glm::mat4 boneMatrix(1.0f);
				glm::mat4 boneTranstion = glm::translate(boneMatrix, joint.pos);
				glm::mat4 boneRotate = glm::mat4(joint.orient);

				boneMatrix = boneTranstion * boneRotate;	// 当前骨骼空间 到 模型空间得变换矩阵
				// 权重在模型空间的位置 = 该骨骼到模型空间变换矩阵 * 当前权重在关联骨骼的位置
				glm::vec3 weightPos = glm::vec3(boneMatrix * glm::vec4(weight.pos, 1.0));	// 计算权重在模型空间中的位置
				// glm::vec3 weightPos = joint.pos + joint.orient * weight.pos;	// 与上面计算方式等价
				vert.pos += weightPos * weight.bias;	// 加权计算
			}

			// !!!!!!!!!!!!!!!!!
			//mesh.posBuffer.push_back(vert.pos);
		}
	}

	// 走帧逻辑
	void MD5_Model::Update(float deltaTime)
	{
		animation.Update(deltaTime);
		const MD5_Animation::X_FrameSkeleton& skeleton = animation.GetSkeleton();

		for (unsigned int i = 0; i < meshList.size(); i++)
		{
			ComputerVertPos(meshList[i], skeleton);
		}
	}

	// 基于CPU的骨骼蒙皮动画，每一帧都需要将计算好的顶点位置从CPU发送到GPU 这一个过程是有性能消耗的
	// 在可编程渲染管线出来之前 我们只能这么做 
	// 但是可编程渲染管线的出现 使我们可以将骨骼蒙皮过程放到 GPU中实现，基于GPU的骨骼蒙皮动画
	// GPU的骨骼蒙皮动画，我们只要每帧发送所有骨骼的变换矩阵
	// 渲染模型（的所有mesh） ->  每帧都要重新将顶点属性的值从cpu传到gpu
	void MD5_Model::Render(xShader shader)
	{
		for (unsigned int i = 0; i < meshList.size(); i++)
		{
			M_Mesh mesh = meshList[i];

			// 顶点数组对象
			glBindVertexArray(mesh.VAO);

			// 顶点缓冲
			glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
			glBufferData(GL_ARRAY_BUFFER, mesh.verts.size() * sizeof(M_Vertex), &(mesh.verts[0]), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(M_Vertex), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(M_Vertex), (void*)offsetof(M_Vertex, texcoord));

			// 顶点索引缓冲
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer.size() * sizeof(unsigned int), &(mesh.indexBuffer[0]), GL_STATIC_DRAW);

			shader.setInt("diffTex", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh.texID);

			glDrawElements(GL_TRIANGLES, mesh.indexBuffer.size(), GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);
		}
	}
#pragma endregion


}