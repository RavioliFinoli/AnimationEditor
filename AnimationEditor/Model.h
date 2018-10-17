#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <memory>
#include "Animation.h"
#include <vector>
using Microsoft::WRL::ComPtr;

 
namespace AE
{
	class AnimatedModel;
	class Model : public std::enable_shared_from_this<Model>
	{
	public:
		Model();
		Model(const ComPtr<ID3D11Buffer>& buffer, const ComPtr<ID3D11InputLayout>& layout, uint32_t vertexCount);
		virtual ~Model();
	
		void SetPosition(DirectX::XMFLOAT4A newPosition);
		void SetRotation(DirectX::XMFLOAT4A newRotation);
		void SetScale(float newScale); //Uniform scale
		void SetVertexBuffer(const ComPtr<ID3D11Buffer>& buffer);
		void SetVertexCount(uint32_t count);
		void SetVertexLayout(const ComPtr<ID3D11InputLayout>& layout);
	
		std::shared_ptr<AE::AnimatedModel> AsAnimatedModel();
	
		DirectX::XMFLOAT4X4A GetWorldMatrix();
		const ComPtr<ID3D11Buffer>& GetVertexBuffer();
		uint32_t GetVertexCount();
		
		DirectX::XMFLOAT4X4A m_WorldMatrix;
		DirectX::XMFLOAT4A m_Translation;
		DirectX::XMFLOAT4A m_RotationQuaternion;
		DirectX::XMFLOAT4A m_Scale;
	
	private:
		ComPtr<ID3D11Buffer> m_VertexBuffer;
		ComPtr<ID3D11InputLayout> m_InputLayout;
		uint32_t m_VertexCount = 0;
		bool m_bDraw = false;
	public:
		void ToggleDrawState();
		void SetDrawState(bool state);
		bool GetDrawState();
	};
	
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
	private:

		AnimationLayer m_MainClipData;

		std::vector<AnimationLayer> m_AnimationLayers;
		std::vector<DirectX::XMFLOAT4X4A> m_SkinningMatrices;
		std::vector<DirectX::XMFLOAT4X4A> m_ModelMatrices;
	
		void _computeSkinningMatrices(Animation::SkeletonPose* firstPose, Animation::SkeletonPose* secondPose, float weight);
		void _computeSkinningMatrices(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2);
		void _computeSkinningMatricesCombined(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2);
		void _computeModelMatrices(Animation::SkeletonPose* firstPose, Animation::SkeletonPose* secondPose, float weight);
		void _computeModelMatrices(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2);
		void _interpolatePose(Animation::SkeletonPose* firstPose, Animation::SkeletonPose* secondPose, float weight);
		Animation::JointPose _interpolateJointPose(Animation::JointPose * firstPose, Animation::JointPose * secondPose, float weight);
		std::pair<uint16_t, float> _computeIndexAndProgression(float deltaTime, float currentTime, uint16_t frameCount);
		std::pair<uint16_t, float> _computeIndexAndProgression(float deltaTime, float* currentTime, uint16_t frameCount);
		void UpdateCombined(float deltaTime);
		void _computeModelMatricesCombined(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2);
		void UpdateAdditive(float deltaTime);
	public:
		float GetProgressNormalized() const;
	};

	typedef std::shared_ptr<AE::Model> SharedStaticModel;
	typedef std::shared_ptr<AE::AnimatedModel> SharedAnimatedModel;
	typedef std::unordered_map<std::string, SharedStaticModel> StaticModelMap;
	typedef std::unordered_map<std::string, SharedAnimatedModel> AnimatedModelMap;
}

