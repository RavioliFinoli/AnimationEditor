#pragma once
#include <vector>
#include <exception>
#include <d3d11.h>
std::vector<std::pair<std::string, std::string>> GetPathsAndNamesToFilesMatching(std::string match, std::string dir);
std::string ws2s(const std::wstring& wstr);


inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch DirectX API errors
		throw std::exception();
	}
}


