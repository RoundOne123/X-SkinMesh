#include "model_md5anim_gpu.h"

namespace xMD5Loader
{
	MD5_GPU_Animation::MD5_GPU_Animation()
	{

	}

	MD5_GPU_Animation::~MD5_GPU_Animation()
	{

	}

	// ���ض����ļ�
	bool MD5_GPU_Animation::LoadAnimation(string &path)
	{
		if (path.substr(path.size() - 8, 8) != ".md5anim")
		{
			cout << "Only Load md5Anim Animation " << path << endl;
			return false;
		}
		ifstream file(path);

		if (!file.is_open())
		{
			cout << "open file error " << path << endl;
			return false;
		}

		string param;
		string junk;

		jointInfos.clear();
		bounds.clear();
		baseFrames.clear();
		frames.clear();
		animatedSkeleton.jointList.clear();

		numFrame = 0;
		int length = GetFileLength(file);

		file >> param;
		while (!file.eof())
		{
			if (param == "MD5Version" || param == "commandline")
			{
				IgnoreLine(file, length);
			}
			else if (param == "numFrames")
			{
				file >> numFrame;
				IgnoreLine(file, length);
			}
			else if (param == "numJoints")
			{
				file >> numJoints;
				IgnoreLine(file, length);
			}
			else if (param == "frameRate")
			{
				file >> rate;
				IgnoreLine(file, length);
			}
			else if (param == "numAnimatedComponents")
			{
				file >> numComponents;
				IgnoreLine(file, length);
			}
			else if (param == "hierarchy")
			{
				file >> junk;
				// �����������
				for (int i = 0; i < numJoints; i++)
				{
					JointInfo jointInfo;
					file >> jointInfo.name >> jointInfo.parentID >> jointInfo.flag >> jointInfo.startIndex;
					IgnoreLine(file, length);
					RemoveNotes(jointInfo.name);

					// !!!!!!!!!!!!!!!!
					jointInfos.push_back(jointInfo);
				}
				file >> junk;
			}
			else if (param == "bounds")
			{
				file >> junk;
				IgnoreLine(file, length);

				// ģ���ڶ������̵ı߽��
				for (int i = 0; i < numFrame; i++)
				{
					Bound bound;
					file >> junk >> bound.min.x >> bound.min.y >> bound.min.z >> junk >> junk >> bound.max.x >> bound.max.y >> bound.max.z;
					IgnoreLine(file, length);

					// !!!!!!!!!!!!!!!!!!
					bounds.push_back(bound);
				}

				file >> junk;
				IgnoreLine(file, length);

			}
			else if (param == "baseframe")
			{
				file >> junk;
				IgnoreLine(file, length);
				for (int i = 0; i < numJoints; i++)
				{
					BaseFrame baseFrame;
					file >> junk >> baseFrame.pos.x >> baseFrame.pos.y >> baseFrame.pos.z >> junk >> junk >> baseFrame.orient.x >> baseFrame.orient.y >> baseFrame.orient.z;
					IgnoreLine(file, length);

					// !!!!!!!!!!!!!!!!!!!!!!!!!
					baseFrames.push_back(baseFrame);
				}
				file >> junk;
				IgnoreLine(file, length);
			}
			else if (param == "frame")
			{
				FrameData frame;
				file >> frame.frameID >> junk;
				IgnoreLine(file, length);

				// ����֡����
				for (int i = 0; i < numComponents; i++)
				{
					float iData;
					file >> iData;

					// !!!!!!!!!!!!!!!!!!!!
					frame.data.push_back(iData);
				}
				file >> junk;
				IgnoreLine(file, length);

				// !!!!!!!!!!!!!!!!!!!
				frames.push_back(frame);

				// ÿ֡�����ݱ����غ󣬵���BuildFrameSkeleton����
				// ������һ֡�����ݼ������й�����ģ�Ϳռ���λ�úͷ���
				// �������ļ��������ʱ�򣬾͵õ��˶�����ÿһ֡���й�����ģ�Ϳռ��λ�úͷ���
				BuildFrameSkeleton(skeletons, jointInfos, baseFrames, frame);
			}
			file >> param;
		}

		// ����Ĭ��֡�����й�������ģ�Ϳռ��λ�úͷ����Լ��任��������
		for (int i = 0; i < numJoints; i++)
		{
			SkeletonJoint joint;
			glm::mat4 matrix(1.0f);
			animatedSkeleton.jointList.push_back(joint);
			animatedSkeleton.boneMatrixs.push_back(matrix);
		}

		frameDuration = 1.0f / rate;
		animDuration = frameDuration * numFrame;
		animTime = 0;

		return true;
	}

