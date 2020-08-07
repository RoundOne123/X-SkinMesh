#pragma once
/*
����ģ�͵Ĳ��裺��������obj��ʽΪ��������ʵ��˵����
	1���˽�objģ�͵ĸ�ʽ����������ʽ��Ҫ�Զ�����Ӧ�Ľ�����������ʹ�����е�ģ�ͼ��ؿ⣩
	2��������ʵ����ݽṹ������ÿһ�еĹؼ��֣�
	3����ȡ��Ӧ�����ݵ����ݽṹ��


��ϸ�Ĳ��裺
	
*/


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <vector>

#include "stb_image.h"
#include "xShader.h"
#include "xTools.h"

using namespace glm;
using namespace std;

using namespace xTools;


namespace xObjLoader
{

	static string absolutePath = "";

	// ���嶥��
	struct M_Vertex
	{
		vec3 pos;
		vec3 normal;
		vec2 texcoord;
	};

	// �������
	struct M_Material
	{
		string name;

		// ���ʵĹ�����
		float Ns;

		// ���ʵ�Alpha͸����
		float d;

		// ���ʵ�������
		float illlum;

		vec3 Ka;	// ��������
		vec3 Ks;	// �������
		vec3 Kd;	// ���������

		string map_Ka;	// ������ͼ

		int ka_id;		// ������ͼid		������Ķ����ģ�
	};

	// *************************************************************************************
	/*
		��������meshΪ��λ���еģ�������M_Mesh�����
			SetupMesh: ����mesh����Ⱦ״̬
			Draw: ����mesh

	*/


	// �洢һ��mesh����������
	class M_Mesh {
	public:
		string name;
		vector<M_Vertex> vertexs;
		vector<unsigned int> indecies;		// �����ʲô������ һ���濪ʼ��ʱ����vertexs ����ʼ�±�
		M_Material material;				// �����Ӧ�Ĳ���

		M_Mesh() {}

		M_Mesh(vector<M_Vertex> Vertexs, vector<unsigned int> Indecies)
		{
			vertexs = Vertexs;
			indecies = Indecies;

			//cout << "vertex.x :" << vertexs[0].normal.x << endl;
			//cout << "vertex.y :" << vertexs[0].normal.y << endl;
			//cout << "vertex.z :" << vertexs[0].normal.z << endl;

			// �ڹ��������ʱ���������Ⱦ״̬ -> ��ʼ���ǰ�ģ�͵��������ø� shader ����� ����������
			// ע������ �������� �� uniform
			setupMesh();
		}

		void Draw(xShader shader);

	private:
		unsigned int VAO, VBO, EBO;
		void setupMesh();
	};

	void M_Mesh::Draw(xShader shader)
	{
		// ��������Ԫ
		glActiveTexture(GL_TEXTURE0);
		// ������ -> �������ֵ��ָ������Ԫ�Ĳ�����
		glBindTexture(GL_TEXTURE_2D, material.ka_id);

		// �� diffTex ��Ӧ�� 0 ��Ӧ������Ԫ 
		shader.setInt("diffTex", 0);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indecies.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}

	void M_Mesh::setupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(M_Vertex) * vertexs.size(), &vertexs[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(M_Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(M_Vertex), (void*)offsetof(M_Vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(M_Vertex), (void*)offsetof(M_Vertex, texcoord));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indecies.size() * sizeof(unsigned int), &indecies[0], GL_STATIC_DRAW);

		glBindVertexArray(0);
	}



	// ****************************************************************************


	// �洢һ��ģ�͵���������
	class M_Model
	{
	public:
		vector<M_Material> loadedMaterials;		// model�����еĵ�mat
		vector<M_Mesh> loadedMeshes;			// model�а���������mesh

		M_Model(string path);
		void GenVerticesObj(vector<M_Vertex> &outVerts, vector<vec3> &pos, vector<vec3> &normals, vector<vec2> texcoords, string curline);
		bool LoadMaterial(string path);
		unsigned int TextureFromFile(const char *path);
		void Draw(xShader shader);
	};

