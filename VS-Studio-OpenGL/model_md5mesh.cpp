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

	// ����ģ��
	bool MD5_Model::LoadModel(string &path)
	{
		// �ж��ļ�����
		if (path.substr(path.size() - 8, 8) != ".md5mesh")
		{
			cout << "Only Load md5mesh Model " << path << endl;
		}

		// ��ָ���ļ�
		ifstream file(path);
		if (!file.is_open())
		{
			cout << "open file error " << path << endl;
		}

		string param;		// �ؼ��� �ַ�
		string junk;		// �ò������ַ�

		jointList.clear();
		meshList.clear();

		int length = GetFileLength(file);	// ��ȡ�ļ��ĳ��� �ֽڳ��Ȱ�
		file >> param;	// file ���뵽 param
		// Ϊʲô����û����ʹ�� getline(file, curline) �������
		// file.eof() �ж��ļ��Ƿ�Ϊ�գ������Ƿ��ȡ���ļ���β��
		while (!file.eof())
		{
			//cout << "param : " << param << endl;
			// ����"MD5Version"��"commandline"��ͷ��������
			if (param == "MD5Version" || param == "commandline")
			{
				IgnoreLine(file, length);
			}

			if (param == "numJoints")					// ��������
			{
				file >> numJoints;
				// Ԥ�����������Ŀռ䣬��û�д�������ע���resize������
				jointList.reserve(numJoints);
			}
			else if (param == "numMeshs")				// ��������
			{
				file >> numMeshs;
				meshList.reserve(numMeshs);
			}
			else if (param == "joints")					// �������
			{
				M_Joint joint;
				file >> junk;	// ǰ��Ĵ�����
				for (int i = 0; i < numJoints; i++)
				{
					// �ֱ��Ӧ
					//     "origin"	         -1               (       -0.000000       0.016430     -0.006044        )   (     0.707107       0.000000        0.707107          )
					file >> joint.name >> joint.parent_ID >> junk >> joint.pos.x >> joint.pos.y >> joint.pos.z >> junk >> junk >> joint.orient.x >> joint.orient.y >> joint.orient.z >> junk;
					IgnoreLine(file, length);		// ���Ե�ǰ�к����λ��
					RemoveNotes(joint.name);		// ȥ�� "" ����
					ComputeQuatW(joint.orient);		// ������ת��w����

					//!!!!!!!!!!!!!!!!!!!!!!!
					jointList.push_back(joint);
				}
				file >> junk;	// ����Ĵ�����
			}
			else if (param == "mesh")					// ��������
			{
				M_Mesh mesh;
				int numVert, numTris, numWeight;
				file >> junk;
				file >> param;
				while (param != "}")
				{
					if (param == "shader")				// meshʹ�õ�����·��
					{
						file >> mesh.shader;			// ������
						RemoveNotes(mesh.shader);		// ȥ�� ""
						string texPath = GetSameDirFile(path, mesh.shader);		// ���������·��
						if (texPath.substr(texPath.size() - 4, 4) != ".tga")	// ����
						{
							texPath += ".tga";
						}
						// ���������������������
						mesh.texID = LoadTexture(texPath);
						IgnoreLine(file, length);
					}
					else if (param == "numverts")		// ��������
					{
						file >> numVert;
						IgnoreLine(file, length);
						// ����������
						for (int i = 0; i < numVert; i++)
						{
							M_Vertex vert;
							file >> junk >> junk >> junk >> vert.texcoord.x >> vert.texcoord.y >> junk >> vert.startWeight >> vert.weightCount;
							IgnoreLine(file, length);

							//!!!!!!!!!!!!!!!!!!!!!!!
							mesh.verts.push_back(vert);
						}
					}
					else if (param == "numtris")			// ����������
					{
						file >> numTris;
						IgnoreLine(file, length);
						// ��������������
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
					else if (param == "numweights")		// Ȩ������
					{
						file >> numWeight;
						IgnoreLine(file, length);
						// ����Ȩ������
						for (int i = 0; i < numWeight; i++)
						{
							M_Weight weight;
							file >> junk >> junk >> weight.joint_ID >> weight.bias >> junk >> weight.pos.x >> weight.pos.y >> weight.pos.z;
							IgnoreLine(file, length);

							//!!!!!!!!!!!!!!!!!!!!!!!
							mesh.weights.push_back(weight);
						}
					}
					else								// �����������
					{
						IgnoreLine(file, length);
					}
					file >> param;
				}
				// �������黺����󡢶��㻺�����������������
				CreateVertBuffer(mesh);

				//!!!!!!!!!!!!!!!!!!!!!!!
				meshList.push_back(mesh);
			}
			// >> ��ȡ��һ������Ҫ���ֵ֮���ٴ� >> ����ȡ��һ������Ҫ���ֵ 
			// �ո����ʱ������������һ��
			file >> param;
		}
		return jointList.size() == numJoints && meshList.size() == numMeshs;
	}

	// �������������������
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
		// ���������Ʒ�ʽ
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);

		return texture;
	}

	// �������黺����󡢶��㻺�����������
	void MD5_Model::CreateVertBuffer(M_Mesh &mesh)
	{
		glGenVertexArrays(1, &(mesh.VAO));
		glGenBuffers(1, &(mesh.VBO));
		glGenBuffers(1, &(mesh.EBO));
	}

	// ���㶥����ģ�Ϳռ��λ�� -> ����Update�����е���
	void MD5_Model::ComputerVertPos(M_Mesh &mesh, const MD5_Animation::X_FrameSkeleton& skeleton)
	{
		// ����mesh�����ж���
		for (unsigned int i = 0; i < mesh.verts.size(); i++)
		{
			M_Vertex &vert = mesh.verts[i];
			vert.pos = glm::vec3(0);

			/*
				������ģ�Ϳռ��е�λ�ã�
					�����λ����һ�����߶��Ȩ�ؾ�����
					Ȩ����ģ�Ϳռ��е�λ������������������Ĺ�����������
					Ҳ����˵ÿ�������λ����һ�����߶������������

					Ȩ�ؿ��Կ�������ĳ�������ռ����궨���һ������㣬
					�����ⶥ������Ź���������ռ�һ��任��
				����λ�� = һ�����߶�����ⶥ����ģ�Ϳռ��е�λ�ó���Ȩ��ռ�����ۼӵõ��ġ�
			*/

			// �������������Ȩ��
			for (int j = 0; j < vert.weightCount; j++)
			{
				// ���Ȩ��
				M_Weight &weight = mesh.weights[vert.startWeight + j];
				//// ���Ȩ�ع����ù������ݣ�Ĭ�ϵĹ������ݣ�
				//M_Joint &joint = jointList[weight.joint_ID];	// ������Ψһ�� ����ָ�ļ��������ݵ����ϵ��µ�˳��
				// -> ʹ�õ�ǰʱ�̵Ĺ������� ���������λ��
				const MD5_Animation::X_SkeletonJoint& joint = skeleton.jointList[weight.joint_ID];

				// ��������
				glm::mat4 boneMatrix(1.0f);
				glm::mat4 boneTranstion = glm::translate(boneMatrix, joint.pos);
				glm::mat4 boneRotate = glm::mat4(joint.orient);

				boneMatrix = boneTranstion * boneRotate;	// ��ǰ�����ռ� �� ģ�Ϳռ�ñ任����
				// Ȩ����ģ�Ϳռ��λ�� = �ù�����ģ�Ϳռ�任���� * ��ǰȨ���ڹ���������λ��
				glm::vec3 weightPos = glm::vec3(boneMatrix * glm::vec4(weight.pos, 1.0));	// ����Ȩ����ģ�Ϳռ��е�λ��
				// glm::vec3 weightPos = joint.pos + joint.orient * weight.pos;	// ��������㷽ʽ�ȼ�
				vert.pos += weightPos * weight.bias;	// ��Ȩ����
			}

			// !!!!!!!!!!!!!!!!!
			//mesh.posBuffer.push_back(vert.pos);
		}
	}

	// ��֡�߼�
	void MD5_Model::Update(float deltaTime)
	{
		animation.Update(deltaTime);
		const MD5_Animation::X_FrameSkeleton& skeleton = animation.GetSkeleton();

		for (unsigned int i = 0; i < meshList.size(); i++)
		{
			ComputerVertPos(meshList[i], skeleton);
		}
	}

	// ����CPU�Ĺ�����Ƥ������ÿһ֡����Ҫ������õĶ���λ�ô�CPU���͵�GPU ��һ�����������������ĵ�
	// �ڿɱ����Ⱦ���߳���֮ǰ ����ֻ����ô�� 
	// ���ǿɱ����Ⱦ���ߵĳ��� ʹ���ǿ��Խ�������Ƥ���̷ŵ� GPU��ʵ�֣�����GPU�Ĺ�����Ƥ����
	// GPU�Ĺ�����Ƥ����������ֻҪÿ֡�������й����ı任����
	// ��Ⱦģ�ͣ�������mesh�� ->  ÿ֡��Ҫ���½��������Ե�ֵ��cpu����gpu
	void MD5_Model::Render(xShader shader)
	{
		for (unsigned int i = 0; i < meshList.size(); i++)
		{
			M_Mesh mesh = meshList[i];

			// �����������
			glBindVertexArray(mesh.VAO);

			// ���㻺��
			glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
			glBufferData(GL_ARRAY_BUFFER, mesh.verts.size() * sizeof(M_Vertex), &(mesh.verts[0]), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(M_Vertex), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(M_Vertex), (void*)offsetof(M_Vertex, texcoord));

			// ������������
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