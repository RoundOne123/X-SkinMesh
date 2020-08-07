#include "model_md5mesh_gpu.h"


namespace xMD5Loader
{
	
	MD5_Model_GPU::MD5_Model_GPU()
	{
	}

	MD5_Model_GPU::~MD5_Model_GPU()
	{
	}

	// 计算绑定姿势的mesh的所有顶点的位置，并设置与顶点关联的骨骼索引和骨骼权重
	void MD5_Model_GPU::PrepareMesh(Mesh &mesh)
	{
		// 遍历所有的顶点
		for (unsigned int i = 0; i < mesh.verts.size(); i++)
		{
			Vertex& vert = mesh.verts[i];
			vert.pos = vec3(0);
			vert.boneIndexs = vec4(0);
			vert.boneWeights = vec4(0);

			// 遍历某一顶点的所有权重
			for (int j = 0; j < vert.weightCount; j++)
			{
				// 获得权重
				Weight& weight = mesh.weights[vert.startWeightIndex + j];
				// 获得权重对应的骨骼
				Joint& joint = jointList[weight.joint_ID];

				/*
					下面的计算方式 使用的是计算 动画文件中 每一帧骨骼的方式，和使用矩阵的方式是等价的
					最终的结果是，顶点在【模型空间】中的位置
				*/
				vec3 rotPos = joint.orient * weight.pos;
				vert.pos += (joint.pos + rotPos) * weight.bias;

				vert.boneWeights[j] = weight.bias;
				vert.boneIndexs[j] = weight.joint_ID;
			}
		}
	}

	// 计算绑定姿势时，各骨骼从 模型空间 到 骨骼空间 的变换矩阵
	void MD5_Model_GPU::BuildBindPose(JointList& jointList)
	{
		inverseBindPose.clear();

		// 迭代器吧...
		JointList::const_iterator iter = jointList.begin();
		while (iter != jointList.end())
		{
			const Joint& joint = (*iter);
			mat4 boneMatrix(1.0f);
			mat4 boneTranstion = translate(boneMatrix, joint.pos);
			mat4 boneRotate = mat4(joint.orient);

			// 针对一个空间坐标系 P，对该空间进行一个旋转 M ，再进行一个平移 T， 得到新的空间坐标系 C， 
			// 对于空间C内一点 V（x,y,z） ,V在P空间内的坐标为 
			//  V' = T * M  * V 
			// 这里C相当于 骨骼空间  P相当于模型空间
			boneMatrix = boneTranstion * boneRotate;		// 从骨骼空间 到 模型空间 的变换矩阵
			mat4 inverseBoneMatrix = inverse(boneMatrix);	// 从模型空间 到 骨骼空间 的变换矩阵
			inverseBindPose.push_back(inverseBoneMatrix);
			iter++;
		}
	}

	// 加载模型
	bool MD5_Model_GPU::LoadModel(string& path)
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
				Joint joint;
				file >> junk;	// 前面的大括号
				for (int i = 0; i < numJoints; i++)
				{
					file >> joint.name >> joint.parent_ID >> junk >> joint.pos.x >> joint.pos.y >> joint.pos.z >> junk >> junk >> joint.orient.x >> joint.orient.y >> joint.orient.z >> junk;
					IgnoreLine(file, length);		// 忽略当前行后面的位置
					RemoveNotes(joint.name);		// 去掉 "" 引号
					ComputeQuatW(joint.orient);		// 计算旋转的w分量

					//!!!!!!!!!!!!!!!!!!!!!!!
					jointList.push_back(joint);
				}
				file >> junk;	// 后面的大括号

				// 计算绑定姿势时，各骨骼从 模型空间 到 骨骼空间 的变换矩阵
				BuildBindPose(jointList);
			}
			else if (param == "mesh")					// 处理网格
			{
				Mesh mesh;
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
							Vertex vert;
							file >> junk >> junk >> junk >> vert.texcoord.x >> vert.texcoord.y >> junk >> vert.startWeightIndex >> vert.weightCount;
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
							Weight weight;
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
				// 计算绑定姿势的mesh的所有顶点的位置，并设置与顶点关联的骨骼索引和骨骼权重
				PrepareMesh(mesh);

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
	unsigned int MD5_Model_GPU::LoadTexture(string &path)
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
	void MD5_Model_GPU::CreateVertBuffer(Mesh& mesh)
	{
		glGenVertexArrays(1, &(mesh.VAO));
		glGenBuffers(1, &(mesh.VBO));
		glGenBuffers(1, &(mesh.EBO));

		// 基于GPU实现的骨骼动画，只需要将绑定姿势的顶点属性传递给GPU一次就行
		// 后面渲染时，每帧只要骨骼空间的传递变换矩阵就行
		glBindVertexArray(mesh.VAO);

		// 顶点缓冲
		glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
		glBufferData(GL_ARRAY_BUFFER, mesh.verts.size() * sizeof(Vertex), &(mesh.verts[0]), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneIndexs));

		// 顶点索引缓冲
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer.size() * sizeof(unsigned int), &(mesh.indexBuffer[0]), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	// 走帧逻辑 在opengl 渲染循环中调用
	void MD5_Model_GPU::Update(float deltaTime)
	{
		animation.Update(deltaTime);

		// 当前动画信息
		const MD5_GPU_Animation::FrameSkeleton& skeleton = animation.GetSkeleton();

		animatedBones.clear();
		for (int i = 0; i < numJoints; i++)
		{
			// 当前姿势的骨骼空间变化到模型空间 * 绑定姿势的模型空间到骨骼空间的变换矩阵
			// 这俩不应该拆开分别用吗 为什么要相乘呢？
			glm::mat4 matrix = skeleton.boneMatrixs[i] * inverseBindPose[i];
			animatedBones.push_back(matrix);
		}
	}

	// 渲染模型（的所有mesh）
	void MD5_Model_GPU::Render(xShader shader)
	{
		shader.setMat4List("boneMatrixs", animatedBones.size(), animatedBones[0]);
		for (unsigned int i = 0; i < meshList.size(); i++) 
		{
			Mesh mesh = meshList[i];
			glBindVertexArray(mesh.VAO);
			shader.setInt("diffTex", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh.texID);
			glDrawElements(GL_TRIANGLES, mesh.indexBuffer.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}



	//// 计算骨骼空间到模型空间的变换矩阵
	//void MD5_Model_GPU::ComputerMatrix(Mesh &mesh, const MD5_GPU_Animation::FrameSkeleton& skeleton)
	//{
	
	//}
}