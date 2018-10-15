#include "AnimationEditorApplication.h"
#include "../AnimationModule/src/AnimationModule.h"
#include "Helpers.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
ComPtr<ID3D11Device> AnimationEditorApplication::gDevice = nullptr;
ComPtr<ID3D11DeviceContext> AnimationEditorApplication::gDeviceContext = nullptr;
ComPtr<IDXGISwapChain> AnimationEditorApplication::gSwapChain = nullptr;
ComPtr<ID3D11RenderTargetView> AnimationEditorApplication::gBackbufferRTV = nullptr;
std::vector<std::string> AnimationEditorApplication::gStaticMeshNames;


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

	///Init cbuffers
	m_PerFrameBuffer          = std::make_unique<ConstantBuffer>(0, sizeof(PerFrameData));
	m_PerStaticObjectBuffer   = std::make_unique<ConstantBuffer>(1, sizeof(PerStaticObjectData));
	m_PerAnimatedObjectBuffer = std::make_unique<ConstantBuffer>(1, sizeof(PerAnimatedObjectData));


	return hr;
}
//#GUI
#pragma region "ImGui"
void AnimationEditorApplication::DoGui()
{
#pragma region "Testing 1"
	ImGui::Begin("poop");
	const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
	static const auto items2 = &AEApp::gStaticMeshNames;
	static const char* item_current = items2->at(0).c_str();            // Here our selection is a single pointer stored outside the object.
	if (ImGui::BeginCombo("combo 1", item_current, 0)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < items2->size(); n++)
		{
			bool is_selected = (item_current == items[n]);
			if (ImGui::Selectable(items2->at(n).c_str(), is_selected))
				item_current = items2->at(n).c_str();
			if (is_selected)
				ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
		}
		ImGui::EndCombo();
	}
	if (ImGui::Button("Toggle render"))
	{
		m_ModelHandler.GetStaticModel(item_current)->ToggleDrawState();
	}
	ImGui::End();
	static bool poo = true;
	ImGui::ShowDemoWindow(&poo);
#pragma endregion "Testing 1"

#pragma region "Testing 2"
	ImGui::Begin("poop 2");

	ImGui::End();
#pragma endregion "Testing 2"
}
#pragma endregion "ImGui"

void AnimationEditorApplication::Update()
{
	//Update gui
	DoGui();
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

	PerFrameData pf = {};
	pf.viewProjectionMatrix = m_Camera.GetViewProjectionMatrix();
	pf.cameraPosition = m_Camera.GetPosition();

	m_PerFrameBuffer->SetData(&pf);
	m_PerFrameBuffer->BindToVertexShader();
	UINT32 vertexSize = sizeof(float) * 11;
	UINT32 offset = 0;
	for (auto& model : staticModels)
	{
		if (model.second->GetDrawState())
		{
			PerStaticObjectData pso = {};
			pso.worldMatrix = model.second->GetWorldMatrix();
			m_PerStaticObjectBuffer->SetData(&pso);
			m_PerStaticObjectBuffer->BindToVertexShader();
			auto buffer = model.second->GetVertexBuffer();
			auto vertexCount = model.second->GetVertexCount();
	
			gDeviceContext->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &vertexSize, &offset);
			gDeviceContext->Draw(vertexCount, 0);
		}
	}

	vertexSize = (sizeof(float) * 15) + (sizeof(unsigned int) * 4);
	for (auto& model : animatedModels)
	{
		if (model.second->GetDrawState())
		{
			auto buffer = model.second->GetVertexBuffer();
			auto vertexCount = model.second->GetVertexCount();

			gDeviceContext->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &vertexSize, &offset);
			gDeviceContext->Draw(vertexCount, 0);
		}
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void AnimationEditorApplication::Present()
{
	gSwapChain->Present(0, 0);
}

bool AnimationEditorApplication::LoadAssetsInDirectory(std::string dir)
{
	auto firstSkeletonFound = LoadSkeletonFilesInDirectory(dir);
	LoadAnimationFilesInDirectory(dir, firstSkeletonFound);
	LoadAnimatedMeshFilesInDirectory(dir);
	LoadStaticMeshFilesInDirectory(dir);
	return true; //#todo
}

AE::SharedSkeleton AnimationEditorApplication::LoadSkeletonFilesInDirectory(std::string dir)
{
	using namespace Animation;

	auto dirsAndFileNames = GetPathsAndNamesToFilesMatching("SKELETON", dir);

	for (const auto& i : dirsAndFileNames)
		m_AnimationHandler.LoadSkeleton(i.first, i.second);

	if (dirsAndFileNames.size())
		return m_AnimationHandler.GetSkeleton(dirsAndFileNames[0].second);
	else
		return AE::SharedSkeleton();
}

bool AnimationEditorApplication::LoadAnimationFilesInDirectory(std::string dir, AE::SharedSkeleton skeleton)
{
	auto dirsAndFileNames = GetPathsAndNamesToFilesMatching("ANIMATION", dir);
	for (const auto& i : dirsAndFileNames)
	{
		m_AnimationHandler.LoadAnimation(i.first, i.second, AE::ANIMATION_TYPE::RAW_CLIP, skeleton);
	}
	return false;
}

bool AnimationEditorApplication::LoadStaticMeshFilesInDirectory(std::string dir)
{
	auto dirsAndFileNames = GetPathsAndNamesToFilesMatching("STATIC", dir);
	for (const auto& i : dirsAndFileNames)
	{
		m_ModelHandler.LoadStaticModel(i.first, i.second);
		gStaticMeshNames.push_back(i.second);
	}
	return false;
}

bool AnimationEditorApplication::LoadAnimatedMeshFilesInDirectory(std::string dir)
{
	auto dirsAndFileNames = GetPathsAndNamesToFilesMatching("ANIMATED", dir);
	for (const auto& i : dirsAndFileNames)
	{
		m_ModelHandler.LoadAnimatedModel(i.first, i.second);
	}
	return false;
}

Camera* AnimationEditorApplication::GetCamera()
{
	return &m_Camera;
}

