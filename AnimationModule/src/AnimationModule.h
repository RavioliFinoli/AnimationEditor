#pragma once
#pragma warning(disable : 4244)
#include <string>
#include <vector>
#include <DirectXMath.h>
#include <d3d11.h>
#include <unordered_map>
#include "importer/FormatHeader.h"

namespace Animation
{
#pragma region AnimatedModelClassStructs
	// Struct containing rotation, translation and scale as XMFLOAT4A, in that order
	struct SRT
	{
		DirectX::XMFLOAT4A m_rotationQuaternion;
		DirectX::XMFLOAT4A m_translation;
		DirectX::XMFLOAT4A m_scale;

		SRT() {};
		SRT(const MyLibrary::Transform& transform);
		SRT(const MyLibrary::DecomposedTransform& transform);
		bool operator==(const SRT& other);
	};

	// Struct containing a joint as XMFLOAT4X4A and a parent index as an int16_t, in that order
	struct Joint
	{
		DirectX::XMFLOAT4X4A m_inverseBindPose;
		int16_t parentIndex;

		Joint() {};
	};

	// Struct containing a joint count as a uint8_t and an array as a unique_ptr, in that order
	struct Skeleton
	{
		uint8_t m_jointCount;
		std::unique_ptr<Joint[]> m_joints;
		std::string name;
		Skeleton() {};
		Skeleton(const MyLibrary::Skeleton& skeleton);
	};

	// Struct containing rot, trans, scale as an SRT struct
	struct JointPose
	{
		SRT m_transformation;

		JointPose() {};
		JointPose(const SRT& srt);
	};

	// Struct containing an array of JointPose structs as a unique_ptr
	struct SkeletonPose
	{
		std::unique_ptr<JointPose[]> m_jointPoses;
	};

	// Struct containing all necessary data for an animation for a given skeleton
	struct AnimationClip
	{
		std::shared_ptr<Skeleton> m_skeleton;
		uint16_t m_frameCount;
		std::unique_ptr<SkeletonPose[]> m_skeletonPoses;
		uint8_t m_framerate;

		AnimationClip() {};
		AnimationClip(const MyLibrary::AnimationFromFileStefan& animation, std::shared_ptr<Skeleton> skeleton);
		~AnimationClip();
	};
	typedef std::unordered_map<std::string, std::shared_ptr<Animation::AnimationClip>> ANIMATION_MAP;
#pragma endregion Joint, Skeleton, AnimationClip, ...

#pragma region GlobalAnimationFunctions
	SRT ConvertTransformToSRT(MyLibrary::Transform transform);
	Animation::AnimationClip* ConvertToAnimationClip(MyLibrary::AnimationFromFile* animation, uint8_t jointCount);
	void SetInverseBindPoses(Animation::Skeleton* mainSkeleton, const MyLibrary::Skeleton* importedSkeleton);
	DirectX::XMMATRIX _createMatrixFromSRT(const SRT& srt);
	DirectX::XMMATRIX _createMatrixFromSRT(const MyLibrary::DecomposedTransform& transform);
	std::shared_ptr<Animation::AnimationClip> LoadAndCreateAnimation(std::string file, std::shared_ptr<Animation::Skeleton> skeleton);
	std::shared_ptr<Animation::Skeleton> LoadAndCreateSkeleton(std::string file);

#pragma endregion Conversion stuff, Loaders, ...
}


