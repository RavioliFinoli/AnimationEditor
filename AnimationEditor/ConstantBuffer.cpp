#include "ConstantBuffer.h"
#include "AnimationEditorApplication.h"

ConstantBuffer::ConstantBuffer(uint8_t slot, size_t size) : m_bufferSlot(slot), m_size(size)
{
	m_data = std::make_unique<BYTE[]>(m_size);
	ZeroMemory(m_data.get(), m_size);
}

ConstantBuffer::~ConstantBuffer()
{
}

void ConstantBuffer::SetData(PVOID64 data)
{
	memcpy(m_data.get(), data, m_size);
}

void ConstantBuffer::BindToVertexShader()
{
	AEApp::gDeviceContext->VSSetConstantBuffers(m_bufferSlot, 1, m_buffer.GetAddressOf());
}

void ConstantBuffer::BindToPixelShader()
{
	AEApp::gDeviceContext->PSSetConstantBuffers(m_bufferSlot, 1, m_buffer.GetAddressOf());
}

