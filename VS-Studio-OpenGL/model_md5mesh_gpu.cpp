#include "model_md5mesh_gpu.h"


namespace xMD5Loader
{
	
	MD5_Model_GPU::MD5_Model_GPU()
	{
	}

	MD5_Model_GPU::~MD5_Model_GPU()
	{
	}

	// ��������Ƶ�mesh�����ж����λ�ã��������붥������Ĺ��������͹���Ȩ��
	void MD5_Model_GPU::PrepareMesh(Mesh &mesh)
	{
		// �������еĶ���
		for (unsigned int i = 0; i < mesh.verts.size(); i++)
		{
			Vertex& vert = mesh.verts[i];
			vert.pos = vec3(0);
			vert.boneIndexs = vec4(0);
			vert.boneWeights = vec4(0);

			// ����ĳһ���������Ȩ��
			for (int j = 0; j < vert.weightCount; j++)
			{
				// ���Ȩ��
				Weight& weight = mesh.weights[vert.startWeightIndex + j];
				// ���Ȩ�ض�Ӧ�Ĺ���
				Joint& joint = jointList[weight.joint_ID];

				/*
					����ļ��㷽ʽ ʹ�õ��Ǽ��� �����ļ��� ÿһ֡�����ķ�ʽ����ʹ�þ���ķ�ʽ�ǵȼ۵�
					���յĽ���ǣ������ڡ�ģ�Ϳռ䡿�е�λ��
				*/
				vec3 rotPos = joint.orient * weight.pos;
				vert.pos += (joint.pos + rotPos) * weight.bias;

				vert.boneWeights[j] = weight.bias;
				vert.boneIndexs[j] = weight.joint_ID;
			}
		}
	}

	// ���������ʱ���������� ģ�Ϳռ� �� �����ռ� �ı任����
	void MD5_Model_GPU::BuildBindPose(JointList& jointList)
	{
		inverseBindPose.clear();

		// ��������...
		JointList::const_iterator iter = jointList.begin();
		while (iter != jointList.end())
		{
			const Joint& joint = (*iter);
			mat4 boneMatrix(1.0f);
			mat4 boneTranstion = translate(boneMatrix, joint.pos);
			mat4 boneRotate = mat4(joint.orient);

			// ���һ���ռ�����ϵ P���Ըÿռ����һ����ת M ���ٽ���һ��ƽ�� T�� �õ��µĿռ�����ϵ C�� 
			// ���ڿռ�C��һ�� V��x,y,z�� ,V��P�ռ��ڵ�����Ϊ 
			//  V' = T * M  * V 
			// ����C�൱�� �����ռ�  P�൱��ģ�Ϳռ�
			boneMatrix = boneTranstion * boneRotate;		// �ӹ����ռ� �� ģ�Ϳռ� �ı任����
			mat4 inverseBoneMatrix = inverse(boneMatrix);	// ��ģ�Ϳռ� �� �����ռ� �ı任����
			inverseBindPose.push_back(inverseBoneMatrix);
			iter++;
		}
	}

	// ����ģ��
	bool MD5_Model_GPU::LoadModel(string& path)
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
				Joint joint;
				file >> junk;	// ǰ��Ĵ�����
				for (int i = 0; i < numJoints; i++)
				{
					file >> joint.name >> joint.parent_ID >> junk >> joint.pos.x >> joint.pos.y >> joint.pos.z >> junk >> junk >> joint.orient.x >> joint.orient.y >> joint.orient.z >> junk;
					IgnoreLine(file, length);		// ���Ե�ǰ�к����λ��
					RemoveNotes(joint.name);		// ȥ�� "" ����
					ComputeQuatW(joint.orient);		// ������ת��w����

					//!!!!!!!!!!!!!!!!!!!!!!!
					jointList.push_back(joint);
				}
				file >> junk;	// ����Ĵ�����

				// ���������ʱ���������� ģ�Ϳռ� �� �����ռ� �ı任����
				BuildBindPose(jointList);
			}
			else if (param == "mesh")					// ��������
			{
				Mesh mesh;
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
							Vertex vert;
							file >> junk >> junk >> junk >> vert.texcoord.x >> vert.texcoord.y >> junk >> vert.startWeightIndex >> vert.weightCount;
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
							Weight weight;
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
				// ��������Ƶ�mesh�����ж����λ�ã��������붥������Ĺ��������͹���Ȩ��
				PrepareMesh(mesh);

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
	void MD5_Model_GPU::CreateVertBuffer(Mesh& mesh)
	{
		glGenVertexArrays(1, &(mesh.VAO));
		glGenBuffers(1, &(mesh.VBO));
		glGenBuffers(1, &(mesh.EBO));

		// ����GPUʵ�ֵĹ���������ֻ��Ҫ�������ƵĶ������Դ��ݸ�GPUһ�ξ���
		// ������Ⱦʱ��ÿֻ֡Ҫ�����ռ�Ĵ��ݱ任�������
		glBindVertexArray(mesh.VAO);

		// ���㻺��
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

		// ������������
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer.size() * sizeof(unsigned int), &(mesh.indexBuffer[0]), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	// ��֡�߼� ��opengl ��Ⱦѭ���е���
	void MD5_Model_GPU::Update(float deltaTime)
	{
		animation.Update(deltaTime);

		// ��ǰ������Ϣ
		const MD5_GPU_Animation::FrameSkeleton& skeleton = animation.GetSkeleton();

		animatedBones.clear();
		for (int i = 0; i < numJoints; i++)
		{
			// ��ǰ���ƵĹ����ռ�仯��ģ�Ϳռ� * �����Ƶ�ģ�Ϳռ䵽�����ռ�ı任����
			// ������Ӧ�ò𿪷ֱ����� ΪʲôҪ����أ�
			glm::mat4 matrix = skeleton.boneMatrixs[i] * inverseBindPose[i];
			animatedBones.push_back(matrix);
		}
	}

	// ��Ⱦģ�ͣ�������mesh��
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



	//// ��������ռ䵽ģ�Ϳռ�ı任����
	//void MD5_Model_GPU::ComputerMatrix(Mesh &mesh, const MD5_GPU_Animation::FrameSkeleton& skeleton)
	//{
	
	//}
}