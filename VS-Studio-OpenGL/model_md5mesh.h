/*
������Ƥ������
	Mesh����Ҫ��Ƥ��Ϣ����Skin���ݣ�û��Skin���ݾ���һ����ͨ�ľ�̬Mesh�ˡ�
	Skin���ݾ���������ΰ󶨵������ϣ������Skin���ݰ�����������Щ����Ӱ���Լ���Щ����Ӱ��ö���ʱ��Ȩ��(weight)��
	�������ÿ������� ��Ҫ����ƫ�ƾ���(BoneOffsetMatrix)�����������Mesh�ռ�任�������ռ䡣
	����������Ƥ(����)�˶���������������˶��أ���Ȼ�Ƕ��������ˡ�
	ÿ���ؼ�֡�а���ʱ��͹����˶���Ϣ���˶���Ϣ���� ��һ������ֱ�ӱ�ʾ�����µı任��Ҳ������Ԫ����ʾ��������ת��
	Ҳ��������Լ�����ʲôֻҪ���ù��������С�����ʹ�ñ༭�趨�õĶ���֡���ݣ�Ҳ����ʹ�� �������Թ�������ʵʱ���ơ�
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



	// ģ��
	class MD5_Model
	{
#pragma region �ṹ�嶨��
		// �ؽڡ�����
		struct M_Joint
		{
			string name;		// ��������
			int parent_ID;		// �������ڹ�����νṹ�е�����
			vec3 pos;			// ��ʼ����ʱ������ģ�Ϳռ��е�λ��
			quat orient;		// ��ʼ����ʱ��������ռ����ģ�Ϳռ����ת
		};
		typedef vector<M_Joint> JointList;

		// ����
		struct M_Vertex
		{
			vec3 pos;			// ������ģ�Ϳռ��λ��	->	�����������ݼ���ó�
			vec2 texcoord;		// ��������
			int startWeight;	// ������Ȩ�ص���ʼ����
			int weightCount;	// ��������Ȩ������
		};
		typedef vector<M_Vertex> VertexList;

		// Ȩ��
		struct M_Weight
		{
			int joint_ID;		// ���Ȩ�ع����Ĺ����ڹ�����νṹ�е�����
			float bias;			// Ȩ��ռ��
			vec3 pos;			// Ȩ������������������ռ��λ��
		};
		typedef vector<M_Weight> WeightList;


		typedef vector<glm::vec3> PositionBuffer;
		typedef vector<glm::vec3> NormalBuffer;
		typedef vector<unsigned int> IndexBuffer;		// �������壿��

		// ����
		struct M_Mesh
		{
			string shader;		// ����
			unsigned int texID;	// ���������
			unsigned int VAO, VBO, EBO;		// ����������� ���㻺����������������
			VertexList verts;				// ��������
			WeightList weights;				// Ȩ������
			IndexBuffer indexBuffer;		// ��������		-> �� vert ��Ӧ�����ݵõ�

			PositionBuffer posBuffer;		// λ�û���
			NormalBuffer normalBuffer;		// ���߻���			����Ҳ��Ҫ������Ƥ����  ������ʱû����

		};
		typedef vector<M_Mesh> MeshList;

#pragma endregion


	public:
		MD5_Model();
		~MD5_Model();
		// ����ģ��
		bool LoadModel(string &path);
		// �������������������
		unsigned int LoadTexture(string &path);
		// �������黺����󡢶��㻺�����������
		void CreateVertBuffer(M_Mesh &mesh);
		// ���㶥����ģ�Ϳռ��λ�� -> ����Update�����е���
		void ComputerVertPos(M_Mesh &mesh, const MD5_Animation::X_FrameSkeleton& skeleton);
		// ��֡�߼� ��opengl ��Ⱦѭ���е���
		void Update(float deltaTime);
		// ��Ⱦģ�ͣ�������mesh��
		void Render(xShader shader);
		// ������
		MD5_Animation animation;	

	private:
		int numJoints;	// ��������
		int numMeshs;	// mesh����
		JointList jointList;	// ��������
		MeshList meshList;		// mesh����

	};


}




#endif // !MODEL_MD5_MESH_H_