#ifndef MODEL_MD5_ANIM_H_
#define MDDEL_MD5_ANIM_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "xTools.h"

using namespace xTools;
using namespace std;

namespace xMD5Loader
{
	


	// ����  �����ļ�ֻ��ÿ֡�ı� ������λ��  �����ı䶥���λ��
	class MD5_Animation
	{
	public:
#pragma region �ṹ�嶨��
		// Ĭ�Ϲ���  ->  ��Ӧ hierarchy �е�����
		struct X_JointInfo
		{
			string name;	// ������
			int parentID;	// �������ڲ�νṹ�е�����
			int flag;		// ��־λ ������֡����Ҫ���Ӧ���ڹ�����λ���Լ�������
			int startIndex;	// ��������Ҫ��������֡�����е�����
		};
		typedef vector<X_JointInfo> JointInfoList;

		// ģ���ڶ��������еı߽��  ->  ��Ӧ bounds �е�����
		struct X_Bound
		{
			// �����γ�����ά�ռ��һ��������
			glm::vec3 min;		// ��ǰ֡��ģ�ͱ߽����ģ�Ϳռ����Сֵ
			glm::vec3 max;		// ��ǰ֡��ģ�ͱ߽����ģ�Ϳռ�����ֵ
		};
		typedef vector<X_Bound> BoundList;

		// ����Ĭ��λ�÷���		->  ��Ӧ baseframe �е�����
		struct X_BaseFrame
		{
			glm::vec3 pos;			// ÿ���������丸��������ռ��е�λ��
			glm::vec3 orient;		// ����������ռ��е���ת
		};
		typedef vector<X_BaseFrame> BaseFrameList;

		// ֡����				->  ��Ӧ frame �е�����
		// һϵ�и���������¼�˵�ǰ֡���й������丸��������ռ��λ�úͷ���
		// ��Щ����ֵ��ÿ������ֵ����ʲô���� �Լ� �����ĸ�������������������� ���������е� flag ֵ�� ����ֵ������
		struct X_FrameData
		{
			int frameID;			// ��ǰ֡����
			vector<float> data;		// ��ǰ֡���� ��һϵ�и�������
		};
		typedef vector<X_FrameData> FrameDataList;

		// ��ĳһ֡��ĳһ��������λ�úͷ���		->		������������ ����ĵ�������
		struct X_SkeletonJoint
		{
			X_SkeletonJoint() :parentID(-1), pos(0) {}
			X_SkeletonJoint(const X_BaseFrame& copy) : pos(copy.pos), orient(copy.orient) {}

			int parentID;		// �������ڹ����ṹ����е�����
			glm::vec3 pos;		// ��ģ�Ϳռ��е�λ��
			glm::quat orient;	// ģ�Ϳռ��е���ת
		};
		typedef vector<X_SkeletonJoint> SkeletonJointList;

		// ĳһ֡���й�����λ�úͷ���
		struct X_FrameSkeleton
		{
			SkeletonJointList jointList;
		};
		typedef vector<X_FrameSkeleton> FrameSkeletonList;

#pragma endregion

		MD5_Animation();
		~MD5_Animation();
		// ���ض����ļ�
		bool LoadAnimation(string &path);
		// ��֡  ��Ҫ�ǲ�ֵ��������
		void Update(float deltaTime);
		// ���ص�ǰʱ������еĹ���λ�úͷ���ģ�Ϳռ䣩
		const X_FrameSkeleton& GetSkeleton() const;

	private:
		int numFrame;			// ����֡��
		int numJoints;			// ������
		int rate;				// ֡��
		int numComponents;		// ֡���ݸ���ֵ����
		float animDuration;		// ����ʱ�� = numFrame / rate
		float frameDuration;	// һ֡��ʱ�� = 1 / rate
		float animTime;			// �����Ѳ���ʱ��

		JointInfoList jointInfos;		// ����ԭʼ����
		BoundList bounds;				// �߿�����
		BaseFrameList baseFrames;		// ���й����ڸ������ռ�Ĭ�ϵ�λ�úͷ�������
		FrameDataList frames;			// ����֡���ݣ���������Щ��
		X_FrameSkeleton animatedSkeleton;	// ��ǰʱ���Ĺ���λ�úͷ���ģ�Ϳռ䣩
		FrameSkeletonList skeletons;	// ����֡�Ĺ���λ�úͷ���	��ģ�Ϳռ䣩
		

		// ������һ֡�����ݼ������й�����ģ�Ϳռ���λ�úͷ���
		void BuildFrameSkeleton(FrameSkeletonList& skeletons, JointInfoList& jointInfos, BaseFrameList& baseFrames, X_FrameData& frameData);
		// �����ڵ���֡�����в�ֵ���������ˢ��ʱ���й�����ģ�Ϳռ��е�λ��
		// fInterpolate����ֵ��
		void InterpolateSkeletons(X_FrameSkeleton& finalSkeleton, X_FrameSkeleton& skeleton0, X_FrameSkeleton& skeleton1, float fInterpolate);
	};
}

#endif // !MODEL_MD5_ANIM_H_

