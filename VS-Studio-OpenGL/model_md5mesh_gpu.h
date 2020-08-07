
// 基于CPU的骨骼蒙皮动画，每一帧都需要将计算好的顶点位置从CPU发送到GPU 这一个过程是有性能消耗的
// 在可编程渲染管线出来之前 我们只能这么做 
// 但是可编程渲染管线的出现 使我们可以将骨骼蒙皮过程放到 GPU中实现，基于GPU的骨骼蒙皮动画
// GPU的骨骼蒙皮动画，我们只要每帧发送所有骨骼的变换矩阵



	/*
		基于GPU的实现思路：
			首先明确，绑定到姿势之后，所有顶点相对于其关联的【骨骼空间】的位置就不会再变化了；
			所以，
			我们将【绑定姿势下】顶点的【模型空间位置】传递到顶点着色器；
			再将【绑定姿势下】模型空间到各骨骼空间的【变换矩阵】传递到顶点着色器；
			然后计算出顶点在【骨骼空间下的位置】；（保存，就不会在变了）
			然后在Update函数中再计算出当前时刻【所有骨骼空间到模型空间的变化矩阵】；（我们每帧要传递个GPU的数据）
			最终就可以利用这些变化矩阵，计算出当前的各顶点再模型空间中的位置了。（我们最终需要的值）
	*/

#ifndef MODEL_MD5_MESH_GPU_H_
#define MODEL_MD5_MESH_GPU_H_

#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "stb_image.h"
#include "xShader.h"
#include "xTools.h"

#include "model_md5anim_gpu.h"

using namespace glm;

namespace xMD5Loader
{
	class MD5_Model_GPU
	{
	public:

#pragma region 结构体声明
		// 顶点数据
		struct Vertex
		{
			glm::vec3 pos;			// 顶点在模型空间中的位置
			glm::vec2 texcoord;		// 纹理坐标
			int startWeightIndex;	// 所关联的权重起始索引
			int weightCount;		// 所关联的权重总数

			// 在顶点着色器中计算顶点位置 -> 需要将骨骼关联的骨骼数据全部上传给GPU

			glm::vec4 boneWeights;	// 所关联的权重值		-> 这里表明最多关联四个权重
			glm::vec4 boneIndexs;	// 所关联的权重，关联的骨骼的索引  -> 也就是（间接）关联的骨骼的索引
		};
		typedef vector<Vertex> VertexList;

		// 关节、骨骼
		struct Joint
		{
			string name;		// 骨骼名称
			int parent_ID;		// 父骨骼在骨骼层次结构中的索引
			vec3 pos;			// 初始姿势时骨骼在模型空间中的位置
			quat orient;		// 初始姿势时骨骼坐标空间相对模型空间的旋转
		};
		typedef vector<Joint> JointList;


		// 权重
		struct Weight
		{
			int joint_ID;		// 与该权重关联的骨骼在骨骼层次结构中的索引
			float bias;			// 权重占比
			vec3 pos;			// 权重在所关联骨骼坐标空间的位置
		};
		typedef vector<Weight> WeightList;
		typedef vector<unsigned int> IndexBuffer;

		typedef vector<glm::mat4> MartrixList;

		typedef vector<glm::vec3> PositionBuffer;
		typedef vector<glm::vec3> NormalBuffer;

		// 网格
		struct Mesh
		{
			string shader;					// 纹理
			unsigned int texID;				// 纹理缓冲对象
			unsigned int VAO, VBO, EBO;		// 顶点数组对象， 顶点缓冲对象，索引缓冲对象
			VertexList verts;				// 顶点数组
			WeightList weights;				// 权重数组
			IndexBuffer indexBuffer;		// 索引数组		-> 由 vert 对应的数据得到

			//PositionBuffer posBuffer;		// 位置缓冲
			//NormalBuffer normalBuffer;		// 法线缓冲			法线也需要进行蒙皮计算  这里暂时没处理

		};
		typedef vector<Mesh> MeshList;
#pragma endregion


		MD5_Model_GPU();
		~MD5_Model_GPU();
		// 加载模型
		bool LoadModel(string &path);
		// 加载纹理并创建纹理对象
		unsigned int LoadTexture(string &path);
		// 创建数组缓冲对象、顶点缓冲和索引缓冲
		void CreateVertBuffer(Mesh &mesh);
		// 计算绑定姿势时，各骨骼从 模型空间 到 骨骼空间 的变换矩阵
		void BuildBindPose(JointList &jointList);
		// 计算绑定姿势的mesh的所有顶点的位置，并设置与顶点关联的骨骼索引和骨骼权重
		void PrepareMesh(Mesh &mesh);
		// 走帧逻辑 在opengl 渲染循环中调用
		void Update(float deltaTime);
		// 渲染模型（的所有mesh）
		void Render(xShader shader);
		// 动画类
		MD5_GPU_Animation animation;


		//// 计算骨骼空间到模型空间的变换矩阵  ->  逻辑简单 直接放在update中弄了 未抽象出一个新的方法
		//void ComputerMatrix(Mesh &mesh, const MD5_GPU_Animation::FrameSkeleton& skeleton);

	private:
		int numJoints;					// 骨骼数量
		int numMeshs;					// mesh数量
		JointList jointList;			// 骨骼数组
		MeshList meshList;				// mesh数组	
		MartrixList inverseBindPose;	// 各骨骼从模型空间到骨骼空间的变换矩阵

		MartrixList animatedBones;		//
	};

}

#endif // !MODEL_MD5_MESH_GPU_H_