	// ���캯��
	M_Model::M_Model(string path)
	{
		// ��model�Ĺ��캯���ж���һ�����������̡�
		// �á��������̡���obj��mtl�ļ����ݼ��ص���Ӧ���ݽṹ��

		string str(absolutePath);	// absolutePath ������·����
		path = str + path;

		if (path.substr(path.size() - 4, 4) != ".obj")
		{
			cout << "Only load obj model : " << path << endl;
			return;
		}

		ifstream file(path);
		if (!file.is_open())
		{
			cout << "Open file fail : " << path << endl;
			return;
		}

		bool is_First = true;
		string meshName;					// ÿ�������name ������o��g���и���
		M_Mesh tmpMesh;						// ��ǰ��mesh���� ������o��g���и���

		// ****** �������ڱ���obj���ݵ����� ******
		// ԭʼ����
		vector<vec3> allPos;				// ��������ģ�͵����еĶ��� 
		vector<vec3> allNormals;
		vector<vec2> allTexcoords;

		// ��Щ�ǰ����Ƕ���õĽṹ��֯������
		// ÿ������ o��g�ؼ���ʱ���
		vector<M_Vertex> allVertexs;		// ����һ������ ��һ�� ���� ��Ӧ�Ķ��� 
		// ÿ������ o��g�ؼ���ʱ���
		vector<unsigned int> allIndices;	// ÿ��������Ӧ һ���� �� allVertexs �е���ʼindex
		vector<string> meshMatNames;		// ������������ʹ�õĲ��ʵ���

		// ��ȡ�ļ��е�����
		string curline;
		while (getline(file, curline))
		{
			// ��ȡ��ǰ�еĹؼ���
			string firstToken = FirstToken(curline);

			/*
				o����ʾһ�����󣬼�һ��mesh
				g����ʾһ���飬ָ���Ӹ��е���һ��g��ͷ�����ж���Ϊһ�飬������Ǹ�ʲô���
					���ǰѺ����һ�Ѷ�����飬������һЩ��Ͷ���ͷ��ߵ�����һ����ɵ�һ������
			*/
			if (firstToken == "o" || firstToken == "g")
			{
				// ��һ��ʱ��û��f�����ݣ��Ѿ��ж��㡢���ߡ�uv��Щ�����ˣ� �޷��������� ��ֻȡһ������
				if (is_First)
				{
					is_First = false;
					// ȡ��ǰ�йؼ��ֺ��������
					meshName = TailToken(curline);
				}
				else
				{
					if (!allIndices.empty() && !allVertexs.empty())
					{
						// ����һ��Mesh
						tmpMesh = M_Mesh(allVertexs, allIndices);
						tmpMesh.name = meshName;
						loadedMeshes.push_back(tmpMesh);
						allIndices.clear();
						allVertexs.clear();

						meshName.clear();
						meshName = xTools::TailToken(curline);
					}
					else
					{
						meshName = xTools::TailToken(curline);
					}
				}

			}
			else if (firstToken == "v")	// ����
			{
				vector<string> strVec;
				vec3 pos;
				xTools::Split(xTools::TailToken(curline), strVec, " ");
				pos.x = stof(strVec[0]);	//string ת���� float
				pos.y = stof(strVec[1]);
				pos.z = stof(strVec[2]);
				allPos.push_back(pos);
			}
			else if (firstToken == "vn")	// ���㷨��
			{
				vector<string> strVec;
				vec3 normal;
				xTools::Split(xTools::TailToken(curline), strVec, " ");
				normal.x = stof(strVec[0]);
				normal.y = stof(strVec[1]);
				normal.z = stof(strVec[2]);
				allNormals.push_back(normal);
			}
			else if (firstToken == "vt")	// ��������
			{
				vector<string> strVec;
				vec2 texcoord;
				xTools::Split(xTools::TailToken(curline), strVec, " ");
				texcoord.x = stof(strVec[0]);
				texcoord.y = stof(strVec[1]);
				allTexcoords.push_back(texcoord);
			}
			else if (firstToken == "f")		// f ����һ���棬���ݷֱ��Ӧһ��������ݵ�����
			{
				// ����һ�鶥��
				vector<M_Vertex> vVerts;
				// ����λ�á����ߡ�uv���꣬��õ�ǰcurline��ʾ�Ķ������͵Ķ���
				GenVerticesObj(vVerts, allPos, allNormals, allTexcoords, curline);

				// ��������� allVertexs ����
				for (int i = 0; i < vVerts.size(); i++)
				{
					allVertexs.push_back(vVerts[i]);
				}

				for (int i = 0; i < 3; i++)
				{
					unsigned int indnum = (unsigned int)((allVertexs.size()) - vVerts.size()) + i;
					// allIndice �洢������ allVertexs ���Ǹ�index�� ��Ӧ���� ���һ����Ķ���
					allIndices.push_back(indnum);
				}
			}
			else if (firstToken == "mtllib")	// ��Ӧ���ʿ��ļ�����
			{
				vector<string> pathArr;		// ���ڴ洢 ���ʵ�·��
				string mtlPath;

				// ��������һ����ɶ�ã���
				xTools::Split(path, pathArr, "/");
				for (int i = 0; i < pathArr.size() - 1; i++)
				{
					mtlPath += pathArr[i] + "/";
				}

				mtlPath += xTools::TailToken(curline);

				// ���ز��� ****
				LoadMaterial(mtlPath);
			}
			else if (firstToken == "usemtl")	// ÿ��mesh��Ӧ�Ĳ�����
			{
				// �������е� ������
				string matName = xTools::TailToken(curline);
				meshMatNames.push_back(matName);
			}

		}

		// �������һ������ Ҫ�ڽ�����ʱ���ٴ�����
		if (!allIndices.empty() && !allVertexs.empty())
		{
			tmpMesh = M_Mesh(allVertexs, allIndices);
			tmpMesh.name = meshName;
			loadedMeshes.push_back(tmpMesh);
		}

		file.close();

		//for (int j = 0; j < loadedMaterials.size(); j++)
		//{
		//	cout << "mat name  :" << loadedMaterials[j].name << endl;
		//}

		// �������mat���и�ֵ
		for (int i = 0; i < meshMatNames.size(); i++)
		{
			string meshMatName = meshMatNames[i];

			for (int j = 0; j < loadedMaterials.size(); j++)
			{
				if (loadedMaterials[j].name == meshMatName)
				{
					loadedMeshes[i].material = loadedMaterials[j];
					break;
				}
			}
		}
	}

