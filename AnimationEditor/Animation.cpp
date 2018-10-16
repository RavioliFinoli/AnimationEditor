#include "Animation.h"
#include "AnimationEditorApplication.h"
#include <algorithm>
AE::AnimationHandler::AnimationHandler()
{

}
AE::AnimationHandler::~AnimationHandler()
{

}

bool AE::AnimationHandler::LoadAnimation(std::string file, std::string name, ANIMATION_TYPE type, SharedSkeleton skeleton)
{
	auto animation = Animation::LoadAndCreateAnimation(file, skeleton);

	SharedAnimationClip clip = std::make_shared<AnimationClip>();
	clip->SetAnimationData(animation);
	clip->SetSkeleton(skeleton);
	clip->SetSpeed(1.0f);
	clip->SetName(name);

	gAnimationClipNames.push_back(name);
	switch (type)
	{
	case AE::ANIMATION_TYPE::AUTO:
		break;
	case AE::ANIMATION_TYPE::RAW_CLIP:
		m_RawClips.insert(std::make_pair(name, clip));
		break;
	case AE::ANIMATION_TYPE::DIFFERENCE_CLIP:
		m_DifferenceClips.insert(std::make_pair(name, clip));
		break;
	case AE::ANIMATION_TYPE::BAKED_CLIP:
		m_BakedClips.insert(std::make_pair(name, clip));
		break;
	default:
		m_RawClips.insert(std::make_pair(name, clip));
		break;
	}
	return true;
}

bool AE::AnimationHandler::LoadSkeleton(std::string file, std::string name)
{
	auto skeleton = Animation::LoadAndCreateSkeleton(file);
	skeleton->name = name;
	m_Skeletons.insert(std::make_pair(name, skeleton));
	gSkeletonNames.push_back(name);
	return true;
}

AE::SharedAnimationClip AE::AnimationHandler::GetRawClip(std::string key)
{
	SharedAnimationClip clip = nullptr;
	try
	{
		clip = m_RawClips.at(key);
	}
	catch (std::out_of_range oor)
	{
		return nullptr;
	}
	return clip;
}

AE::SharedAnimationClip AE::AnimationHandler::GetDifferenceClip(std::string key)
{
	SharedAnimationClip clip = nullptr;
	try
	{
		clip = m_DifferenceClips.at(key);
	}
	catch (std::out_of_range oor)
	{
		return nullptr;
	}
	return clip;
}

AE::SharedAnimationClip AE::AnimationHandler::GetBakedClip(std::string key)
{
	SharedAnimationClip clip = nullptr;
	try
	{
		clip = m_BakedClips.at(key);
	}
	catch (std::out_of_range oor)
	{
		return nullptr;
	}
	return clip;
}

AE::SharedSkeleton AE::AnimationHandler::GetSkeleton(std::string key)
{
	return m_Skeletons.at(key);
}

std::string AE::AnimationHandler::GetNameOfSkeleton(SharedSkeleton skeleton)
{
	auto it = std::find_if(std::begin(m_Skeletons), std::end(m_Skeletons),
		[&skeleton](auto&& p) { return p.second == skeleton; });

	if (it != m_Skeletons.end())
		return it->first;
	else return "-";
}

void AE::AnimationHandler::AddDifferenceClip(std::string key, AE::SharedDifferenceClip clip)
{
	m_DifferenceClips.insert(std::make_pair(key, clip));
}

std::unordered_map<std::string, AE::SharedSkeleton>& AE::AnimationHandler::GetSkeletonMap()
{
	return m_Skeletons;
}

std::unordered_map<std::string, AE::SharedAnimationClip>& AE::AnimationHandler::GetRawClipMap()
{
	return m_RawClips;
}

AE::AnimationClip::AnimationClip()
{
}
AE::AnimationClip::~AnimationClip()
{
}

void AE::AnimationClip::SetSpeed(float speed)
{
	m_AnimationSpeed = speed;
}

void AE::AnimationClip::SetMask(float maskValue, uint32_t jointIndex)
{
	m_JointMask.at(jointIndex) = maskValue;
}

void AE::AnimationClip::SetSkeleton(SharedSkeleton skeleton)
{
	m_SkeletonData = skeleton;
}

void AE::AnimationClip::SetName(std::string name)
{
	m_Name = name;
}

void AE::AnimationClip::SetAnimationData(SharedAnimationData animationData)
{
	m_AnimationData = animationData;
	if (animationData->m_skeleton)
		m_SkeletonData = animationData->m_skeleton;
}

AE::SharedSkeleton AE::AnimationClip::GetSkeleton()
{
	return m_SkeletonData;
}

uint32_t AE::AnimationClip::GetFrameCount()
{
	return m_AnimationData->m_frameCount;
}

std::shared_ptr<AE::DifferenceClip> AE::AnimationClip::AsDifferenceClip()
{
	auto poop = shared_from_this();
	return std::dynamic_pointer_cast<AE::DifferenceClip>(shared_from_this());
}

std::shared_ptr<AE::BakedClip> AE::AnimationClip::AsBakedClip()
{
	return std::dynamic_pointer_cast<AE::BakedClip>(shared_from_this());
}

Animation::SkeletonPose & AE::AnimationClip::operator[](size_t index)
{
	return m_AnimationData->m_skeletonPoses[index];
}

Animation::SkeletonPose& AE::AnimationClip::GetSkeletonPose(int index)
{
	return m_AnimationData->m_skeletonPoses[index];
}

