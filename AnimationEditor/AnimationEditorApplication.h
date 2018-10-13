#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "Animation.h"
#include "Helpers.h"
using Microsoft::WRL::ComPtr;

class AnimationEditorApplication
{
public:
	AnimationEditorApplication();
	~AnimationEditorApplication();

	static ComPtr<ID3D11Device> gDevice;
	static ComPtr<ID3D11DeviceContext> gDeviceContext;

	bool LoadSkeletonFilesInDirectory(std::string dir);
private:
	AE::AnimationHandler m_handler;
};

