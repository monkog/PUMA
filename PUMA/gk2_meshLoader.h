#ifndef __GK2_MESH_LOADER_H_
#define __GK2_MESH_LOADER_H_

#include "gk2_deviceHelper.h"
#include "gk2_vertices.h"
#include "gk2_mesh.h"
#include <string>

namespace gk2
{
	class MeshLoader
	{
	public:

		const gk2::DeviceHelper& getDevice() const { return m_device; }
		void setDevice(const gk2::DeviceHelper& device) { m_device = device; }
		XMFLOAT3 GetCircleVertex(int i);
		XMFLOAT3 GetCirclePartVertex(int i);
		gk2::Mesh GetQuad(float side = 1.0f);
		gk2::Mesh LoadMesh(const std::wstring& fileName);
		gk2::Mesh GetCircle(float radius);

	private:
		XMVECTOR m_center = XMVectorSet(-0.9f - 1.2f / 2.0f, -1.0f + 1.2f / 2.0f* sqrt(3), 0.0f,1.0f);
		gk2::DeviceHelper m_device;
		VertexPosNormal* m_meshVertices;
		VertexPosNormal* m_meshVerticesPart;
	};
}

#endif __GK2_MESH_LOADER_H_