std::string AE::AnimationClip::GetName() const
{
	return m_Name;
}

AE::SharedAnimationData AE::AnimationClip::GetAnimationData()
{
	return m_AnimationData;
}

Animation::JointPose getDifferencePose(Animation::JointPose sourcePose, Animation::JointPose referencePose)
{
	using namespace DirectX;

	XMMATRIX sourceMatrix = Animation::_createMatrixFromSRT(sourcePose.m_transformation);
	XMMATRIX referenceMatrix = Animation::_createMatrixFromSRT(referencePose.m_transformation);
	XMMATRIX referenceMatrixInverse = XMMatrixInverse(nullptr, referenceMatrix);
	XMMATRIX differenceMatrix = XMMatrixMultiply(sourceMatrix, referenceMatrixInverse);

	Animation::SRT differencePose = {};
	XMVECTOR s, r, t;
	XMMatrixDecompose(&s, &r, &t, differenceMatrix);
	XMStoreFloat4A(&differencePose.m_scale, s);
	XMStoreFloat4A(&differencePose.m_rotationQuaternion, r);
	XMStoreFloat4A(&differencePose.m_translation, t);

	return Animation::JointPose(differencePose);
}

AE::SharedAnimationData AE::MakeNewDifferenceClip(AE::SharedAnimationClip sourceClip, AE::SharedAnimationClip referenceClip)
{
	return MakeNewDifferenceClip(sourceClip->GetAnimationData(), referenceClip->GetAnimationData());
}

DirectX::XMMATRIX AE::GetBindpose(DirectX::XMFLOAT4X4A inverseBindPose)
{
	using namespace DirectX;

	return XMMatrixInverse(nullptr, XMLoadFloat4x4A(&inverseBindPose));

}

DirectX::XMMATRIX AE::GetBindpose(DirectX::XMMATRIX inverseBindPose)
{
	using namespace DirectX;

	return XMMatrixInverse(nullptr, inverseBindPose);
}

void AE::BakeOntoBindpose(AE::SharedDifferenceClip animation)
{
	using namespace DirectX;
	std::vector<XMMATRIX> bindPoses;

	auto skeleton = animation->GetSkeleton();
	auto frameCount = animation->GetFrameCount();
	auto jointCount = skeleton->m_jointCount;
	auto animationData = animation->GetAnimationData();

	for (int skelPose = 0; skelPose < frameCount; skelPose++)
	{
		for (int joint = 0; joint < jointCount; joint++)
		{
			if (skelPose == 0) //get bindposes on first iteration
			{
				bindPoses.push_back(GetBindpose(skeleton->m_joints[joint].m_inverseBindPose));
				if (skeleton->m_joints[joint].parentIndex >= 0)
				{
					bindPoses.back() = XMMatrixMultiply(bindPoses.back(), XMLoadFloat4x4A(&skeleton->m_joints[skeleton->m_joints[joint].parentIndex].m_inverseBindPose));
				}
			}
			
			// #checkOrder
			auto oldPose = _createMatrixFromSRT(animationData->m_skeletonPoses[skelPose].m_jointPoses[joint].m_transformation);
			XMMATRIX newPoseMatrix = XMMatrixMultiply(bindPoses[joint], _createMatrixFromSRT(animationData->m_skeletonPoses[skelPose].m_jointPoses[joint].m_transformation));

			Animation::SRT newPose = {};
			XMVECTOR t, r, s;
			XMMatrixDecompose(&s, &r, &t, newPoseMatrix);
			XMStoreFloat4A(&newPose.m_scale, s);
			XMStoreFloat4A(&newPose.m_rotationQuaternion, r);
			XMStoreFloat4A(&newPose.m_translation, t);
			animationData->m_skeletonPoses[skelPose].m_jointPoses[joint] = Animation::JointPose(newPose);
		}
	}
}

AE::SharedAnimationData AE::MakeNewDifferenceClip(AE::SharedAnimationData sourceClip, AE::SharedAnimationData referenceClip)
{
	AE::SharedAnimationData animationToReturn = std::make_shared<Animation::AnimationClip>();
	animationToReturn->m_frameCount = sourceClip->m_frameCount;
	animationToReturn->m_framerate = sourceClip->m_framerate;
	animationToReturn->m_skeleton = sourceClip->m_skeleton;
	animationToReturn->m_skeletonPoses = std::make_unique<Animation::SkeletonPose[]>(animationToReturn->m_frameCount);

	//Go through each skeleton pose and set new difference pose for each joint
	for (int frame = 0; frame < animationToReturn->m_frameCount; frame++)
	{
		//Init joint pose array for this skeleton pose
		animationToReturn->m_skeletonPoses[frame].m_jointPoses = std::make_unique<Animation::JointPose[]>(animationToReturn->m_skeleton->m_jointCount);

		//Go through each joint and assign the difference pose
		for (int jointPose = 0; jointPose < animationToReturn->m_skeleton->m_jointCount; jointPose++)
		{
			auto sourcePose = sourceClip->m_skeletonPoses[frame].m_jointPoses[jointPose];
			auto referencePose = referenceClip->m_skeletonPoses[frame].m_jointPoses[jointPose];
			auto differencePose = getDifferencePose(sourcePose, referencePose);

			animationToReturn->m_skeletonPoses[frame].m_jointPoses[jointPose] = differencePose;
		}
	}

	return animationToReturn;
}

AE::DifferenceClip::DifferenceClip()
{
}

AE::DifferenceClip::~DifferenceClip()
{
}
