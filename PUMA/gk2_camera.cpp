#include "gk2_camera.h"

using namespace gk2;

Camera::Camera()
: m_angleX(0.0f), m_angleY(0.0f), m_upVector(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
{
	m_position = XMVectorSet(-8.0f, 0.0f, 0.0f, 0.0f);
	m_focusPosition = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	m_lastCam = m_focusPosition;
}

void Camera::Move(float x, float y, float z)
{
	x = x > 0 ? 0.01 : x < 0 ? -0.01 : 0;
	y = y > 0 ? 0.01 : y < 0 ? -0.01 : 0;
	z = z > 0 ? 0.01 : z < 0 ? -0.01 : 0;
	XMVECTOR deltaVec;
	if (x != 0)
		deltaVec = XMVector3Normalize(m_focusPosition)*x;
	else if (y != 0)
		deltaVec = XMVector3Normalize(m_upVector)*y;
	else if (z != 0)
		deltaVec = XMVector3Normalize(XMVector3Cross(XMVector3Normalize(m_focusPosition), m_upVector))*z;
	m_angleX = 0;
	m_angleY = 0;
	m_position += deltaVec;
	m_lastCam = m_focusPosition;
}

void Camera::Rotate(float dx, float dy)
{
	dy = dy > 0 ? 1 : dy < 0 ? -1 : 0;
	dx = dx > 0 ? 1 : dx < 0 ? -1 : 0;

	XMVECTOR dot = XMVector3Dot(XMVector3Normalize(m_focusPosition), m_upVector);
	if (dy == -1 && XMVectorGetX(dot) < -0.999)
		return;
	if (dy == 1 && XMVectorGetX(dot) > 0.999)
		return;

	dx /= 300.0f;
	dy /= 300.0f;

	if (m_angleY + dy < -XM_PIDIV2)
		m_angleY = -XM_PIDIV2;
	else if (m_angleY + dy >= XM_PIDIV2)
		m_angleY = XM_PIDIV2;
	else
		m_angleY += dy;

	m_angleX += dx;

	XMMATRIX matrixRot = XMMatrixRotationAxis(XMVector3Cross(m_lastCam, m_upVector), m_angleY) *XMMatrixRotationAxis(m_upVector, m_angleX);
	m_focusPosition = XMVector3TransformCoord(m_lastCam, matrixRot);
}

XMMATRIX Camera::GetViewMatrix()
{
	XMMATRIX viewMtx;
	GetViewMatrix(viewMtx);
	return viewMtx;
}

void Camera::GetViewMatrix(XMMATRIX& viewMtx)
{
	viewMtx = XMMatrixLookToLH(m_position, m_focusPosition, m_upVector);
}

XMFLOAT4 Camera::GetPosition()
{
	XMFLOAT4 pos;
	XMStoreFloat4(&pos, m_position);
	return pos;
}