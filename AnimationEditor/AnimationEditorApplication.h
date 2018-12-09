#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "Animation.h"
#include "Helpers.h"
#include "GraphicsHandler.h"
#include "Camera.h"
#include "ConstantBuffer.h"

using Microsoft::WRL::ComPtr;
const uint16_t MAX_JOINTS = 128;

extern Camera gCamera;
extern std::vector<std::string> gStaticMeshNames;
extern std::vector<std::string> gAnimatedMeshNames;
extern std::vector<std::string> gAnimationClipNames;
extern std::vector<std::string> gSkeletonNames;
extern std::vector<std::string> gDifferenceClipNames;

struct PerFrameVSData 
{
	DirectX::XMFLOAT4X4A viewProjectionMatrix = {};
};

struct PerFramePSData
{
	DirectX::XMFLOAT4A cameraPosition = {};
};

struct PerStaticObjectData
{
	DirectX::XMFLOAT4X4A worldMatrix = {};
};

struct PerAnimatedObjectData
{
	DirectX::XMFLOAT4X4A worldMatrix = {};
	DirectX::XMFLOAT4X4A skinningMatrices[MAX_JOINTS];
};

class AnimationEditorApplication
{
public:
	AnimationEditorApplication();
	~AnimationEditorApplication();

	HRESULT Init(HWND hwnd);

	void DoGui();
	void Update();
	void Render();
	void Present();

	static ComPtr<ID3D11Device> gDevice;
	static ComPtr<ID3D11DeviceContext> gDeviceContext;
	static ComPtr<IDXGISwapChain> gSwapChain;
	static ComPtr<ID3D11RenderTargetView> gBackbufferRTV;
	static ComPtr<ID3D11DepthStencilView> gDepthStencilView;
	static ComPtr<ID3D11DepthStencilState> gDepthStencilState;
	
	bool LoadAssetsInDirectory(std::string dir);
	AE::SharedSkeleton LoadSkeletonFilesInDirectory(std::string dir);
	bool LoadAnimationFilesInDirectory(std::string dir, AE::SharedSkeleton skeleton);
	bool LoadStaticMeshFilesInDirectory(std::string dir);
	bool LoadAnimatedMeshFilesInDirectory(std::string dir);
	void IdentifyAndLoadFile(std::wstring fullPathWide);
private:
	AE::AnimationHandler m_AnimationHandler;
	AE::GraphicsHandler m_ModelHandler;

	std::unique_ptr<ConstantBuffer> m_PerFrameVSBuffer;
	std::unique_ptr<ConstantBuffer> m_PerStaticObjectBuffer;
	std::unique_ptr<ConstantBuffer> m_PerAnimatedObjectBuffer;
	std::unique_ptr<ConstantBuffer> m_PerFramePSBuffer;

	void RenameAnimatedModel(std::string oldKey, std::string newKey);
	void RenameSkeleton(std::string oldKey, std::string newKey);
	void RenameRawClip(std::string oldKey, std::string newKey);
public:
	AE::SharedAnimatedModel GetAnimatedFromKey(std::string key);
private:
	void SetDepthStencil();
	HRESULT CreateDirect3DContext(HWND wndHandle);
};

typedef AnimationEditorApplication AEApp;
extern AEApp gApp;

