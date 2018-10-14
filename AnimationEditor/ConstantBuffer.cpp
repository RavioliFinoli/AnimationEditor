#include "ConstantBuffer.h"
#include "AnimationEditorApplication.h"

ConstantBuffer::ConstantBuffer(uint8_t slot, size_t size) : m_bufferSlot(slot), m_size(size)
{
	m_data = std::make_unique<BYTE[]>(m_size);
	ZeroMemory(m_data.get(), m_size);

	_createBuffer();
}

ConstantBuffer::~ConstantBuffer()
{
}

void ConstantBuffer::SetData(PVOID64 data)
{
	memcpy(m_data.get(), data, m_size);
	_update();
}

void ConstantBuffer::BindToVertexShader()
{
	AEApp::gDeviceContext->VSSetConstantBuffers(m_bufferSlot, 1, m_buffer.GetAddressOf());
}

void ConstantBuffer::BindToPixelShader()
{
	AEApp::gDeviceContext->PSSetConstantBuffers(m_bufferSlot, 1, m_buffer.GetAddressOf());
}

void ConstantBuffer::_update()
{
	AEApp::gDeviceContext->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m_Subresource);
	memcpy(m_Subresource.pData, m_data.get(), m_size);
	AEApp::gDeviceContext->Unmap(m_buffer.Get(), 0);
}

void ConstantBuffer::_createBuffer()
{
	D3D11_BUFFER_DESC AnimationBufferDesc;
	AnimationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	AnimationBufferDesc.ByteWidth = m_size;
	AnimationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	AnimationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	AnimationBufferDesc.MiscFlags = 0;
	AnimationBufferDesc.StructureByteStride = 0;

	// check if the creation failed for any reason
	HRESULT hr = 0;
	hr = AEApp::gDevice->CreateBuffer(&AnimationBufferDesc, nullptr, m_buffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		// handle the error, could be fatal or a warning...
		exit(-3);
	}
}

