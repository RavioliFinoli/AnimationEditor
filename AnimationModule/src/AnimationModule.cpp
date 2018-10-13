#include "AnimationModule.h"
#include "importer/formatImporter.h"


DirectX::XMMATRIX Animation::_createMatrixFromSRT(const SRT& srt)
{
	using namespace DirectX;

	XMFLOAT4A fScale = ( srt.m_scale);
	XMFLOAT4A fRotation = srt.m_rotationQuaternion;
	XMFLOAT4A fTranslation = srt.m_translation;				   

	auto translationMatrix = XMMatrixTranslationFromVector(XMLoadFloat4A(&fTranslation));
	auto rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4A(&fRotation));
	auto scaleMatrix = XMMatrixScalingFromVector(XMLoadFloat4A(&fScale));
	
	//return XMMatrixMultiply(rotationMatrix, translationMatrix); // #todo test/make sure we dont need this
	return XMMatrixAffineTransformation(XMLoadFloat4A(&fScale), { 0.0, 0.0, 0.0, 1.0 }, XMLoadFloat4A(&fRotation), XMLoadFloat4A(&fTranslation));
}

DirectX::XMMATRIX Animation::_createMatrixFromSRT(const MyLibrary::DecomposedTransform& transform)
{
	using namespace DirectX;

	XMFLOAT4A fScale = {transform.scale.x, transform.scale.y, transform.scale.z, transform.scale.w};
	XMFLOAT4A fRotation = { transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w };
	XMFLOAT4A fTranslation = { transform.translation.x, transform.translation.y, transform.translation.z, transform.translation.w };

	auto t = XMMatrixTranslationFromVector(XMLoadFloat4A(&fTranslation));
	auto r = XMMatrixRotationQuaternion(XMLoadFloat4A(&fRotation));
	auto s = XMMatrixScalingFromVector(XMLoadFloat4A(&fTranslation));

	// #NEWCHECK
	//return XMMatrixMultiply(t, r);

	return XMMatrixAffineTransformation(XMLoadFloat4A(&fScale), { 0.0f, 0.0f, 0.0f, 1.0f }, XMLoadFloat4A(&fRotation), XMLoadFloat4A(&fTranslation));
}

// #todo rename
std::shared_ptr<Animation::AnimationClip> Animation::LoadAndCreateAnimation(std::string file, std::shared_ptr<Animation::Skeleton> skeleton)
{
	MyLibrary::Loadera loader;
	auto importedAnimation = loader.readAnimationFile(file, skeleton->m_jointCount);

	return std::make_shared<Animation::AnimationClip>(importedAnimation, skeleton);
}

// #todo rename
std::shared_ptr<Animation::Skeleton> Animation::LoadAndCreateSkeleton(std::string file)
{
	MyLibrary::Loadera loader;
	auto importedSkeleton = loader.readSkeletonFile(file);
	return std::make_shared<Animation::Skeleton>(importedSkeleton);
}

// #convert Transform conversion
Animation::SRT Animation::ConvertTransformToSRT(MyLibrary::Transform transform)
{
	using namespace DirectX;
	SRT srt = {};
	float pitch = DirectX::XMConvertToRadians(transform.transform_rotation[0]);
	float yaw = DirectX::XMConvertToRadians(transform.transform_rotation[1]);
	float roll = DirectX::XMConvertToRadians(transform.transform_rotation[2]);
	XMStoreFloat4A(&srt.m_rotationQuaternion, XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));

	srt.m_rotationQuaternion.x *= -1;
	srt.m_rotationQuaternion.y *= -1;

	srt.m_translation = { transform.transform_position[0], transform.transform_position[1], transform.transform_position[2], 1.0f };
	srt.m_scale = { 1.0, 1.0, 1.0, 1.0f };
	return srt;
}

// #convert #animationclip AnimationClip conversion
Animation::AnimationClip* Animation::ConvertToAnimationClip(MyLibrary::AnimationFromFile* animation, uint8_t jointCount)
{
	uint32_t keyCount = animation->nr_of_keyframes;

	AnimationClip* clipToReturn = new AnimationClip();
	clipToReturn->m_frameCount = static_cast<uint16_t>(keyCount);
	clipToReturn->m_skeletonPoses = std::make_unique<SkeletonPose[]>(clipToReturn->m_frameCount);

	// Review
	//Init joint poses for skeleton poses
	for (int i = 0; i < clipToReturn->m_frameCount; i++)
	{
		clipToReturn->m_skeletonPoses[i].m_jointPoses = std::make_unique<JointPose[]>(jointCount);
	}

	for (int j = 0; j < jointCount; j++)
	{
		//for each key
		for (unsigned int k = 0; k < keyCount; k++)
		{
			// Review
			Animation::SRT trans = ConvertTransformToSRT(animation->keyframe_transformations[j * animation->nr_of_keyframes + k]);
			clipToReturn->m_skeletonPoses[k].m_jointPoses[j].m_transformation = trans;
		}
	}
	clipToReturn->m_framerate = 24;
	return clipToReturn;
}

