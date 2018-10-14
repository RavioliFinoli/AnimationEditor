#pragma once
#include <d3d11.h>
#include <windows.h>
#include <wrl.h>
#include <inttypes.h>
#include <memory>
using Microsoft::WRL::ComPtr;

class ConstantBuffer
{
public:
	ConstantBuffer(uint8_t slot, size_t size);
	~ConstantBuffer();

	void SetData(PVOID64 data);
	void BindToVertexShader();
	void BindToPixelShader();
private:
	uint8_t m_bufferSlot = 0;
	size_t m_size = 0;
	ComPtr<ID3D11Buffer> m_buffer = nullptr;
	std::unique_ptr<BYTE[]> m_data;
};

