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
	class Model
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
	
		DirectX::XMFLOAT4X4A GetWorldMatrix();
		const ComPtr<ID3D11Buffer>& GetVertexBuffer();
		uint32_t GetVertexCount();
		float GetScale();
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

	typedef std::shared_ptr<AE::Model> SharedStaticModel;
	typedef std::unordered_map<std::string, SharedStaticModel> StaticModelMap;
}