	// ******** ��CPU������ÿ֡�Ķ���λ�ü�����������ݸ�GPU ********
	// ������һ֡�����ݼ������й�����ģ�Ϳռ���λ�úͷ���
	void MD5_GPU_Animation::BuildFrameSkeleton(FrameSkeletonList& skeletons, JointInfoList& jointInfos, BaseFrameList& baseFrames, FrameData& frameData)
	{
		// ĳһ֡�����й���������
		FrameSkeleton skeleton;
		// ��������ԭʼ��������
		for (unsigned int i = 0; i < jointInfos.size(); i++)
		{
			unsigned int j = 0;

			// ����֡���ݣ��õ������ڸ���������ռ��е�λ�úͷ���
			const JointInfo& jointInfo = jointInfos[i];	// ����iԭʼ����
			// ����i�ڸ������ռ��λ�úͷ������� ��ʼֵΪĬ��baseframe ��Ӧ��Ĭ��ֵ
			SkeletonJoint animatedJoint = baseFrames[i];
			animatedJoint.parentID = jointInfo.parentID;

			// ȡflag���6λ ���� ���Ӧ��ÿ֡�еĸ�����  ʾ�������ļ��� 63  ����Ӧ������6�������� ����pos��orient
			if (jointInfo.flag & 1)
			{
				animatedJoint.pos.x = frameData.data[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flag & 2)
			{
				animatedJoint.pos.y = frameData.data[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flag & 4)
			{
				animatedJoint.pos.z = frameData.data[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flag & 8)
			{
				animatedJoint.orient.x = frameData.data[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flag & 16)
			{
				animatedJoint.orient.y = frameData.data[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flag & 32)
			{
				animatedJoint.orient.z = frameData.data[jointInfo.startIndex + j];
				j++;
			}

			// ���������ģ�Ϳռ��е�λ�úͷ���
			ComputeQuatW(animatedJoint.orient);
			if (animatedJoint.parentID >= 0)
			{
				/*
				ÿ��������ģ�Ϳռ��λ����Ҫ���ݹ������һ�������������õ�
				����ļ��㶼��ֱ��������ˣ�������ͨ��������б任
				������ �� model_md5mesh.cpp ��ʹ�þ���ļ��㷽ʽ�ȼ�
				// ��������
				glm::mat4 boneMatrix(1.0f);
				glm::mat4 boneTranstion = glm::translate(boneMatrix, parentJoint.pos);
				glm::mat4 boneRotate = glm::mat4(parentJoint.orient);

				boneMatrix = boneTranstion * boneRotate;
				animatedJoint.pos = glm::vec3(boneMatrix * glm::vec4(animatedJoint.pos, 1.0));
				*/

				// ������  �����ļ��Ĺ��������ǰ��չ�����Σ���root��Ҷ�ӣ����ģ����������ܻ�ȡ�� ������
				SkeletonJoint &parentJoint = skeleton.jointList[animatedJoint.parentID];
				// �������յ�λ�� ��һ������õ���
				// �ù�����ģ�Ϳռ��λ�� = ��������λ�� + �������ķ�����ת�� * ��ǰ�������ڸ�������λ��
				glm::vec3 rotPos = parentJoint.orient * animatedJoint.pos;		// �൱�ڣ������任���������ռ�
				animatedJoint.pos = parentJoint.pos + rotPos;					// �൱�ڣ������任��ģ�Ϳռ�

				// �ù�����ģ�Ϳռ����ת = ����������ת * ��ǰ�����ڸ������е���ת �ٽ��й�һ��
				animatedJoint.orient = parentJoint.orient * animatedJoint.orient;
				animatedJoint.orient = glm::normalize(animatedJoint.orient);
			}

			// !!!!!!!!!!!!!!!!!!!!!!!!
			skeleton.jointList.push_back(animatedJoint);
		}
		// !!!!!!!!!!!!!!!!!!!!!!!
		skeletons.push_back(skeleton);
	}

	// �����ڵ���֡�����в�ֵ���������ˢ��ʱ���й�����ģ�Ϳռ��е�λ��
	// fInterpolate����ֵ��
	void MD5_GPU_Animation::InterpolateSkeletons(FrameSkeleton& finalSkeleton, FrameSkeleton& skeleton0, FrameSkeleton& skeleton1, float fInterpolate)
	{
		for (int i = 0; i < numJoints; i++)
		{
			SkeletonJoint& joint0 = skeleton0.jointList[i];
			SkeletonJoint& joint1 = skeleton1.jointList[i];
			SkeletonJoint& finalJoint = finalSkeleton.jointList[i];

			finalJoint.parentID = joint0.parentID;
			finalJoint.pos = joint0.pos * (1 - fInterpolate) + joint1.pos * fInterpolate;
			finalJoint.orient = glm::mix(joint0.orient, joint1.orient, fInterpolate);

			// ���ݲ�ֵ��λ�úͷ���  �����ֵ�ı任����
			glm::mat4 finalMart(1.0f);
			// ��ǰ�����ռ� �� ģ�Ϳռ�ñ任����
			finalMart = glm::translate(finalMart, finalJoint.pos) * glm::mat4(finalJoint.orient);
			finalSkeleton.boneMatrixs[i] = finalMart;
		}
	}

	// ��֡  ��Ҫ�ǲ�ֵ��������
	void MD5_GPU_Animation::Update(float deltaTime)
	{
		animTime += deltaTime;
		while (animTime > animDuration)
		{
			animTime -= animDuration;
		}

		// ����Ӧ�������в�����
		while (animTime < 0)
		{
			// cout << "��Ӧ�ò���ִ�� ������" << endl;
			animTime += animDuration;
		}

		float framePro = animTime * rate;	// ���ݵ�ǰʱ�� ����õ�һ��֡�� ��ͨ��������׼ȷ��ĳ������֡ ��Ҫ���в�ֵ��
		// ��ǰ��������֡��ֵ
		int lowFrame = (int)floorf(framePro);
		int highFrame = (int)ceilf(framePro);
		lowFrame = lowFrame % numFrame;
		highFrame = highFrame % numFrame;

		// ����ط���ֵ������һ֡�������ݶ��ڵ�ǰʱ�̵�ռ��
		// fmodf ������ �������ƽʱ�������������� 
		float fInterpolate = fmodf(animTime, frameDuration) / frameDuration;
		InterpolateSkeletons(animatedSkeleton, skeletons[lowFrame], skeletons[highFrame], fInterpolate);
	}

	// ���ص�ǰʱ������еĹ���λ�úͷ���ģ�Ϳռ䣩
	const MD5_GPU_Animation::FrameSkeleton& MD5_GPU_Animation::GetSkeleton() const
	{
		return animatedSkeleton;
	}

}