	// ����λ�á����ߡ�uv�����Ѿ���ǰ��������������һ�鶥�����͵Ľṹ��
	void M_Model::GenVerticesObj(vector<M_Vertex> &outVerts, vector<vec3> &pos, vector<vec3> &normals, vector<vec2> texcoords, string curline)
	{
		vector<string> faces, verts;

		//��ȡcurline�е�ԭʼ���� ��������� ����/uv����/���� ������
		xTools::Split(xTools::TailToken(curline), faces, " ");	// �����������Ϣ
		for (int i = 0; i < faces.size(); i++)
		{
			M_Vertex tmpVertex;
			verts.clear();			// һ���������Ϣ
			xTools::Split(faces[i], verts, "/");

			int types = verts.size();
			tmpVertex.pos = xTools::GetElement(pos, verts[0]);
			tmpVertex.normal = xTools::GetElement(normals, verts[2]);
			tmpVertex.texcoord = xTools::GetElement(texcoords, verts[1]);
			// ����ģ�͵�uv�����y���opengl���Ƿ񸴺ϣ������Ƿ���Ҫ���� 1- ����
			tmpVertex.texcoord.y = 1 - tmpVertex.texcoord.y;

			// ����Ҫ���صĶ���װ����Ӧ�������
			outVerts.push_back(tmpVertex);
		}
	}

