#pragma once
#include "Model.h"

namespace AE
{
	
	class AnimatedModel : public Model
	{
	public:
		AnimatedModel();
		AnimatedModel(const ComPtr<ID3D11Buffer>& buffer, const ComPtr<ID3D11InputLayout>& layout, uint32_t vertexCount, std::shared_ptr<AnimationClip> clip);
		AnimatedModel(const ComPtr<ID3D11Buffer>& buffer, const ComPtr<ID3D11InputLayout>& layout, uint32_t vertexCount);
	
		///Animation methods
		void SetMainClip(std::shared_ptr<AE::AnimationClip> clip);
		void AddAnimationLayer(std::shared_ptr<AE::AnimationClip> clip);
		void PopAnimationLayer(uint8_t layerIndex);
		void SetAnimationLayer(std::shared_ptr<AE::AnimationClip> clip, uint8_t layer);
		std::vector<DirectX::XMFLOAT4X4A>* GetSkinningMatrices();
		const PlaybackData& GetMainClipPlaybackData();
		uint8_t GetLayerCount();
		AnimatedModelInformation GetInformation();
		void Update(float deltaTime);
		void SetLayerWeight(float weight, int layer);
	private:
	
		AnimationLayer m_MainClipData;
	
		std::vector<AnimationLayer> m_AnimationLayers;
		std::vector<DirectX::XMFLOAT4X4A> m_SkinningMatrices;
		std::vector<DirectX::XMFLOAT4X4A> m_ModelMatrices;
	
		Animation::JointPose _getPoseFromLayer(AE::AnimationLayer& layer, uint8_t jointIndex, float deltaTime);
		void _computeSkinningMatrices(Animation::SkeletonPose* firstPose, Animation::SkeletonPose* secondPose, float weight);
		void _computeSkinningMatrices(Animation::SkeletonPose * preInterpolatedPose);
		void _computeSkinningMatrices(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2);
		//void _computeSkinningMatricesCombined(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2);
		void _computeModelMatrices(Animation::SkeletonPose* firstPose, Animation::SkeletonPose* secondPose, float weight);
		void _computeModelMatrices(Animation::SkeletonPose * preInterpolatedPose);
		void _computeModelMatrices(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2);
		void _interpolatePose(Animation::SkeletonPose* firstPose, Animation::SkeletonPose* secondPose, float weight, Animation::SkeletonPose* outPose, uint8_t jointCount);
		Animation::JointPose _interpolateJointPose(Animation::JointPose * firstPose, Animation::JointPose * secondPose, float weight);
		std::pair<uint16_t, float> _computeIndexAndProgression(float deltaTime, float currentTime, uint16_t frameCount);
		std::pair<uint16_t, float> _computeIndexAndProgression(float deltaTime, float* currentTime, uint16_t frameCount);
		//void UpdateCombined(float deltaTime);
		//void _computeModelMatricesCombined(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2);
		void UpdateAdditive(float deltaTime);
		void _weightPose(Animation::JointPose& jointPose, float weight);
	public:
		float GetProgressNormalized() const;
	};

	typedef std::shared_ptr<AE::AnimatedModel> SharedAnimatedModel;
	typedef std::unordered_map<std::string, SharedAnimatedModel> AnimatedModelMap;

}

