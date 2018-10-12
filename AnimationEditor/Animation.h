#pragma once
#include <memory>
#include <vector>
#include <string>
#include <d3d11.h>
#include <unordered_map>

namespace AE
{
#pragma region AnimationClip
	struct AnimationData;
	struct SkeletonData;
	typedef std::shared_ptr<SkeletonData> SharedSkeleton;
	class DifferenceClip;
	class BakedClip;

	class AnimationClip : std::enable_shared_from_this<AnimationClip>
	{
	public:
		AnimationClip();
		~AnimationClip();

		void SetSpeed(float speed);
		void SetMask(float maskValue, uint32_t jointIndex);
		void SetSkeleton(std::shared_ptr<SkeletonData> skeleton);

		std::shared_ptr<DifferenceClip> AsDifferenceClip();
		std::shared_ptr<BakedClip> AsBakedClip();
	private:
		std::shared_ptr<AnimationData> m_animationData = nullptr;
		std::shared_ptr<SkeletonData> m_skeletonData = nullptr;
		std::vector<float> m_jointMask;
		float m_animationSpeed = 1.0f;
	};

	typedef std::shared_ptr<AnimationClip> SharedAnimationClip;
	typedef AnimationClip RawClip;

	class DifferenceClip : public AnimationClip
	{
	public:
		DifferenceClip();
		~DifferenceClip();
	private:
		float m_blendWeight = 1.0;
		std::shared_ptr<AnimationClip> m_originalSource = nullptr;
		std::shared_ptr<AnimationClip> m_originalReference = nullptr;

	};

	class BakedClip : public AnimationClip
	{
	public:
		BakedClip();
		~BakedClip();

	private:
		float m_bakedWeight = 1.0;
		std::shared_ptr<AnimationClip> m_originalClip = nullptr;
		std::shared_ptr<AnimationClip> m_originalDifferenceClip = nullptr;
	};
#pragma endregion AnimationClip base and derived

#pragma region AnimationHandler

	typedef std::unordered_map<std::string, std::shared_ptr<AnimationClip>> AnimationMap;
	typedef std::unordered_map<std::string, std::shared_ptr<SkeletonData>> SkeletonMap;

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

		bool loadAnimation(std::string file, std::string name, ANIMATION_TYPE type = ANIMATION_TYPE::AUTO);
		bool loadSkeleton(std::string file, std::string name);

		SharedAnimationClip getRawClip(std::string key);
		SharedAnimationClip getDifferenceClip(std::string key);
		SharedAnimationClip getBakedClip(std::string key);
		
	private:
		AnimationMap m_rawClips;
		AnimationMap m_differenceClips;
		AnimationMap m_bakedClips;
		SkeletonMap  m_skeletons;
	};
#pragma endregion AnimationClip and Skeleton handler
}
