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
	};
	
	class AnimatedModel : public Model
	{
	public:
		AnimatedModel();
		AnimatedModel(const ComPtr<ID3D11Buffer>& buffer, const ComPtr<ID3D11InputLayout>& layout, uint32_t vertexCount);
	
		///Animation methods
		void SetMainClip(std::shared_ptr<AE::AnimationClip> clip);
		void SetAnimationLayer(std::shared_ptr<AE::AnimationClip> clip, uint8_t layer);
	
		void Update();
	private:
		std::shared_ptr<AE::AnimationClip> m_MainClip;
		std::shared_ptr<AE::AnimationClip> m_AnimationLayer1;
	
		std::vector<DirectX::XMFLOAT4X4A> m_SkinningMatrices;
		std::vector<DirectX::XMFLOAT4X4A> m_ModelMatrices;
	
		
	};

	typedef std::shared_ptr<AE::Model> SharedStaticModel;
	typedef std::shared_ptr<AE::AnimatedModel> SharedAnimatedModel;
	typedef std::unordered_map<std::string, SharedStaticModel> StaticModelMap;
	typedef std::unordered_map<std::string, SharedAnimatedModel> AnimatedModelMap;
}

