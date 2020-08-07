#version 330 core

#define Max_Bones 58	// 最大骨骼数
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexcoord;
layout (location = 2) in vec4 boneWeights;
layout (location = 3) in vec4 boneIndexs;

out vec2 Texcoord;

uniform mat4[Max_Bones] boneMatrixs;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec4 ComputeVertexByBoneMatrix();

void main()
{
	Texcoord = aTexcoord;
	vec4 pos = ComputeVertexByBoneMatrix();	// 蒙皮过程
	gl_Position = projection * view * model * pos;
}


vec4 ComputeVertexByBoneMatrix()
{
	// 获得 （当前【骨骼空间到模型空间的变换矩阵】* 绑定姿势下【模型空间到骨骼空间的变换】）* 权重 再累加
	mat4 matTrans = boneMatrixs[int(boneIndexs.x)] * boneWeights.x;
	matTrans += boneMatrixs[int(boneIndexs.y)] * boneWeights.y;
	matTrans += boneMatrixs[int(boneIndexs.z)] * boneWeights.z;
	float finalWeight = 1.0f - boneWeights.x - boneWeights.y - boneWeights.z;	// 这里w的权重要通过计算得到，确保最终权重之和为1
	matTrans += boneMatrixs[int(boneIndexs.w)] * finalWeight;

	// 用加权平均后得到的最终的变换矩阵 * 绑定姿势下顶点再模型空间的位置 => 当前 顶点在模型空间下的位置
	vec4 pos = matTrans * vec4(aPos, 1.0);		// 这里的aPos 只会传递一次
	return pos;
}