	// ���ز���
	bool M_Model::LoadMaterial(string path)
	{
		if (path.substr(path.size() - 4, path.size()) != ".mtl")
			return false;

		ifstream file;
		file.open(path);

		M_Material material;
		string curline;
		bool isFirst = true;

		while (getline(file, curline))
		{
			// ��ùؼ���
			string first = xTools::FirstToken(curline);
			if (first == "newmtl")	// ��ʾ������һ������
			{
				if (isFirst)
				{
					isFirst = false;
					if (curline.size() > 7)	// ��Ϊ �϶����� Material ��ͷ ���Ա�Ȼ > 7
						material.name = xTools::TailToken(curline);
				}
				else
				{
					loadedMaterials.push_back(material);	// �ṹ�� ����Ҫ new
					material = M_Material();
					if (curline.size() > 7)
						material.name = xTools::TailToken(curline);
				}
			}
			else if (first == "Ns")
			{
				material.Ns = stof(xTools::TailToken(curline));
			}
			else if (first == "d")
			{
				material.d = stof(xTools::TailToken(curline));
			}
			else if (first == "illum")
			{
				material.illlum = stof(xTools::TailToken(curline));
			}
			else if (first == "Ka")
			{
				vector<string> kaArr;
				xTools::Split(xTools::TailToken(curline), kaArr, " ");
				material.Ka = glm::vec3(stof(kaArr[0]), stof(kaArr[1]), stof(kaArr[2]));
			}

			else if (first == "Kd")
			{
				vector<string> kdArr;
				xTools::Split(xTools::TailToken(curline), kdArr, " ");
				material.Kd = glm::vec3(stof(kdArr[0]), stof(kdArr[1]), stof(kdArr[2]));
			}

			else if (first == "Ks")
			{
				vector<string> ksArr;
				xTools::Split(xTools::TailToken(curline), ksArr, " ");
				material.Ks = glm::vec3(stof(ksArr[0]), stof(ksArr[1]), stof(ksArr[2]));
			}
			else if (first == "map_Ka")	// ������ͼ	��һ����ʱ�������û�е�
			{

				// ������ͼ����
				material.map_Ka = xTools::TailToken(curline);

				// ��û�����ͼ·��
				vector<string> pathArr;
				string texturePath;
				xTools::Split(path, pathArr, "/");
				for (int i = 0; i < pathArr.size() - 1; i++)
				{
					texturePath += pathArr[i] + "/";
				}
				texturePath += material.map_Ka;

				//cout << "texturePath : " << texturePath << endl;
				// ���ػ�����ͼ
				material.ka_id = TextureFromFile(texturePath.c_str());
			}
		}

		// ���һ�� ��Ҫ������push��
		loadedMaterials.push_back(material);

		if (loadedMaterials.empty())
			return false;
		return true;
	}

	unsigned int M_Model::TextureFromFile(const char *path)
	{
		// �����������
		unsigned int texture;
		glGenTextures(1, &texture);

		// ������
		glBindTexture(GL_TEXTURE_2D, texture);

		// Ϊ��ǰ�󶨵�����������û��ơ����˷�ʽ
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// ������������
		int width, height, nrChannels;
		// ��ת�����y��
		//stbi_set_flip_vertically_on_load(true);	// �����ڽ���ģ�����ݵĶ�������ʱ�Ѿ���ת�� ����Ͳ��÷�ת��
		unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

		if (data)
		{
			int mode = nrChannels == 3 ? GL_RGB : GL_RGBA;

			// ʹ���������� ��������
			glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode, GL_UNSIGNED_BYTE, data);
			// �Զ����ɶ༶�������� 
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			cout << "Failed to load texture1" << endl;
		}

		// �ͷ����������ڴ�
		stbi_image_free(data);

		return texture;
	}

	// ����ģ��
	void M_Model::Draw(xShader shader)
	{
		//cout << " size: " << loadedMeshes.size() << endl;

		// ����ÿһ��mesh
		for (int i = 0; i < loadedMeshes.size(); i++)
		{
			loadedMeshes[i].Draw(shader);
		}
	}
}


