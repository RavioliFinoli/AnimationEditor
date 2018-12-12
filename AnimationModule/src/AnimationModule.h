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
	/*
	//Flips the Z axis before writing
	void appendFloat4AsDirectXVector(std::ofstream& file, const Vec4 vec)
	{
		if (!file.is_open())
			return;

		Vec4 v = vec;
		v.z *= -1.0f;

		file.write((const char*)&v, sizeof(Vec4));
	}
	//Flips X and Y components before writing
	void appendFloat4AsDirectXQuaternion(std::ofstream& file, const Vec4 vec)
	{
		if (!file.is_open())
			return;

		Vec4 v = vec;
		v.x *= -1.0f;
		v.y *= -1.0f;

		file.write((const char*)&v, sizeof(Vec4));
	}
	/// Writes each Vec4 component of the Transform (order is T, R, S)
	void appendTransform(std::ofstream& file, const DecomposedTransform transform)
	{
		if (!file.is_open())
			return;

		/// Write each vector (T, R, S) (R is quaternion)
		appendFloat4AsDirectXVector(file, transform.translation);
		appendFloat4AsDirectXQuaternion(file, transform.rotation);
		appendFloat4(file, transform.scale);
	}*/



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
	inline void appendAsDirectXTransform(std::ofstream& file, SRT transform)
	{
		transform.m_translation.z *= -1.0f;
		transform.m_rotationQuaternion.x *= -1.f;
		transform.m_rotationQuaternion.y *= -1.f;

		file.write((const char*)&transform.m_translation, sizeof(DirectX::XMFLOAT4A));
		file.write((const char*)&transform.m_rotationQuaternion, sizeof(DirectX::XMFLOAT4A));
		file.write((const char*)&transform.m_scale, sizeof(DirectX::XMFLOAT4A));

	}
	inline void appendAsRawTransform(std::ofstream& file, SRT transform)
	{

		file.write((const char*)&transform.m_translation, sizeof(DirectX::XMFLOAT4A));
		file.write((const char*)&transform.m_rotationQuaternion, sizeof(DirectX::XMFLOAT4A));
		file.write((const char*)&transform.m_scale, sizeof(DirectX::XMFLOAT4A));

	}

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
		JointPose(const DirectX::XMFLOAT4X4A& matrix);
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


