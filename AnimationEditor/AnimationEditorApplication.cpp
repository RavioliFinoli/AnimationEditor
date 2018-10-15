#include "AnimationEditorApplication.h"
#include "../AnimationModule/src/AnimationModule.h"
#include "Helpers.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "InputHandler.h"
ComPtr<ID3D11Device> AnimationEditorApplication::gDevice = nullptr;
ComPtr<ID3D11DeviceContext> AnimationEditorApplication::gDeviceContext = nullptr;
ComPtr<IDXGISwapChain> AnimationEditorApplication::gSwapChain = nullptr;
ComPtr<ID3D11RenderTargetView> AnimationEditorApplication::gBackbufferRTV = nullptr;


Camera gCamera;

std::vector<std::string> gStaticMeshNames;
std::vector<std::string> gAnimatedMeshNames;
std::vector<std::string> gAnimationClipNames;
std::vector<std::string> gSkeletonNames;

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
	vp.Width = (float)1280;
	vp.Height = (float)720;
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
	std::vector<std::string>().swap(gStaticMeshNames);
	std::vector<std::string>().swap(gAnimatedMeshNames);
	ComPtr<ID3D11Device>().Swap(gDevice);
	ComPtr<ID3D11DeviceContext>().Swap(gDeviceContext);
	ComPtr<IDXGISwapChain>().Swap(gSwapChain);
	ComPtr<ID3D11RenderTargetView>().Swap(gBackbufferRTV);
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
	static const char* item_current_static = "None selected";
	static const char* item_current_animated = "None selected";
	static const char* item_current_skeleton = "None selected";
	static const char* item_current_animation = "None selected";
		ImGui::Begin("poop");
		if (ImGui::BeginCombo("Static Meshes", item_current_static, 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < gStaticMeshNames.size(); n++)
			{
				bool is_selected = (item_current_static == gAnimatedMeshNames[n]);
				if (ImGui::Selectable(gStaticMeshNames.at(n).c_str(), is_selected))
					item_current_static = gStaticMeshNames.at(n).c_str();
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Animated Meshes", item_current_animated, 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < gAnimatedMeshNames.size(); n++)
			{
				bool is_selected = (item_current_animated == gAnimatedMeshNames[n]);
				if (ImGui::Selectable(gAnimatedMeshNames.at(n).c_str(), is_selected))
					item_current_animated = gAnimatedMeshNames.at(n).c_str();
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Skeletons", item_current_skeleton, 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < gSkeletonNames.size(); n++)
			{
				bool is_selected = (item_current_skeleton == gSkeletonNames[n]);
				if (ImGui::Selectable(gSkeletonNames.at(n).c_str(), is_selected))
					item_current_skeleton = gSkeletonNames.at(n).c_str();
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Animations", item_current_animation, 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < gAnimationClipNames.size(); n++)
			{
				bool is_selected = (item_current_animation == gAnimationClipNames[n]);
				if (ImGui::Selectable(gAnimationClipNames.at(n).c_str(), is_selected))
					item_current_animation = gAnimationClipNames.at(n).c_str();
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Toggle render"))
		{
			if (item_current_static != "None selected")
				m_ModelHandler.GetStaticModel(item_current_static)->ToggleDrawState();
		}
		if (ImGui::Button("Set Skeleton"))
		{
			auto skeleton = m_AnimationHandler.GetSkeleton(item_current_skeleton);
			m_AnimationHandler.GetRawClip(item_current_animation)->SetSkeleton(skeleton);
		}
		if (ImGui::Button("Set Animation"))
		{
			auto animation = m_AnimationHandler.GetRawClip(item_current_animation);
			m_ModelHandler.GetAnimatedModel(item_current_animated)->SetMainClip(animation);
		}
		ImGui::End();
		static bool poo = true;
		ImGui::ShowDemoWindow(&poo);
	
#pragma endregion "Testing 1"

#pragma region "Testing 2"
	ImGui::Begin("poop 2");
	float progress = 0.0f;
	if (item_current_animated != "None selected")
		progress = m_ModelHandler.GetAnimatedModel(item_current_animated)->GetProgressNormalized();
	ImGui::ProgressBar(progress);
	ImGui::End();
#pragma endregion "Testing 2"
}
#pragma endregion "ImGui"

void AnimationEditorApplication::Update()
{
	//Update gui
	DoGui();
	float deltaTime = ImGui::GetIO().DeltaTime;
	//Update animated models
	for (auto& model : m_ModelHandler.GetAnimatedModelMap())
	{
		model.second->Update(deltaTime);
	}

	if (gInputHandler.GetWState())
	{
		gCamera.MoveForward(); //#todo broken af
	}
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
	pf.viewProjectionMatrix = gCamera.GetViewProjectionMatrix();
	pf.cameraPosition = gCamera.GetPosition();

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
	gDeviceContext->IASetInputLayout(m_ModelHandler.GetAnimatedModelInputLayout().Get());
	gDeviceContext->VSSetShader(m_ModelHandler.GetAnimatedModelVertexShader().Get(), nullptr, 0);
	vertexSize = (sizeof(float) * 15) + (sizeof(unsigned int) * 4);
	for (auto& model : animatedModels)
	{
		if (model.second->GetDrawState())
		{
			PerAnimatedObjectData pao = {};
			pao.worldMatrix = model.second->GetWorldMatrix();
			auto skinningMatrices = model.second->GetSkinningMatrices();
			memcpy(&pao.skinningMatrices, skinningMatrices->data(), sizeof(DirectX::XMFLOAT4X4A) * skinningMatrices->size());
			m_PerAnimatedObjectBuffer->SetData(&pao);
			m_PerAnimatedObjectBuffer->BindToVertexShader();

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
		gAnimatedMeshNames.push_back(i.second);
	}
	return false;
}


