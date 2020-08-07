#pragma once
/*
加载模型的步骤：（这里已obj格式为例，进行实现说明）
	1：了解obj模型的格式，（其他格式的要自定义相应的解析器，可以使用现有的模型加载库）
	2：定义合适的数据结构，解析每一行的关键字，
	3：读取对应的数据到数据结构中


详细的步骤：
	
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

	// 定义顶点
	struct M_Vertex
	{
		vec3 pos;
		vec3 normal;
		vec2 texcoord;
	};

	// 定义材质
	struct M_Material
	{
		string name;

		// 材质的光亮度
		float Ns;

		// 材质的Alpha透明度
		float d;

		// 材质的照明度
		float illlum;

		vec3 Ka;	// 环境参数
		vec3 Ks;	// 镜面参数
		vec3 Kd;	// 漫反射参数

		string map_Ka;	// 环境贴图

		int ka_id;		// 环境贴图id		这个是哪儿来的？
	};

	// *************************************************************************************
	/*
		绘制是以mesh为单位进行的，我们在M_Mesh中添加
			SetupMesh: 设置mesh的渲染状态
			Draw: 绘制mesh

	*/


	// 存储一个mesh的所有数据
	class M_Mesh {
	public:
		string name;
		vector<M_Vertex> vertexs;
		vector<unsigned int> indecies;		// 这里对什么的索引 一个面开始的时候再vertexs 的起始下标
		M_Material material;				// 网格对应的材质

		M_Mesh() {}

		M_Mesh(vector<M_Vertex> Vertexs, vector<unsigned int> Indecies)
		{
			vertexs = Vertexs;
			indecies = Indecies;

			//cout << "vertex.x :" << vertexs[0].normal.x << endl;
			//cout << "vertex.y :" << vertexs[0].normal.y << endl;
			//cout << "vertex.z :" << vertexs[0].normal.z << endl;

			// 在构建网格的时候就设置渲染状态 -> 起始就是把模型的数据设置给 shader 对象的 顶点属性中
			// 注意区分 顶点属性 和 uniform
			setupMesh();
		}

		void Draw(xShader shader);

	private:
		unsigned int VAO, VBO, EBO;
		void setupMesh();
	};

	void M_Mesh::Draw(xShader shader)
	{
		// 激活纹理单元
		glActiveTexture(GL_TEXTURE0);
		// 绑定纹理 -> 会把纹理赋值给指定纹理单元的采样器
		glBindTexture(GL_TEXTURE_2D, material.ka_id);

		// 将 diffTex 对应到 0 对应的纹理单元 
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


	// 存储一个模型的所有数据
	class M_Model
	{
	public:
		vector<M_Material> loadedMaterials;		// model中所有的的mat
		vector<M_Mesh> loadedMeshes;			// model中包含的所有mesh

		M_Model(string path);
		void GenVerticesObj(vector<M_Vertex> &outVerts, vector<vec3> &pos, vector<vec3> &normals, vector<vec2> texcoords, string curline);
		bool LoadMaterial(string path);
		unsigned int TextureFromFile(const char *path);
		void Draw(xShader shader);
	};

	// 构造函数
	M_Model::M_Model(string path)
	{
		// 在model的构造函数中定义一个【解析过程】
		// 该【解析过程】将obj和mtl文件数据加载到对应数据结构中

		string str(absolutePath);	// absolutePath 是完整路径的
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
		string meshName;					// 每个网格的name 会随着o、g进行更新
		M_Mesh tmpMesh;						// 当前的mesh对象 会随着o、g进行更新

		// ****** 声明用于保存obj数据的容器 ******
		// 原始数据
		vector<vec3> allPos;				// 报错整个模型的所有的顶点 
		vector<vec3> allNormals;
		vector<vec2> allTexcoords;

		// 这些是按我们定义好的结构组织的数据
		// 每次遇到 o、g关键字时清除
		vector<M_Vertex> allVertexs;		// 保存一个网格 或一组 数据 对应的顶点 
		// 每次遇到 o、g关键字时清除
		vector<unsigned int> allIndices;	// 每个索引对应 一个面 在 allVertexs 中的起始index
		vector<string> meshMatNames;		// 保存所有网格使用的材质的名

		// 读取文件中的数据
		string curline;
		while (getline(file, curline))
		{
			// 获取当前行的关键字
			string firstToken = FirstToken(curline);

			/*
				o：表示一个对象，即一个mesh
				g：表示一个组，指定从该行到下一个g开头的所有对象为一组，这个组是个什么概念？
					就是把后面的一堆对象成组，可能是一些面和顶点和法线等数据一起组成的一组数据
			*/
			if (firstToken == "o" || firstToken == "g")
			{
				// 第一次时还没有f的数据（已经有顶点、法线、uv那些数据了） 无法构成网格 故只取一个名字
				if (is_First)
				{
					is_First = false;
					// 取当前行关键字后面的内容
					meshName = TailToken(curline);
				}
				else
				{
					if (!allIndices.empty() && !allVertexs.empty())
					{
						// 生成一个Mesh
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
			else if (firstToken == "v")	// 顶点
			{
				vector<string> strVec;
				vec3 pos;
				xTools::Split(xTools::TailToken(curline), strVec, " ");
				pos.x = stof(strVec[0]);	//string 转换成 float
				pos.y = stof(strVec[1]);
				pos.z = stof(strVec[2]);
				allPos.push_back(pos);
			}
			else if (firstToken == "vn")	// 顶点法线
			{
				vector<string> strVec;
				vec3 normal;
				xTools::Split(xTools::TailToken(curline), strVec, " ");
				normal.x = stof(strVec[0]);
				normal.y = stof(strVec[1]);
				normal.z = stof(strVec[2]);
				allNormals.push_back(normal);
			}
			else if (firstToken == "vt")	// 纹理坐标
			{
				vector<string> strVec;
				vec2 texcoord;
				xTools::Split(xTools::TailToken(curline), strVec, " ");
				texcoord.x = stof(strVec[0]);
				texcoord.y = stof(strVec[1]);
				allTexcoords.push_back(texcoord);
			}
			else if (firstToken == "f")		// f 代表一个面，数据分别对应一组基本数据的索引
			{
				// 生成一组顶点
				vector<M_Vertex> vVerts;
				// 根据位置、法线、uv坐标，获得当前curline表示的顶点类型的对象
				GenVerticesObj(vVerts, allPos, allNormals, allTexcoords, curline);

				// 将顶点放入 allVertexs 容器
				for (int i = 0; i < vVerts.size(); i++)
				{
					allVertexs.push_back(vVerts[i]);
				}

				for (int i = 0; i < 3; i++)
				{
					unsigned int indnum = (unsigned int)((allVertexs.size()) - vVerts.size()) + i;
					// allIndice 存储了用于 allVertexs 的那个index起 对应的是 组成一个面的顶点
					allIndices.push_back(indnum);
				}
			}
			else if (firstToken == "mtllib")	// 对应材质库文件名称
			{
				vector<string> pathArr;		// 用于存储 材质的路径
				string mtlPath;

				// 这两步在一起有啥用？？
				xTools::Split(path, pathArr, "/");
				for (int i = 0; i < pathArr.size() - 1; i++)
				{
					mtlPath += pathArr[i] + "/";
				}

				mtlPath += xTools::TailToken(curline);

				// 加载材质 ****
				LoadMaterial(mtlPath);
			}
			else if (firstToken == "usemtl")	// 每个mesh对应的材质名
			{
				// 保存所有的 材质名
				string matName = xTools::TailToken(curline);
				meshMatNames.push_back(matName);
			}

		}

		// 对于最后一组数据 要在结束的时候再处理下
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

		// 给网格的mat进行赋值
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

	// 根据位置、法线、uv坐标已经当前顶点的索引，获得一组顶点类型的结构体
	void M_Model::GenVerticesObj(vector<M_Vertex> &outVerts, vector<vec3> &pos, vector<vec3> &normals, vector<vec2> texcoords, string curline)
	{
		vector<string> faces, verts;

		//读取curline中的原始数据 三个顶点的 顶点/uv坐标/法线 的索引
		xTools::Split(xTools::TailToken(curline), faces, " ");	// 三个顶点的信息
		for (int i = 0; i < faces.size(); i++)
		{
			M_Vertex tmpVertex;
			verts.clear();			// 一个顶点的信息
			xTools::Split(faces[i], verts, "/");

			int types = verts.size();
			tmpVertex.pos = xTools::GetElement(pos, verts[0]);
			tmpVertex.normal = xTools::GetElement(normals, verts[2]);
			tmpVertex.texcoord = xTools::GetElement(texcoords, verts[1]);
			// 根据模型的uv坐标的y轴和opengl的是否复合，决定是否需要进行 1- 操作
			tmpVertex.texcoord.y = 1 - tmpVertex.texcoord.y;

			// 给需要返回的对象装载相应顶点对象
			outVerts.push_back(tmpVertex);
		}
	}

	// 加载材质
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
			// 获得关键字
			string first = xTools::FirstToken(curline);
			if (first == "newmtl")	// 表示定义了一个材质
			{
				if (isFirst)
				{
					isFirst = false;
					if (curline.size() > 7)	// 因为 肯定是以 Material 开头 所以必然 > 7
						material.name = xTools::TailToken(curline);
				}
				else
				{
					loadedMaterials.push_back(material);	// 结构体 不需要 new
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
			else if (first == "map_Ka")	// 环境贴图	这一项有时候可以是没有的
			{

				// 环境贴图名称
				material.map_Ka = xTools::TailToken(curline);

				// 获得环境贴图路径
				vector<string> pathArr;
				string texturePath;
				xTools::Split(path, pathArr, "/");
				for (int i = 0; i < pathArr.size() - 1; i++)
				{
					texturePath += pathArr[i] + "/";
				}
				texturePath += material.map_Ka;

				//cout << "texturePath : " << texturePath << endl;
				// 加载环境贴图
				material.ka_id = TextureFromFile(texturePath.c_str());
			}
		}

		// 最后一个 需要单独再push下
		loadedMaterials.push_back(material);

		if (loadedMaterials.empty())
			return false;
		return true;
	}

	unsigned int M_Model::TextureFromFile(const char *path)
	{
		// 声明纹理对象
		unsigned int texture;
		glGenTextures(1, &texture);

		// 绑定纹理
		glBindTexture(GL_TEXTURE_2D, texture);

		// 为当前绑定的纹理对象设置环绕、过滤方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 加载纹理数据
		int width, height, nrChannels;
		// 反转纹理的y轴
		//stbi_set_flip_vertically_on_load(true);	// 我们在解析模型数据的顶点坐标时已经反转了 这里就不用反转了
		unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

		if (data)
		{
			int mode = nrChannels == 3 ? GL_RGB : GL_RGBA;

			// 使用纹理数据 生成纹理
			glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode, GL_UNSIGNED_BYTE, data);
			// 自动生成多级渐变纹理 
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			cout << "Failed to load texture1" << endl;
		}

		// 释放纹理数据内存
		stbi_image_free(data);

		return texture;
	}

	// 绘制模型
	void M_Model::Draw(xShader shader)
	{
		//cout << " size: " << loadedMeshes.size() << endl;

		// 绘制每一个mesh
		for (int i = 0; i < loadedMeshes.size(); i++)
		{
			loadedMeshes[i].Draw(shader);
		}
	}
}


