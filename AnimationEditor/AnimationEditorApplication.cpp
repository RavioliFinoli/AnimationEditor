#include "AnimationEditorApplication.h"
#include "../AnimationModule/src/AnimationModule.h"
#include "Helpers.h"

ComPtr<ID3D11Device> AnimationEditorApplication::gDevice = nullptr;
ComPtr<ID3D11DeviceContext> AnimationEditorApplication::gDeviceContext = nullptr;
ComPtr<IDXGISwapChain> AnimationEditorApplication::gSwapChain = nullptr;
ComPtr<ID3D11RenderTargetView> AnimationEditorApplication::gBackbufferRTV = nullptr;


HRESULT CreateDirect3DContext(HWND wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = wndHandle;                           // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multi-samples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		AEApp::gSwapChain.GetAddressOf(),
		AEApp::gDevice.GetAddressOf(),
		NULL,
		AEApp::gDeviceContext.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		AEApp::gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		AEApp::gDevice->CreateRenderTargetView(pBackBuffer, NULL, AEApp::gBackbufferRTV.GetAddressOf());
		pBackBuffer->Release();

		// set the render target as the back buffer
		AEApp::gDeviceContext->OMSetRenderTargets(1, AEApp::gBackbufferRTV.GetAddressOf(), NULL);
	}
	return hr;
}

void SetViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width = (float)640;
	vp.Height = (float)480;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	AEApp::gDeviceContext->RSSetViewports(1, &vp);
}

AnimationEditorApplication::AnimationEditorApplication()
{
}

AnimationEditorApplication::~AnimationEditorApplication()
{
}

HRESULT AnimationEditorApplication::Init(HWND hwnd)
{
	HRESULT hr = CreateDirect3DContext(hwnd);
	if (hr != S_OK)
		return hr;

	SetViewport();
	m_ModelHandler.Init();
	return hr;
}


void AnimationEditorApplication::Update()
{
	//Update stuff
}

void AnimationEditorApplication::Render()
{
	///Render animated models:

	const float clearcolor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	auto staticModels = m_ModelHandler.GetStaticModelMap();
	auto animatedModels = m_ModelHandler.GetAnimatedModelMap();

	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV.Get(), clearcolor);

	gDeviceContext->VSSetShader(m_ModelHandler.GetStaticModelVertexShader().Get(), nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(m_ModelHandler.GetPixelShader().Get(), nullptr, 0);

	gDeviceContext->IASetInputLayout(m_ModelHandler.GetStaticModelInputLayout().Get());
	UINT32 vertexSize = sizeof(float) * 11;
	UINT32 offset = 0;
	for (auto& model : staticModels)
	{
		auto buffer = model.second->GetVertexBuffer();
		auto vertexCount = model.second->GetVertexCount();

		gDeviceContext->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &vertexSize, &offset);
		gDeviceContext->Draw(vertexCount, 0);
	}

	//vertexSize = (sizeof(float) * 15) + (sizeof(unsigned int) * 4);
	//for (auto& model : animatedModels)
	//{
	//	auto buffer = model.second->GetVertexBuffer();
	//	auto vertexCount = model.second->GetVertexCount();

	//	gDeviceContext->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &vertexSize, &offset);
	//	gDeviceContext->Draw(vertexCount, 0);
	//}
}

void AnimationEditorApplication::Present()
{
	gSwapChain->Present(0, 0);
}

bool AnimationEditorApplication::LoadSkeletonFilesInDirectory(std::string dir)
{
	using namespace Animation;

	auto dirsAndFileNames = GetPathsAndNamesToFilesMatching("SKELETON", dir);

	for (const auto& i : dirsAndFileNames)
		m_AnimationHandler.LoadSkeleton(i.first, i.second);

	return false;
}

bool AnimationEditorApplication::LoadAnimationFilesInDirectory(std::string dir)
{
	return false;
}

bool AnimationEditorApplication::LoadStaticMeshFilesInDirectory(std::string dir)
{
	auto dirsAndFileNames = GetPathsAndNamesToFilesMatching("STATIC", dir);
	for (const auto& i : dirsAndFileNames)
		m_ModelHandler.LoadStaticModel(i.first, i.second);
	return false;
}

bool AnimationEditorApplication::LoadAnimatedMeshFilesInDirectory(std::string dir)
{
	auto dirsAndFileNames = GetPathsAndNamesToFilesMatching("ANIMATED", dir);
	return false;
}

