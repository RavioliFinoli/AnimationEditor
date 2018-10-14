#pragma once
#include <memory>
#include <vector>
#include <string>
#include <d3d11.h>
#include <wrl.h>
#include <unordered_map>
#include "../AnimationModule/src/AnimationModule.h"

using Microsoft::WRL::ComPtr;


namespace AE
{
#pragma region AnimationClip
	
	typedef std::shared_ptr<Animation::Skeleton> SharedSkeleton;
	typedef SharedSkeleton SharedSkeletonData;
	class DifferenceClip;
	class BakedClip;
	typedef std::shared_ptr<Animation::AnimationClip> SharedAnimationData;

	class AnimationClip : std::enable_shared_from_this<AnimationClip>
	{
	public:
		AnimationClip();
		virtual ~AnimationClip();

		void SetSpeed(float speed);
		void SetMask(float maskValue, uint32_t jointIndex);
		void SetSkeleton(SharedSkeleton skeleton);
		void SetAnimationData(SharedAnimationData animationData);

		std::shared_ptr<DifferenceClip> AsDifferenceClip();
		std::shared_ptr<BakedClip> AsBakedClip();
	private:
		SharedAnimationData m_AnimationData = nullptr;
		SharedSkeletonData m_SkeletonData = nullptr;
		std::vector<float> m_JointMask;
		float m_AnimationSpeed = 1.0f;
	};

	
	typedef AnimationClip RawClip;

	class DifferenceClip : public AnimationClip
	{
	public:
		DifferenceClip();
		virtual ~DifferenceClip();
	private:
		float m_BlendWeight = 1.0;
		std::shared_ptr<AnimationClip> m_OriginalSource = nullptr;
		std::shared_ptr<AnimationClip> m_OriginalReference = nullptr;

	};

	class BakedClip : public AnimationClip
	{
	public:
		BakedClip();
		virtual ~BakedClip();

	private:
		float m_BakedWeight = 1.0;
		std::shared_ptr<AnimationClip> m_OriginalClip = nullptr;
		std::shared_ptr<AnimationClip> m_OriginalDifferenceClip = nullptr;
	};

	typedef std::shared_ptr<AE::AnimationClip> SharedAnimationClip;
#pragma endregion AnimationClip base and derived

#pragma region AnimationHandler

	typedef std::unordered_map<std::string, std::shared_ptr<AnimationClip>> AnimationMap;
	typedef std::unordered_map<std::string, SharedSkeleton> SkeletonMap;

	enum class ANIMATION_TYPE : uint8_t
	{
		AUTO,
		RAW_CLIP,
		DIFFERENCE_CLIP,
		BAKED_CLIP,
	};

	class AnimationHandler
	{
	public:
		AnimationHandler();
		~AnimationHandler();

		bool LoadAnimation(std::string file, std::string name, ANIMATION_TYPE type, SharedSkeleton skeleton);
		bool LoadSkeleton(std::string file, std::string name);

		SharedAnimationClip GetRawClip(std::string key);
		SharedAnimationClip GetDifferenceClip(std::string key);
		SharedAnimationClip GetBakedClip(std::string key);
		
	private:
		AnimationMap m_RawClips;
		AnimationMap m_DifferenceClips;
		AnimationMap m_BakedClips;
		SkeletonMap  m_Skeletons;
	};


#pragma endregion AnimationClip and Skeleton handler
}
