#include "Animation.h"

AE::AnimationHandler::AnimationHandler()
{

}
AE::AnimationHandler::~AnimationHandler()
{

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

void AE::AnimationClip::SetSkeleton(std::shared_ptr<SkeletonData> skeleton)
{
	m_skeletonData = skeleton;
}

std::shared_ptr<AE::DifferenceClip> AE::AnimationClip::AsDifferenceClip()
{
	return std::dynamic_pointer_cast<AE::DifferenceClip>(shared_from_this());
}

std::shared_ptr<AE::BakedClip> AE::AnimationClip::AsBakedClip()
{
	return std::dynamic_pointer_cast<AE::BakedClip>(shared_from_this());
}
