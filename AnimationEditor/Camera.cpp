#include "Camera.h"

const DirectX::XMFLOAT4A default_forward = { 0.0f, 1.0f, 0.0f, 0.0f };
const DirectX::XMFLOAT4A default_right = { 1.0f, 0.0f, 0.0f, 0.0f };

Camera::Camera()
{
	using namespace DirectX;
	XMStoreFloat4x4A(&m_ViewProjectionMatrix, XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.77777777778f, 0.1f, 100.0f));
}


Camera::~Camera()
{
}

DirectX::XMFLOAT4X4A Camera::GetViewProjectionMatrix()
{
	using namespace DirectX;

	_constructViewMatrix();

	XMMATRIX view = XMLoadFloat4x4A(&m_ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4A(&m_ProjectionMatrix);

	XMStoreFloat4x4A(&m_ViewProjectionMatrix, XMMatrixMultiply(view, proj));

	return m_ViewProjectionMatrix;
}

void Camera::SetPitchYaw(float pitch, float yaw)
{
	m_Pitch = pitch;
	m_Yaw = yaw;
}

void Camera::IncreasePitchYaw(float pitch, float yaw)
{
	m_Pitch += pitch;
	m_Yaw += yaw;
}

void Camera::SetPitch(float pitch)
{
	m_Pitch = pitch;
}

void Camera::SetYaw(float yaw)
{
	m_Yaw = yaw;
}

std::pair<float, float> Camera::GetPitchYaw()
{
	return std::make_pair(m_Pitch, m_Yaw);
}

float Camera::GetPitch()
{
	return m_Pitch;
}

float Camera::GetYaw()
{
	return m_Yaw;
}

void Camera::_constructCameraOrientation()
{
	using namespace DirectX;
	XMStoreFloat4x4A(&m_CameraOrientation, XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f));
}

void Camera::_constructViewMatrix()
{
	using namespace DirectX;
	_constructCameraOrientation();
	XMVECTOR lookAtPosition;
	lookAtPosition = XMVectorAdd(XMLoadFloat4A(&m_CameraPosition), XMVector4Transform(XMLoadFloat4A(&default_forward), XMMatrixIdentity()));
	XMMATRIX viewMatrix = XMMatrixLookAtLH(XMLoadFloat4A(&m_CameraPosition), lookAtPosition, {0.0f, 1.0f, 0.0f, 0.0f});
	XMStoreFloat4x4A(&m_ViewMatrix, viewMatrix);
}
