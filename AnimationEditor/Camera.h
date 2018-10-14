#pragma once
#include <DirectXMath.h>
#include <utility>
class Camera
{
public:
	Camera();
	~Camera();

	DirectX::XMFLOAT4X4A GetViewProjectionMatrix();

	void SetPitchYaw(float pitch, float yaw);
	void IncreasePitchYaw(float pitch, float yaw);
	void SetPitch(float pitch);
	void SetYaw(float yaw);

	std::pair<float, float> GetPitchYaw();
	float GetPitch();
	float GetYaw();

private:
	DirectX::XMFLOAT4X4A m_ViewProjectionMatrix = {};
	DirectX::XMFLOAT4X4A m_ProjectionMatrix     = {};
	DirectX::XMFLOAT4X4A m_ViewMatrix           = {};
	DirectX::XMFLOAT4X4A m_CameraOrientation    = {};
	DirectX::XMFLOAT4A m_CameraPosition         = {};

	float m_Pitch = 0.0f;
	float m_Yaw = 0.0f;

	void _constructCameraOrientation();
	void _constructViewMatrix();
};

