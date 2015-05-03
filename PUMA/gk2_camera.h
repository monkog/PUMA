#ifndef __GK2_CAMERA_H_
#define __GK2_CAMERA_H_

#include <d3d11.h>
#include <xnamath.h>

namespace gk2
{
	class Camera
	{
	public:
		Camera();

		void Move(float x, float y, float z);
		void Rotate(float dx, float dy);
		XMMATRIX GetViewMatrix();
		void GetViewMatrix(XMMATRIX& viewMatrix);
		XMFLOAT4 GetPosition();

	private:
		float m_angleX;
		float m_angleY;
		XMVECTOR m_upVector;
		XMVECTOR m_focusPosition; 
		XMVECTOR m_position;
		XMVECTOR m_lastCam;
	};
}

#endif __GK2_CAMERA_H_
