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

		void Move(float x, float y);
		void Rotate(float dx, float dy);
		XMMATRIX GetViewMatrix();
		void GetViewMatrix(XMMATRIX& viewMatrix);
		XMFLOAT4 GetPosition();

	private:
		float m_angleX;
		float m_angleY;
		float m_distance;
		float m_xPos;
		float m_yPos;
	};
}

#endif __GK2_CAMERA_H_
