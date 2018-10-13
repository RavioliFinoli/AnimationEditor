#include "AnimationEditorApplication.h"
#include "../AnimationModule/src/AnimationModule.h"
#include "Helpers.h"

ComPtr<ID3D11Device> AnimationEditorApplication::gDevice = nullptr;
ComPtr<ID3D11DeviceContext> AnimationEditorApplication::gDeviceContext = nullptr;

AnimationEditorApplication::AnimationEditorApplication()
{
}


AnimationEditorApplication::~AnimationEditorApplication()
{
}

bool AnimationEditorApplication::LoadSkeletonFilesInDirectory(std::string dir)
{
	using namespace Animation;

	auto dirsAndFileNames = GetPathsAndNamesToFilesMatching("SKELETON", dir);
	for (const auto& i : dirsAndFileNames)
	{
		auto skeleton = LoadAndCreateSkeleton(i.first);
		m_handler.loadSkeleton(i.first, i.second);
	}
	return false;
}

bool AnimationEditorApplication::LoadAnimationFilesInDirectory(std::string dir)
{
	return false;
}
