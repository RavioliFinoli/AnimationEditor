#include "Camera.h"

const DirectX::XMFLOAT4A default_forward = { 0.0f, 0.0, 1.0f, 0.0f };
const DirectX::XMFLOAT4A default_right = { 1.0f, 0.0f, 0.0f, 0.0f };

Camera::Camera()
{
	using namespace DirectX;
	XMStoreFloat4x4A(&m_ProjectionMatrix, XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4 * 1.5f, 1.7777777f, 0.2f, 100.0f));
	m_CameraPosition = { 0.0f, 3.0, -8.0f, 1.0f };
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
	//m_Pitch += pitch;
	//m_Yaw += yaw;
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

DirectX::XMFLOAT4X4A Camera::GetProjectionMatrix()
{
	return m_ProjectionMatrix;
}

void Camera::MoveForward()
{
	using namespace DirectX;
	XMStoreFloat4A(&m_CameraPosition, XMVectorScale(XMVectorAdd(XMLoadFloat4A(&m_CameraPosition), XMVector4Transform(XMLoadFloat4A(&default_forward), XMLoadFloat4x4A(&m_CameraOrientation))), -0.01f));
}

DirectX::XMFLOAT4A Camera::GetPosition()
{
	return m_CameraPosition;
}

void Camera::_constructCameraOrientation()
{
	using namespace DirectX;
	XMStoreFloat4x4A(&m_CameraOrientation, XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f));
}

void Camera::_constructViewMatrix()
{
	m_CameraPosition.w = 1.0f;
	using namespace DirectX;
	_constructCameraOrientation();
	XMVECTOR lookAtPosition;
	lookAtPosition = XMVectorAdd(XMLoadFloat4A(&m_CameraPosition), XMVector4Transform(XMLoadFloat4A(&default_forward), XMLoadFloat4x4A(&m_CameraOrientation)));
	XMMATRIX viewMatrix = XMMatrixLookAtLH(XMLoadFloat4A(&m_CameraPosition), lookAtPosition, {0.0f, 1.0f, 0.0f, 0.0f});
	XMStoreFloat4x4A(&m_ViewMatrix, viewMatrix);
}
