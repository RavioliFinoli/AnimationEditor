#include "Animation.h"

AE::AnimationHandler::AnimationHandler()
{

}
AE::AnimationHandler::~AnimationHandler()
{

}

bool AE::AnimationHandler::loadAnimation(std::string file, std::string name, ANIMATION_TYPE type, SharedSkeleton skeleton)
{
	auto animation = Animation::LoadAndCreateAnimation(file, skeleton);

	SharedAnimationClip clip = std::make_shared<AnimationClip>();
	clip->SetAnimationData(animation);
	clip->SetSkeleton(skeleton);
	clip->SetSpeed(1.0f);

	switch (type)
	{
	case AE::ANIMATION_TYPE::AUTO:
		break;
	case AE::ANIMATION_TYPE::RAW_CLIP:
		m_rawClips.insert(std::make_pair(name, clip));
		break;
	case AE::ANIMATION_TYPE::DIFFERENCE_CLIP:
		m_differenceClips.insert(std::make_pair(name, clip));
		break;
	case AE::ANIMATION_TYPE::BAKED_CLIP:
		m_bakedClips.insert(std::make_pair(name, clip));
		break;
	default:
		m_rawClips.insert(std::make_pair(name, clip));
		break;
	}
	return true;
}

bool AE::AnimationHandler::loadSkeleton(std::string file, std::string name)
{
	auto skeleton = Animation::LoadAndCreateSkeleton(file);
	m_skeletons.insert(std::make_pair(name, skeleton));
	return true;
}

AE::SharedAnimationClip AE::AnimationHandler::getRawClip(std::string key)
{
	SharedAnimationClip clip = nullptr;
	try
	{
		clip = m_rawClips.at(key);
	}
	catch (std::out_of_range oor)
	{
		return nullptr;
	}
	return clip;
}

AE::SharedAnimationClip AE::AnimationHandler::getDifferenceClip(std::string key)
{
	SharedAnimationClip clip = nullptr;
	try
	{
		clip = m_differenceClips.at(key);
	}
	catch (std::out_of_range oor)
	{
		return nullptr;
	}
	return clip;
}

AE::SharedAnimationClip AE::AnimationHandler::getBakedClip(std::string key)
{
	SharedAnimationClip clip = nullptr;
	try
	{
		clip = m_bakedClips.at(key);
	}
	catch (std::out_of_range oor)
	{
		return nullptr;
	}
	return clip;
}

AE::AnimationClip::AnimationClip()
{
}
AE::AnimationClip::~AnimationClip()
{
}

void AE::AnimationClip::SetSpeed(float speed)
{
	m_animationSpeed = speed;
}

void AE::AnimationClip::SetMask(float maskValue, uint32_t jointIndex)
{
	m_jointMask.at(jointIndex) = maskValue;
}

void AE::AnimationClip::SetSkeleton(SharedSkeleton skeleton)
{
	m_skeletonData = skeleton;
}

void AE::AnimationClip::SetAnimationData(SharedAnimationData animationData)
{
	m_animationData = animationData;
}

std::shared_ptr<AE::DifferenceClip> AE::AnimationClip::AsDifferenceClip()
{
	return std::dynamic_pointer_cast<AE::DifferenceClip>(shared_from_this());
}

std::shared_ptr<AE::BakedClip> AE::AnimationClip::AsBakedClip()
{
	return std::dynamic_pointer_cast<AE::BakedClip>(shared_from_this());
}
