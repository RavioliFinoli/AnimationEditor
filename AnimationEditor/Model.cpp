#include "Model.h"

#define CLIP(x) (x.clip)

namespace AE
{
	Model::Model()
	{
		using namespace DirectX;
	
		m_InputLayout->Release();
		m_VertexBuffer->Release();
	
		m_Translation = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_RotationQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_Scale = { 1.0f, 1.0f, 1.0f, 0.0f };
	
		XMStoreFloat4x4A(&m_WorldMatrix, XMMatrixIdentity());
	}
	
	
	Model::Model(const ComPtr<ID3D11Buffer>& buffer, const ComPtr<ID3D11InputLayout>& layout, uint32_t vertexCount) : m_VertexBuffer(buffer), m_InputLayout(layout), m_VertexCount(vertexCount)
	{
		using namespace DirectX;
		m_Translation = {0.0f, 0.0f, 0.0f, 1.0f};
		m_RotationQuaternion = {0.0f, 0.0f, 0.0f, 1.0f};
		m_Scale = {1.0f, 1.0f, 1.0f, 0.0f};
		
		XMStoreFloat4x4A(&m_WorldMatrix, XMMatrixIdentity());
	}
	
	Model::~Model()
	{
	}
	
	void Model::SetPosition(DirectX::XMFLOAT4A newPosition)
	{
		m_Translation = newPosition;
	}
	
	void Model::SetRotation(DirectX::XMFLOAT4A newRotation)
	{
		m_RotationQuaternion = newRotation;
	}
	
	void Model::SetScale(float newScale)
	{
		m_Scale = { newScale, newScale, newScale, 0.0 };
	}
	
	void Model::SetVertexBuffer(const ComPtr<ID3D11Buffer>& buffer)
	{
		m_VertexBuffer = buffer;
	}

	void Model::SetVertexCount(uint32_t count)
	{
		m_VertexCount = count;
	}
	
	void Model::SetVertexLayout(const ComPtr<ID3D11InputLayout>& layout)
	{
		m_InputLayout = layout;
	}
	
	DirectX::XMFLOAT4X4A Model::GetWorldMatrix()
	{
		using namespace DirectX;
	
		XMVECTOR t = XMLoadFloat4A(&m_Translation);
		XMVECTOR r = XMLoadFloat4A(&m_RotationQuaternion);
		XMVECTOR s = XMLoadFloat4A(&m_Scale);
	
		XMStoreFloat4x4A(&m_WorldMatrix, DirectX::XMMatrixAffineTransformation(s, XMVectorZero(), r, t));
		return m_WorldMatrix;
	}
	
	const ComPtr<ID3D11Buffer>& Model::GetVertexBuffer()
	{
		return m_VertexBuffer;
	}

	uint32_t Model::GetVertexCount()
	{
		return m_VertexCount;
	}

	float Model::GetScale()
	{
		return m_Scale.x;
	}

	void Model::ToggleDrawState()
	{
		m_bDraw = !m_bDraw;
	}

	void Model::SetDrawState(bool state)
	{
		m_bDraw = state;
	}

	bool Model::GetDrawState()
	{
		return m_bDraw;
	}
}
