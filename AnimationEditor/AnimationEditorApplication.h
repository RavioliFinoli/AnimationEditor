#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "Animation.h"
#include "Helpers.h"
#include "GraphicsHandler.h"
#include "Camera.h"

using Microsoft::WRL::ComPtr;

class AnimationEditorApplication
{
public:
	AnimationEditorApplication();
	~AnimationEditorApplication();

	HRESULT Init(HWND hwnd);

	void Update();
	void Render();
	void Present();

	static ComPtr<ID3D11Device> gDevice;
	static ComPtr<ID3D11DeviceContext> gDeviceContext;
	static ComPtr<IDXGISwapChain> gSwapChain;
	static ComPtr<ID3D11RenderTargetView> gBackbufferRTV;
	bool LoadSkeletonFilesInDirectory(std::string dir);
	bool LoadAnimationFilesInDirectory(std::string dir);
	bool LoadStaticMeshFilesInDirectory(std::string dir);
	bool LoadAnimatedMeshFilesInDirectory(std::string dir);
private:
	AE::AnimationHandler m_AnimationHandler;
	AE::GraphicsHandler m_ModelHandler;

	Camera m_Camera;
};

typedef AnimationEditorApplication AEApp;

