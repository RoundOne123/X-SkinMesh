
// ����CPU�Ĺ�����Ƥ������ÿһ֡����Ҫ������õĶ���λ�ô�CPU���͵�GPU ��һ�����������������ĵ�
// �ڿɱ����Ⱦ���߳���֮ǰ ����ֻ����ô�� 
// ���ǿɱ����Ⱦ���ߵĳ��� ʹ���ǿ��Խ�������Ƥ���̷ŵ� GPU��ʵ�֣�����GPU�Ĺ�����Ƥ����
// GPU�Ĺ�����Ƥ����������ֻҪÿ֡�������й����ı任����



	/*
		����GPU��ʵ��˼·��
			������ȷ���󶨵�����֮�����ж��������������ġ������ռ䡿��λ�þͲ����ٱ仯�ˣ�
			���ԣ�
			���ǽ����������¡�����ġ�ģ�Ϳռ�λ�á����ݵ�������ɫ����
			�ٽ����������¡�ģ�Ϳռ䵽�������ռ�ġ��任���󡿴��ݵ�������ɫ����
			Ȼ�����������ڡ������ռ��µ�λ�á��������棬�Ͳ����ڱ��ˣ�
			Ȼ����Update�������ټ������ǰʱ�̡����й����ռ䵽ģ�Ϳռ�ı仯���󡿣�������ÿ֡Ҫ���ݸ�GPU�����ݣ�
			���վͿ���������Щ�仯���󣬼������ǰ�ĸ�������ģ�Ϳռ��е�λ���ˡ�������������Ҫ��ֵ��
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

#pragma region �ṹ������
		// ��������
		struct Vertex
		{
			glm::vec3 pos;			// ������ģ�Ϳռ��е�λ��
			glm::vec2 texcoord;		// ��������
			int startWeightIndex;	// ��������Ȩ����ʼ����
			int weightCount;		// ��������Ȩ������

			// �ڶ�����ɫ���м��㶥��λ�� -> ��Ҫ�����������Ĺ�������ȫ���ϴ���GPU

			glm::vec4 boneWeights;	// ��������Ȩ��ֵ		-> ��������������ĸ�Ȩ��
			glm::vec4 boneIndexs;	// ��������Ȩ�أ������Ĺ���������  -> Ҳ���ǣ���ӣ������Ĺ���������
		};
		typedef vector<Vertex> VertexList;

		// �ؽڡ�����
		struct Joint
		{
			string name;		// ��������
			int parent_ID;		// �������ڹ�����νṹ�е�����
			vec3 pos;			// ��ʼ����ʱ������ģ�Ϳռ��е�λ��
			quat orient;		// ��ʼ����ʱ��������ռ����ģ�Ϳռ����ת
		};
		typedef vector<Joint> JointList;


		// Ȩ��
		struct Weight
		{
			int joint_ID;		// ���Ȩ�ع����Ĺ����ڹ�����νṹ�е�����
			float bias;			// Ȩ��ռ��
			vec3 pos;			// Ȩ������������������ռ��λ��
		};
		typedef vector<Weight> WeightList;
		typedef vector<unsigned int> IndexBuffer;

		typedef vector<glm::mat4> MartrixList;

		typedef vector<glm::vec3> PositionBuffer;
		typedef vector<glm::vec3> NormalBuffer;

		// ����
		struct Mesh
		{
			string shader;					// ����
			unsigned int texID;				// ���������
			unsigned int VAO, VBO, EBO;		// ����������� ���㻺����������������
			VertexList verts;				// ��������
			WeightList weights;				// Ȩ������
			IndexBuffer indexBuffer;		// ��������		-> �� vert ��Ӧ�����ݵõ�

			//PositionBuffer posBuffer;		// λ�û���
			//NormalBuffer normalBuffer;		// ���߻���			����Ҳ��Ҫ������Ƥ����  ������ʱû����

		};
		typedef vector<Mesh> MeshList;
#pragma endregion


		MD5_Model_GPU();
		~MD5_Model_GPU();
		// ����ģ��
		bool LoadModel(string &path);
		// �������������������
		unsigned int LoadTexture(string &path);
		// �������黺����󡢶��㻺�����������
		void CreateVertBuffer(Mesh &mesh);
		// ���������ʱ���������� ģ�Ϳռ� �� �����ռ� �ı任����
		void BuildBindPose(JointList &jointList);
		// ��������Ƶ�mesh�����ж����λ�ã��������붥������Ĺ��������͹���Ȩ��
		void PrepareMesh(Mesh &mesh);
		// ��֡�߼� ��opengl ��Ⱦѭ���е���
		void Update(float deltaTime);
		// ��Ⱦģ�ͣ�������mesh��
		void Render(xShader shader);
		// ������
		MD5_GPU_Animation animation;


		//// ��������ռ䵽ģ�Ϳռ�ı任����  ->  �߼��� ֱ�ӷ���update��Ū�� δ�����һ���µķ���
		//void ComputerMatrix(Mesh &mesh, const MD5_GPU_Animation::FrameSkeleton& skeleton);

	private:
		int numJoints;					// ��������
		int numMeshs;					// mesh����
		JointList jointList;			// ��������
		MeshList meshList;				// mesh����	
		MartrixList inverseBindPose;	// ��������ģ�Ϳռ䵽�����ռ�ı任����

		MartrixList animatedBones;		//
	};

}

#endif // !MODEL_MD5_MESH_GPU_H_
