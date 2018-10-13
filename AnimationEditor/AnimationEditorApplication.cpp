#include "AnimationEditorApplication.h"

ComPtr<ID3D11Device> AnimationEditorApplication::gDevice = nullptr;
ComPtr<ID3D11DeviceContext> AnimationEditorApplication::gDeviceContext = nullptr;

AnimationEditorApplication::AnimationEditorApplication()
{
}


AnimationEditorApplication::~AnimationEditorApplication()
{
}
