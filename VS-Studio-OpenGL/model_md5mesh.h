/*
骨骼蒙皮动画：
	Mesh还需要蒙皮信息，即Skin数据，没有Skin数据就是一个普通的静态Mesh了。
	Skin数据决定顶点如何绑定到骨骼上，顶点的Skin数据包括顶点受哪些骨骼影响以及这些骨骼影响该顶点时的权重(weight)。
	另外对于每块骨骼还 需要骨骼偏移矩阵(BoneOffsetMatrix)用来将顶点从Mesh空间变换到骨骼空间。
	骨骼控制蒙皮(顶点)运动，而骨骼本身的运动呢？当然是动画数据了。
	每个关键帧中包含时间和骨骼运动信息，运动信息可以 用一个矩阵直接表示骨骼新的变换，也可用四元数表示骨骼的旋转，
	也可以随便自己定义什么只要能让骨骼动就行。除了使用编辑设定好的动画帧数据，也可以使用 物理计算对骨骼进行实时控制。
*/

#ifndef MODEL_MD5_MESH_H_
#define MODEL_MD5_MESH_H_

#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "stb_image.h"
#include "xShader.h"
#include "xTools.h"
#include "model_md5anim.h"

using namespace std;
using namespace glm;

using namespace xTools;

namespace xMD5Loader
{



	// 模型
	class MD5_Model
	{
#pragma region 结构体定义
		// 关节、骨骼
		struct M_Joint
		{
			string name;		// 骨骼名称
			int parent_ID;		// 父骨骼在骨骼层次结构中的索引
			vec3 pos;			// 初始姿势时骨骼在模型空间中的位置
			quat orient;		// 初始姿势时骨骼坐标空间相对模型空间的旋转
		};
		typedef vector<M_Joint> JointList;

		// 顶点
		struct M_Vertex
		{
			vec3 pos;			// 顶点在模型空间的位置	->	根据其他数据计算得出
			vec2 texcoord;		// 纹理坐标
			int startWeight;	// 所关联权重的起始索引
			int weightCount;	// 所关联的权重总数
		};
		typedef vector<M_Vertex> VertexList;

		// 权重
		struct M_Weight
		{
			int joint_ID;		// 与该权重关联的骨骼在骨骼层次结构中的索引
			float bias;			// 权重占比
			vec3 pos;			// 权重在所关联骨骼坐标空间的位置
		};
		typedef vector<M_Weight> WeightList;


		typedef vector<glm::vec3> PositionBuffer;
		typedef vector<glm::vec3> NormalBuffer;
		typedef vector<unsigned int> IndexBuffer;		// 索引缓冲？？

		// 网格
		struct M_Mesh
		{
			string shader;		// 纹理
			unsigned int texID;	// 纹理缓冲对象
			unsigned int VAO, VBO, EBO;		// 顶点数组对象， 顶点缓冲对象，索引缓冲对象
			VertexList verts;				// 顶点数组
			WeightList weights;				// 权重数组
			IndexBuffer indexBuffer;		// 索引数组		-> 由 vert 对应的数据得到

			PositionBuffer posBuffer;		// 位置缓冲
			NormalBuffer normalBuffer;		// 法线缓冲			法线也需要进行蒙皮计算  这里暂时没处理

		};
		typedef vector<M_Mesh> MeshList;

#pragma endregion


	public:
		MD5_Model();
		~MD5_Model();
		// 加载模型
		bool LoadModel(string &path);
		// 加载纹理并创建纹理对象
		unsigned int LoadTexture(string &path);
		// 创建数组缓冲对象、顶点缓冲和索引缓冲
		void CreateVertBuffer(M_Mesh &mesh);
		// 计算顶点在模型空间的位置 -> 放在Update函数中调用
		void ComputerVertPos(M_Mesh &mesh, const MD5_Animation::X_FrameSkeleton& skeleton);
		// 走帧逻辑 在opengl 渲染循环中调用
		void Update(float deltaTime);
		// 渲染模型（的所有mesh）
		void Render(xShader shader);
		// 动画类
		MD5_Animation animation;	

	private:
		int numJoints;	// 骨骼数量
		int numMeshs;	// mesh数量
		JointList jointList;	// 骨骼数组
		MeshList meshList;		// mesh数组

	};


}




#endif // !MODEL_MD5_MESH_H_