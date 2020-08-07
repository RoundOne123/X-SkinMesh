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
	


	// 动画  动画文件只是每帧改变 骨骼的位置  进而改变顶点的位置
	class MD5_Animation
	{
	public:
#pragma region 结构体定义
		// 默认骨骼  ->  对应 hierarchy 中的数据
		struct X_JointInfo
		{
			string name;	// 骨骼名
			int parentID;	// 父骨骼在层次结构中的索引
			int flag;		// 标志位 描述了帧数据要如何应用在骨骼的位置以及方向上
			int startIndex;	// 骨骼所需要的数据在帧数据中的索引
		};
		typedef vector<X_JointInfo> JointInfoList;

		// 模型在动画过程中的边界框  ->  对应 bounds 中的数据
		struct X_Bound
		{
			// 两点形成在三维空间的一个长方体
			glm::vec3 min;		// 当前帧，模型边界框在模型空间的最小值
			glm::vec3 max;		// 当前帧，模型边界框在模型空间的最大值
		};
		typedef vector<X_Bound> BoundList;

		// 骨骼默认位置方向		->  对应 baseframe 中的数据
		struct X_BaseFrame
		{
			glm::vec3 pos;			// 每个骨骼在其父骨骼坐标空间中的位置
			glm::vec3 orient;		// 父骨骼坐标空间中的旋转
		};
		typedef vector<X_BaseFrame> BaseFrameList;

		// 帧数据				->  对应 frame 中的数据
		// 一系列浮点数，记录了当前帧所有骨骼在其父骨骼坐标空间的位置和方向
		// 这些浮点值中每个浮点值代表什么数据 以及 属于哪个骨骼则是由上面读到的 骨骼数据中的 flag 值和 索引值来决定
		struct X_FrameData
		{
			int frameID;			// 当前帧索引
			vector<float> data;		// 当前帧数据 （一系列浮点数）
		};
		typedef vector<X_FrameData> FrameDataList;

		// （某一帧，某一）骨骼的位置和方向		->		根据其他数据 计算的到的类型
		struct X_SkeletonJoint
		{
			X_SkeletonJoint() :parentID(-1), pos(0) {}
			X_SkeletonJoint(const X_BaseFrame& copy) : pos(copy.pos), orient(copy.orient) {}

			int parentID;		// 父骨骼在骨骼结构层次中的索引
			glm::vec3 pos;		// 在模型空间中的位置
			glm::quat orient;	// 模型空间中的旋转
		};
		typedef vector<X_SkeletonJoint> SkeletonJointList;

		// 某一帧所有骨骼的位置和方向
		struct X_FrameSkeleton
		{
			SkeletonJointList jointList;
		};
		typedef vector<X_FrameSkeleton> FrameSkeletonList;

#pragma endregion

		MD5_Animation();
		~MD5_Animation();
		// 加载动画文件
		bool LoadAnimation(string &path);
		// 走帧  主要是插值动画数据
		void Update(float deltaTime);
		// 返回当前时间点所有的骨骼位置和方向（模型空间）
		const X_FrameSkeleton& GetSkeleton() const;

	private:
		int numFrame;			// 动画帧数
		int numJoints;			// 骨骼数
		int rate;				// 帧率
		int numComponents;		// 帧数据浮点值数量
		float animDuration;		// 动画时长 = numFrame / rate
		float frameDuration;	// 一帧的时间 = 1 / rate
		float animTime;			// 动画已播放时长

		JointInfoList jointInfos;		// 骨骼原始数据
		BoundList bounds;				// 边框数据
		BaseFrameList baseFrames;		// 所有骨骼在父骨骼空间默认的位置和方向数据
		FrameDataList frames;			// 所有帧数据（浮点数那些）
		X_FrameSkeleton animatedSkeleton;	// 当前时间点的骨骼位置和方向（模型空间）
		FrameSkeletonList skeletons;	// 所有帧的骨骼位置和方向	（模型空间）
		

		// 根据这一帧的数据计算所有骨骼在模型空间中位置和方向
		void BuildFrameSkeleton(FrameSkeletonList& skeletons, JointInfoList& jointInfos, BaseFrameList& baseFrames, X_FrameData& frameData);
		// 从相邻的两帧数据中插值计算出画面刷新时所有骨骼在模型空间中的位置
		// fInterpolate：插值数
		void InterpolateSkeletons(X_FrameSkeleton& finalSkeleton, X_FrameSkeleton& skeleton0, X_FrameSkeleton& skeleton1, float fInterpolate);
	};
}

#endif // !MODEL_MD5_ANIM_H_

