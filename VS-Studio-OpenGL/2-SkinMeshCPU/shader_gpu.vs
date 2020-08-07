#version 330 core

#define Max_Bones 58	// ��������
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
	vec4 pos = ComputeVertexByBoneMatrix();	// ��Ƥ����
	gl_Position = projection * view * model * pos;
}


vec4 ComputeVertexByBoneMatrix()
{
	// ��� ����ǰ�������ռ䵽ģ�Ϳռ�ı任����* �������¡�ģ�Ϳռ䵽�����ռ�ı任����* Ȩ�� ���ۼ�
	mat4 matTrans = boneMatrixs[int(boneIndexs.x)] * boneWeights.x;
	matTrans += boneMatrixs[int(boneIndexs.y)] * boneWeights.y;
	matTrans += boneMatrixs[int(boneIndexs.z)] * boneWeights.z;
	float finalWeight = 1.0f - boneWeights.x - boneWeights.y - boneWeights.z;	// ����w��Ȩ��Ҫͨ������õ���ȷ������Ȩ��֮��Ϊ1
	matTrans += boneMatrixs[int(boneIndexs.w)] * finalWeight;

	// �ü�Ȩƽ����õ������յı任���� * �������¶�����ģ�Ϳռ��λ�� => ��ǰ ������ģ�Ϳռ��µ�λ��
	vec4 pos = matTrans * vec4(aPos, 1.0);		// �����aPos ֻ�ᴫ��һ��
	return pos;
}