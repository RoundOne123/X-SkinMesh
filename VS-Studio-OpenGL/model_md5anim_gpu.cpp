#include "model_md5anim_gpu.h"

namespace xMD5Loader
{
	MD5_GPU_Animation::MD5_GPU_Animation()
	{

	}

	MD5_GPU_Animation::~MD5_GPU_Animation()
	{

	}

	// 加载动画文件
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
				// 处理骨骼数据
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

				// 模型在动画过程的边界框
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

				// 处理帧数据
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

				// 每帧的数据被加载后，调用BuildFrameSkeleton函数
				// 根据这一帧的数据计算所有骨骼在模型空间中位置和方向
				// 这样在文件加载完的时候，就得到了动画的每一帧所有骨骼在模型空间的位置和方向
				BuildFrameSkeleton(skeletons, jointInfos, baseFrames, frame);
			}
			file >> param;
		}

		// 设置默认帧的所有骨骼（在模型空间的位置和方向以及变换矩阵）数据
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

	// ******** 在CPU将动画每帧的顶点位置计算出来，传递给GPU ********
	// 根据这一帧的数据计算所有骨骼在模型空间中位置和方向
	void MD5_GPU_Animation::BuildFrameSkeleton(FrameSkeletonList& skeletons, JointInfoList& jointInfos, BaseFrameList& baseFrames, FrameData& frameData)
	{
		// 某一帧的所有骨骼的数据
		FrameSkeleton skeleton;
		// 遍历所有原始骨骼数据
		for (unsigned int i = 0; i < jointInfos.size(); i++)
		{
			unsigned int j = 0;

			// 解析帧数据，得到骨骼在父骨骼坐标空间中的位置和方向
			const JointInfo& jointInfo = jointInfos[i];	// 骨骼i原始数据
			// 骨骼i在父骨骼空间的位置和方向数据 初始值为默认baseframe 对应的默认值
			SkeletonJoint animatedJoint = baseFrames[i];
			animatedJoint.parentID = jointInfo.parentID;

			// 取flag最后6位 决定 如何应用每帧中的浮点数  示例动画文件是 63  即会应用连续6个浮点数 当作pos和orient
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

			// 计算骨骼在模型空间中的位置和方向
			ComputeQuatW(animatedJoint.orient);
			if (animatedJoint.parentID >= 0)
			{
				/*
				每个骨骼再模型空间的位置是要根据骨骼层次一级级计算上来得到
				这里的计算都是直接向量相乘，并不是通过矩阵进行变换
				计算结果 和 model_md5mesh.cpp 中使用矩阵的计算方式等价
				// 声明矩阵
				glm::mat4 boneMatrix(1.0f);
				glm::mat4 boneTranstion = glm::translate(boneMatrix, parentJoint.pos);
				glm::mat4 boneRotate = glm::mat4(parentJoint.orient);

				boneMatrix = boneTranstion * boneRotate;
				animatedJoint.pos = glm::vec3(boneMatrix * glm::vec4(animatedJoint.pos, 1.0));
				*/

				// 父骨骼  动画文件的骨骼数据是按照骨骼层次（从root到叶子）来的，所以这里能获取到 父物体
				SkeletonJoint &parentJoint = skeleton.jointList[animatedJoint.parentID];
				// 这里最终的位置 是一层层计算得到的
				// 该骨骼在模型空间的位置 = 父骨骼的位置 + 父骨骼的方向（旋转） * 当前骨骼的在父骨骼的位置
				glm::vec3 rotPos = parentJoint.orient * animatedJoint.pos;		// 相当于（？）变换到父骨骼空间
				animatedJoint.pos = parentJoint.pos + rotPos;					// 相当于（？）变换到模型空间

				// 该骨骼在模型空间的旋转 = 父骨骼的旋转 * 当前骨骼在父骨骼中的旋转 再进行归一化
				animatedJoint.orient = parentJoint.orient * animatedJoint.orient;
				animatedJoint.orient = glm::normalize(animatedJoint.orient);
			}

			// !!!!!!!!!!!!!!!!!!!!!!!!
			skeleton.jointList.push_back(animatedJoint);
		}
		// !!!!!!!!!!!!!!!!!!!!!!!
		skeletons.push_back(skeleton);
	}

	// 从相邻的两帧数据中插值计算出画面刷新时所有骨骼在模型空间中的位置
	// fInterpolate：插值数
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

			// 根据插值的位置和方向  计算插值的变换矩阵
			glm::mat4 finalMart(1.0f);
			// 当前骨骼空间 到 模型空间得变换矩阵
			finalMart = glm::translate(finalMart, finalJoint.pos) * glm::mat4(finalJoint.orient);
			finalSkeleton.boneMatrixs[i] = finalMart;
		}
	}

	// 走帧  主要是插值动画数据
	void MD5_GPU_Animation::Update(float deltaTime)
	{
		animTime += deltaTime;
		while (animTime > animDuration)
		{
			animTime -= animDuration;
		}

		// 这里应该是运行不到的
		while (animTime < 0)
		{
			// cout << "我应该不会执行 ！！！" << endl;
			animTime += animDuration;
		}

		float framePro = animTime * rate;	// 根据当前时间 计算得到一个帧数 （通常不会是准确的某个整数帧 需要进行插值）
		// 求前后两整数帧的值
		int lowFrame = (int)floorf(framePro);
		int highFrame = (int)ceilf(framePro);
		lowFrame = lowFrame % numFrame;
		highFrame = highFrame % numFrame;

		// 这个地方插值就是下一帧骨骼数据对于当前时刻的占比
		// fmodf 求余数 这个不是平时理解的整数的余数 
		float fInterpolate = fmodf(animTime, frameDuration) / frameDuration;
		InterpolateSkeletons(animatedSkeleton, skeletons[lowFrame], skeletons[highFrame], fInterpolate);
	}

	// 返回当前时间点所有的骨骼位置和方向（模型空间）
	const MD5_GPU_Animation::FrameSkeleton& MD5_GPU_Animation::GetSkeleton() const
	{
		return animatedSkeleton;
	}

}