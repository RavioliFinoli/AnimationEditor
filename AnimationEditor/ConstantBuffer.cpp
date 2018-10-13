#include "ConstantBuffer.h"


ConstantBuffer::ConstantBuffer(uint8_t slot, size_t size) : m_bufferSlot(slot), m_size(size)
{
	m_data = std::make_unique<BYTE[]>(m_size);
	ZeroMemory(m_data.get(), m_size);
}

void ConstantBuffer::SetData(PVOID64 data)
{
	memcpy(m_data.get(), data, m_size);
}

