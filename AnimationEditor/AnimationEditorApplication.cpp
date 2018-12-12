#include "AnimationEditorApplication.h"
#include "../AnimationModule/src/AnimationModule.h"
#include "Model.h"
#include "AnimatedModel.h"
#include "Helpers.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "InputHandler.h"
ComPtr<ID3D11Device> AnimationEditorApplication::gDevice = nullptr;
ComPtr<ID3D11DeviceContext> AnimationEditorApplication::gDeviceContext = nullptr;
ComPtr<IDXGISwapChain> AnimationEditorApplication::gSwapChain = nullptr;
ComPtr<ID3D11RenderTargetView> AnimationEditorApplication::gBackbufferRTV = nullptr;
ComPtr<ID3D11DepthStencilView> AnimationEditorApplication::gDepthStencilView = nullptr;
ComPtr<ID3D11DepthStencilState> AnimationEditorApplication::gDepthStencilState = nullptr;


Camera gCamera;
AEApp gApp;

std::vector<std::string> gStaticMeshNames;
std::vector<std::string> gAnimatedMeshNames;
std::vector<std::string> gAnimationClipNames;
std::vector<std::string> gSkeletonNames;
std::vector<std::string> gDifferenceClipNames;

HRESULT AnimationEditorApplication::CreateDirect3DContext(HWND wndHandle)
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
	scd.SampleDesc.Count = 1;                               // how many multi-samples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		AEApp::gSwapChain.GetAddressOf(),
		AEApp::gDevice.GetAddressOf(),
		NULL,
		AEApp::gDeviceContext.GetAddressOf());

	SetDepthStencil();

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		AEApp::gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		AEApp::gDevice->CreateRenderTargetView(pBackBuffer, NULL, AEApp::gBackbufferRTV.GetAddressOf());
		pBackBuffer->Release();
		AEApp::gDeviceContext->OMSetRenderTargets(1, AEApp::gBackbufferRTV.GetAddressOf(), AEApp::gDepthStencilView.Get());
		// set the render target as the back buffer
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

std::string GetPrefix(std::string input)
{
	auto pos = input.find_first_of("_");
	if (pos != std::string::npos)
	{
		input.erase(input.begin() + pos + 1, input.end());
	}
	return input;
}

