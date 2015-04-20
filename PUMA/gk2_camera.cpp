#include "gk2_camera.h"

using namespace gk2;

Camera::Camera()
	: m_angleX(0.0f), m_angleY(1.0f), m_distance(5.0f), m_xPos(0.0f), m_yPos(1.0f)
{ }

void Camera::Move(float x, float y)
{
	m_xPos -= x / 10.0f;
	m_yPos += y / 10.0f;
}

void Camera::Rotate(float dx, float dy)
{
	m_angleX = XMScalarModAngle(m_angleX + dx);
	m_angleY = XMScalarModAngle(m_angleY + dy);
}

XMMATRIX Camera::GetViewMatrix()
{
	XMMATRIX viewMtx;
	GetViewMatrix(viewMtx);
	return viewMtx;
}

void Camera::GetViewMatrix(XMMATRIX& viewMtx)
{
	viewMtx = XMMatrixRotationX(m_angleX) * XMMatrixRotationY(-m_angleY) * XMMatrixTranslation(m_xPos, m_yPos, m_distance);
}

XMFLOAT4 Camera::GetPosition()
{
	XMMATRIX viewMtx;
	GetViewMatrix(viewMtx);
	XMVECTOR det;
	viewMtx = XMMatrixInverse(&det, viewMtx);
	XMFLOAT3 res(0.0f, 0.0f, 0.0f);
	XMVECTOR transl = XMVector3TransformCoord(XMLoadFloat3(&res), viewMtx);
	XMStoreFloat3(&res, transl);
	return XMFLOAT4(res.x, res.y, res.z, 1.0f);
}