// #bindpose
void Animation::SetInverseBindPoses(Animation::Skeleton* mainSkeleton, const MyLibrary::Skeleton* importedSkeleton)
{
	using namespace DirectX;

	std::vector<XMFLOAT4X4A> vec;
	std::vector <MyLibrary::Transform > vec2;
	DirectX::XMStoreFloat4x4A(&mainSkeleton->m_joints[0].m_inverseBindPose, _createMatrixFromSRT(importedSkeleton->joints[0].jointInverseBindPoseTransform));

	for (int i = 1; i < mainSkeleton->m_jointCount; i++)
	{
		const int16_t parentIndex = mainSkeleton->m_joints[i].parentIndex;
		assert(i > parentIndex);

		DirectX::XMStoreFloat4x4A
		(&mainSkeleton->m_joints[i].m_inverseBindPose, _createMatrixFromSRT(importedSkeleton->joints[i].jointInverseBindPoseTransform));
	}

}

Animation::SRT::SRT(const MyLibrary::Transform& transform)
{
	using namespace DirectX;
	
	float pitch = DirectX::XMConvertToRadians(transform.transform_rotation[0]);
	float yaw = DirectX::XMConvertToRadians(transform.transform_rotation[1]);
	float roll = DirectX::XMConvertToRadians(transform.transform_rotation[2]);
	XMStoreFloat4A(&m_rotationQuaternion, XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));

	m_rotationQuaternion.x *= -1;
	m_rotationQuaternion.y *= -1;

	m_translation = { transform.transform_position[0], transform.transform_position[1], transform.transform_position[2], 1.0f };
	m_scale = { 1.0, 1.0, 1.0, 1.0f };
}

Animation::SRT::SRT(const MyLibrary::DecomposedTransform& transform)
{
	m_rotationQuaternion.x = transform.rotation.x;
	m_rotationQuaternion.y = transform.rotation.y;
	m_rotationQuaternion.z = transform.rotation.z;
	m_rotationQuaternion.w = transform.rotation.w;

	m_translation.x = transform.translation.x;
	m_translation.y = transform.translation.y;
	m_translation.z = transform.translation.z;
	m_translation.w = transform.translation.w;

	m_scale.x = transform.scale.x;
	m_scale.y = transform.scale.y;
	m_scale.z = transform.scale.z;
	m_scale.w = transform.scale.w;
}

bool Animation::SRT::operator==(const SRT& other)
{
	return (m_rotationQuaternion.x == other.m_rotationQuaternion.x
		&& m_rotationQuaternion.y == other.m_rotationQuaternion.y
		&& m_rotationQuaternion.z == other.m_rotationQuaternion.z
		&& m_rotationQuaternion.w == other.m_rotationQuaternion.w
		&& m_scale.x == other.m_scale.x
		&& m_scale.z == other.m_scale.z
		&& m_scale.y == other.m_scale.y
		&& m_scale.w == other.m_scale.w
		&& m_translation.x == other.m_translation.x
		&& m_translation.z == other.m_translation.z
		&& m_translation.y == other.m_translation.y
		&& m_translation.w == other.m_translation.w
		);
}

Animation::Skeleton::Skeleton(const MyLibrary::Skeleton& skeleton)
{
	m_jointCount = static_cast<uint8_t>(skeleton.joints.size());
	m_joints = std::make_unique<Animation::Joint[]>(m_jointCount);

	for (int i = 0; i < m_jointCount; i++)
	{
		m_joints[i].parentIndex = skeleton.joints[i].parentIndex;
	}
	m_joints[0].parentIndex = -1; // Root does not have a real parent index
	Animation::SetInverseBindPoses(this, &skeleton);
}

Animation::JointPose::JointPose(const SRT& srt)
{
	m_transformation = srt;
}

Animation::AnimationClip::AnimationClip(const MyLibrary::AnimationFromFileStefan& animation, std::shared_ptr<Skeleton> skeleton)
{
	m_skeleton = skeleton;
	m_framerate = 24; //TODO maybe...
	uint32_t keyCount = animation.nr_of_keyframes;
	m_frameCount = static_cast<uint16_t>(keyCount);
	m_skeletonPoses = std::make_unique<SkeletonPose[]>(m_frameCount);

	//Init joint poses for skeleton poses
	for (int i = 0; i < m_frameCount; i++)
	{
		m_skeletonPoses[i].m_jointPoses = std::make_unique<JointPose[]>(m_skeleton->m_jointCount);
	}

	//for each key
	for (uint32_t k = 0; k < keyCount; k++)
	{
		for (uint8_t j = 0; j < m_skeleton->m_jointCount; j++)
		{
			// Review
			SRT srt = SRT(animation.keyframe_transformations[k * skeleton->m_jointCount + j]);
			m_skeletonPoses[k].m_jointPoses[j].m_transformation = srt;
		}
	}
}

Animation::AnimationClip::~AnimationClip()
{
}