HRESULT AnimationEditorApplication::Init(HWND hwnd)
{
	///Create device, context, backbuffer, swap-chain and depth-stencil
	HRESULT hr = CreateDirect3DContext(hwnd);
	if (hr != S_OK)
		return hr;

	///Set viewport
	SetViewport();

	///Init model handler
	m_ModelHandler.Init();

	///Init cbuffers
	m_PerFrameVSBuffer		  = std::make_unique<ConstantBuffer>(0, sizeof(PerFrameVSData));
	m_PerFramePSBuffer		  = std::make_unique<ConstantBuffer>(0, sizeof(PerFrameVSData));
	m_PerStaticObjectBuffer   = std::make_unique<ConstantBuffer>(1, sizeof(PerStaticObjectData));
	m_PerAnimatedObjectBuffer = std::make_unique<ConstantBuffer>(1, sizeof(PerAnimatedObjectData));

	return hr;
}
//#GUI
#pragma region "ImGui"
void AnimationEditorApplication::DoGui()
{
	using namespace DirectX;
#pragma region "Testing 1"
	static bool wantsNewDifferenceClip = false;
	static bool wantsToDuplicate = false;
	static std::string item_current_static = "None selected";
	static std::string item_current_animated = "None selected";
	static std::string item_current_skeleton = "None selected";
	static std::string item_current_animation = "None selected";
	static std::string item_current_diffAnimation = "None selected";
		ImGui::Begin("Assets");

		if (ImGui::BeginCombo("Animated Meshes", item_current_animated.c_str(), 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < gAnimatedMeshNames.size(); n++)
			{
				bool is_selected = (item_current_animated == gAnimatedMeshNames[n]);
				if (ImGui::Selectable(gAnimatedMeshNames.at(n).c_str(), is_selected))
				{
					if (item_current_animated != "None selected")
						m_ModelHandler.GetAnimatedModel(item_current_animated)->SetDrawState(false);
					item_current_animated = gAnimatedMeshNames.at(n).c_str();
					m_ModelHandler.GetAnimatedModel(item_current_animated)->SetDrawState(true);

				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}
		ImGui::Separator();
		if (ImGui::BeginCombo("Skeletons", item_current_skeleton.c_str(), 0)) // The second parameter is the label previewed before opening the combo.
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

		if (ImGui::Button("Set Skeleton"))
		{
			if (item_current_skeleton != "None selected" && item_current_animation != "None selected")
			{
				auto skeleton = m_AnimationHandler.GetSkeleton(item_current_skeleton);
				m_AnimationHandler.GetRawClip(item_current_animation)->SetSkeleton(skeleton);
			}
		}
		ImGui::Separator();

		if (ImGui::BeginCombo("Animations", item_current_animation.c_str(), 0)) // The second parameter is the label previewed before opening the combo.
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
		if (ImGui::Button("Set Animation"))
		{
			if (item_current_animation != "None selected" && item_current_animated != "None selected")
			{
				auto animation = m_AnimationHandler.GetRawClip(item_current_animation);
				m_ModelHandler.GetAnimatedModel(item_current_animated)->SetMainClip(animation);
			}
		}
		ImGui::Separator();
		if (ImGui::Button("Toggle render"))
		{
			if (item_current_static != "None selected")
				m_ModelHandler.GetStaticModel(item_current_static)->ToggleDrawState();
		}




		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();
		{
			static char text_input[64] = ""; ImGui::InputText("New Name", text_input, 64);
			static bool keepPrefix = true;
			if (ImGui::Button("Rename Model"))
			{
				std::string newName = "";
				if (keepPrefix)
					newName.append(GetPrefix(item_current_animated));
				newName.append(text_input);
				RenameAnimatedModel(item_current_animated, newName);
				item_current_animated = newName.c_str();
			}

			if (ImGui::Button("Rename Skeleton"))
			{

			}

			if (ImGui::Button("Rename Animation"))
			{

			}

			ImGui::Checkbox("Keep Prefix", &keepPrefix);
		}

		ImGui::Separator();
		ImGui::Separator();

		ImGui::BulletText("Difference Clips");

		if (ImGui::BeginCombo("Difference Clips", item_current_diffAnimation.c_str(), 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < gDifferenceClipNames.size(); n++)
			{
				bool is_selected = (item_current_diffAnimation == gDifferenceClipNames[n]);
				if (ImGui::Selectable(gDifferenceClipNames.at(n).c_str(), is_selected))
				{
					item_current_diffAnimation = gDifferenceClipNames.at(n);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Add Clip as Layer"))
		{
			if (item_current_animated != "None selected" && item_current_diffAnimation != "None selected")
			{
				m_ModelHandler.GetAnimatedModel(item_current_animated)->AddAnimationLayer(m_AnimationHandler.GetDifferenceClip(item_current_diffAnimation));
			}
		}
		ImGui::Separator();
		if (ImGui::Button("Open Difference Clip maker"))
		{
			wantsNewDifferenceClip = true;
		}

		ImGui::Separator();
		ImGui::Separator();


		if (ImGui::Button("Duplicate Model"))
		{
			wantsToDuplicate = true;
		}

		ImGui::End();
		static bool poo = true;
		ImGui::ShowDemoWindow(&poo);
	
#pragma endregion "Testing 1"

#pragma region "Playback"

	///Display animation playback
	ImGui::Begin("Playback");
	float progress = 0.0f;
	if (item_current_animated != "None selected")
		progress = m_ModelHandler.GetAnimatedModel(item_current_animated)->GetProgressNormalized();
	ImGui::ProgressBar(progress);
	ImGui::End();

#pragma endregion "Playback"

#pragma region "Mesh Information"

	if (item_current_animated != "None selected") 
	{
		ImGui::Begin("Animated Mesh Information");
		std::string skeletonName = "N/A";
		std::string mainClipName = "N/A";

		///Get current model info
		auto info = m_ModelHandler.GetAnimatedModel(item_current_animated)->GetInformation();
		static float scale = info.scale;
		XMFLOAT4 pyr = info.pitchYawRoll;
		pyr.x = DirectX::XMConvertToDegrees(pyr.x);
		pyr.y = DirectX::XMConvertToDegrees(pyr.y);
		pyr.z = DirectX::XMConvertToDegrees(pyr.z);

		///Update scale
		if (ImGui::SliderFloat("Scale", &scale, 0.001f, 10.0f))
		{
			m_ModelHandler.GetAnimatedModel(item_current_animated)->SetScale(scale);
		}
		///Update pitch
		if (ImGui::SliderFloat("Pitch", &pyr.x, -180.0f, 180.0f))
		{
			m_ModelHandler.GetAnimatedModel(item_current_animated)->SetRotation(
				XMConvertToRadians(pyr.x),
				XMConvertToRadians(pyr.y),
				XMConvertToRadians(pyr.z));
		}
		///Update yaw
		if (ImGui::SliderFloat("Yaw", &pyr.y, -180.0f, 180.0f))
		{
			m_ModelHandler.GetAnimatedModel(item_current_animated)->SetRotation(
				XMConvertToRadians(pyr.x),
				XMConvertToRadians(pyr.y),
				XMConvertToRadians(pyr.z));
		}
		///Update roll
		if (ImGui::SliderFloat("Roll", &pyr.z, -180.0f, 180.0f))
		{
			m_ModelHandler.GetAnimatedModel(item_current_animated)->SetRotation(
				XMConvertToRadians(pyr.x),
				XMConvertToRadians(pyr.y),
				XMConvertToRadians(pyr.z));
		}

		///Display information about current animated model
		ImGui::BulletText("Main Animation: %s", info.mainAnimationName.c_str());
		ImGui::BulletText("Skeleton: %s", info.skeletonName.c_str());
		ImGui::BulletText("Frame Count: %d", info.frameCount);
		ImGui::Separator();
		ImGui::BulletText("Current framerate: %f", ImGui::GetIO().Framerate);
		ImGui::End();
	}

#pragma endregion "Mesh Information"

#pragma region "Difference Clip Maker"

	static const char* item_current_animationRef = "None selected";
	static const char* item_current_animationSource = "None selected";
	if (wantsNewDifferenceClip)
	{
		ImGui::Begin("Difference Clip Maker");

		///Source clip for additive clip creation
		if (ImGui::BeginCombo("Reference", item_current_animationRef, 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < gAnimationClipNames.size(); n++)
			{
				bool is_selected = (item_current_animationRef == gAnimationClipNames[n]);
				if (ImGui::Selectable(gAnimationClipNames.at(n).c_str(), is_selected))
					item_current_animationRef = gAnimationClipNames.at(n).c_str();
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		///Source clip for additive clip creation
		if (ImGui::BeginCombo("Source", item_current_animationSource, 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < gAnimationClipNames.size(); n++)
			{
				bool is_selected = (item_current_animationSource == gAnimationClipNames[n]);
				if (ImGui::Selectable(gAnimationClipNames.at(n).c_str(), is_selected))
					item_current_animationSource = gAnimationClipNames.at(n).c_str();
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();
		if (ImGui::Button("Create"))
		{
			if (item_current_skeleton != "None selected")
			{
				AE::SharedDifferenceClip clip = std::make_shared<AE::DifferenceClip>();
				auto source = m_AnimationHandler.GetRawClip(item_current_animationSource);
				auto reference = m_AnimationHandler.GetRawClip(item_current_animationRef);

				reference
					///Create additive clip from reference and source
					? clip->SetAnimationData(MakeNewDifferenceClip(source, reference))
					///Create additive clip from bind-pose and source
					: clip->SetAnimationData(MakeNewDifferenceClip(source, m_AnimationHandler.GetSkeleton(item_current_skeleton)));

				clip->SetName(source->GetName() + "_DIFF");
				m_AnimationHandler.AddDifferenceClip(clip->GetName(), clip);
			}
		}
		ImGui::NewLine();

		if (ImGui::Button("Close"))
		{
			wantsNewDifferenceClip = false;
			item_current_animationRef = "None selected";
			item_current_animationSource = "None selected";
		}

		ImGui::End();
	}

	if (wantsToDuplicate)
	{
		static char text_input[64] = "";

		ImGui::Begin("Duplicate selected");
		ImGui::InputText("Name", text_input, 64);
		ImGui::NewLine();

		if (ImGui::Button("Duplicate"))
		{
			m_ModelHandler.DuplicateModel(item_current_animated, text_input);

			wantsToDuplicate = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
		{
			wantsToDuplicate = false;
		}
		ImGui::End();
	}

#pragma endregion "Difference Clip Maker"

	static float weights[5] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
	if (item_current_animated != "None selected")
	{
		ImGui::Begin("Layers");

		for (int layer = 0; layer < m_ModelHandler.GetAnimatedModel(item_current_animated)->GetLayerCount(); layer++)
		{
			std::string layerString = "Layer " + std::to_string(layer);
			ImGui::BulletText(layerString.c_str());
			if (ImGui::SliderFloat("Weight", &(weights[layer]), 0.0f, 1.0f))
			{
				m_ModelHandler.GetAnimatedModel(item_current_animated)->SetLayerWeight(weights[layer], layer);
			}
			if (ImGui::Button("Remove Layer"))
			{
				m_ModelHandler.GetAnimatedModel(item_current_animated)->PopAnimationLayer(layer);
			}
			if (ImGui::Button("Export.."))
			{
				m_ModelHandler.GetAnimatedModel(item_current_animated)->BakeNewClip(layer, false);
			}
		}

		ImGui::End();
	}
}

#pragma endregion "ImGui"

void AnimationEditorApplication::Update()
{
	///Update GUI
	DoGui();

	///Update animated models
	float deltaTime = ImGui::GetIO().DeltaTime;
	for (auto& model : m_ModelHandler.GetAnimatedModelMap())
	{
		model.second->Update(deltaTime);
	}

	if (gInputHandler.GetWState())
	{
		gCamera.MoveForward(); //#todo very broken
	}
}

void AnimationEditorApplication::Render()
{
	///Set and clear old textures
	{
		gDeviceContext->OMSetRenderTargets(1, AEApp::gBackbufferRTV.GetAddressOf(), gDepthStencilView.Get());
		const float clearcolor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		gDeviceContext->ClearRenderTargetView(gBackbufferRTV.Get(), clearcolor);
		gDeviceContext->ClearDepthStencilView(gDepthStencilView.Get(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

	}

	///Set shaders and configure input assembler for static models
	{
		gDeviceContext->VSSetShader(m_ModelHandler.GetStaticModelVertexShader().Get(), nullptr, 0);
		gDeviceContext->HSSetShader(nullptr, nullptr, 0);
		gDeviceContext->DSSetShader(nullptr, nullptr, 0);
		gDeviceContext->GSSetShader(nullptr, nullptr, 0);
		gDeviceContext->PSSetShader(m_ModelHandler.GetPixelShader().Get(), nullptr, 0);
		gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gDeviceContext->IASetInputLayout(m_ModelHandler.GetStaticModelInputLayout().Get());
	}

	///Set per-frame buffers
	{
		PerFrameVSData pfVS = {};
		pfVS.viewProjectionMatrix = gCamera.GetViewProjectionMatrix();
		PerFramePSData pfPS = {};
		pfPS.cameraPosition = gCamera.GetPosition();

		m_PerFrameVSBuffer->SetData(&pfVS);
		m_PerFramePSBuffer->SetData(&pfPS);
		m_PerFrameVSBuffer->BindToVertexShader();
		m_PerFramePSBuffer->BindToPixelShader();
	}

	///Draw static models
	{
		auto staticModels = m_ModelHandler.GetStaticModelMap();

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
	}

	///Set shaders and configure Input Assembler for animated models
	{
		gDeviceContext->IASetInputLayout(m_ModelHandler.GetAnimatedModelInputLayout().Get());
		gDeviceContext->VSSetShader(m_ModelHandler.GetAnimatedModelVertexShader().Get(), nullptr, 0);
	}

	///Draw animated models
	{
		auto animatedModels = m_ModelHandler.GetAnimatedModelMap();

		UINT32 vertexSize = (sizeof(float) * 15) + (sizeof(unsigned int) * 4);
		UINT32 offset = 0;

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
	}

	///Render ImGui
	{
		AEApp::gDeviceContext->OMSetRenderTargets(1, AEApp::gBackbufferRTV.GetAddressOf(), nullptr);
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
}

void AnimationEditorApplication::Present()
{
	gSwapChain->Present(0, 0);
}

bool AnimationEditorApplication::LoadAssetsInDirectory(std::string dir)
{
	//Check for skeletons first, since the animation data format 
	//currently does not contain information about joint count..
	auto firstSkeletonFound = LoadSkeletonFilesInDirectory(dir);

	LoadAnimationFilesInDirectory(dir, firstSkeletonFound);
	LoadAnimatedMeshFilesInDirectory(dir);
	LoadStaticMeshFilesInDirectory(dir);
	return true;
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

void AnimationEditorApplication::IdentifyAndLoadFile(std::wstring fullPathWide)
{
	std::string fullPath = ws2s(fullPathWide);
	std::string filename = "";
	{
		filename = fullPath;
		auto pos = filename.find_last_of("\\");
		filename.erase(filename.begin(), filename.begin() + pos + 1);
	}
	size_t pos = 0;
	if (std::string::npos != (pos = filename.find("ANIMATION")))
	{
		if (!m_AnimationHandler.GetSkeletonMap().empty())
			m_AnimationHandler.LoadAnimation(fullPath, filename, AE::ANIMATION_TYPE::RAW_CLIP, (*m_AnimationHandler.GetSkeletonMap().begin()).second);
	}
	else if (std::string::npos != (pos = filename.find("SKELETON")))
	{
		m_AnimationHandler.LoadSkeleton(fullPath, filename);
	}
	else if (std::string::npos != (pos = filename.find("ANIMATED")))
	{
		m_ModelHandler.LoadAnimatedModel(fullPath, filename);
	}
	else return; //could not identify file
}

void AnimationEditorApplication::RenameAnimatedModel(std::string oldKey, std::string newKey)
{
	auto iter = m_ModelHandler.GetAnimatedModelMap().extract(oldKey);
	iter.key() = newKey;
	m_ModelHandler.GetAnimatedModelMap().insert(std::move(iter));

	auto vIter = std::find(gAnimatedMeshNames.begin(), gAnimatedMeshNames.end(), oldKey);
	vIter = gAnimatedMeshNames.erase(vIter);
	gAnimatedMeshNames.insert(vIter, newKey);
}

void AnimationEditorApplication::RenameSkeleton(std::string oldKey, std::string newKey)
{
	auto iter = m_AnimationHandler.GetSkeletonMap().extract(oldKey);
	iter.key() = newKey;
	m_AnimationHandler.GetSkeletonMap().insert(std::move(iter));

	auto vIter = std::find(gSkeletonNames.begin(), gSkeletonNames.end(), oldKey);
	gSkeletonNames.emplace(vIter, newKey);
}

void AnimationEditorApplication::RenameRawClip(std::string oldKey, std::string newKey)
{
	auto iter = m_AnimationHandler.GetRawClipMap().extract(oldKey);
	iter.key() = newKey;
	m_AnimationHandler.GetRawClipMap().insert(std::move(iter));

	auto vIter = std::find(gAnimationClipNames.begin(), gAnimationClipNames.end(), oldKey);
	gAnimationClipNames.emplace(vIter, newKey);
}

AE::SharedAnimatedModel AnimationEditorApplication::GetAnimatedFromKey(std::string key)
{
	return m_ModelHandler.GetAnimatedModel(key);
}

void AnimationEditorApplication::SetDepthStencil()
{
	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = 1280;
	depthStencilDesc.Height = 720;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));

	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	ThrowIfFailed(gDevice->CreateDepthStencilState(&dsDesc, gDepthStencilState.ReleaseAndGetAddressOf()));

	ID3D11Texture2D* tex = nullptr;

	ThrowIfFailed(gDevice->CreateTexture2D(&depthStencilDesc, NULL, &tex));
	ThrowIfFailed(gDevice->CreateDepthStencilView(tex, &descDSV, gDepthStencilView.ReleaseAndGetAddressOf()));


}

