#ifndef __GK2_MESH_LOADER_H_
#define __GK2_MESH_LOADER_H_

#include "gk2_deviceHelper.h"
#include "gk2_mesh.h"
#include <string>

namespace gk2
{
	class MeshLoader
	{
	public:

		const gk2::DeviceHelper& getDevice() const { return m_device; }
		void setDevice(const gk2::DeviceHelper& device) { m_device = device; }

		gk2::Mesh GetSphere(int stacks, int slices, float radius = 0.5f);
		gk2::Mesh GetCylinder(int stacks, int slices, float radius = 0.5f, float height = 1.0f);
		gk2::Mesh GetDisc(int slices, float radius = 0.5f);
		gk2::Mesh GetBox(float side = 1.0f);
		gk2::Mesh GetQuad(float side = 1.0f);
		gk2::Mesh LoadMesh(const std::wstring& fileName);

	private:
		gk2::DeviceHelper m_device;
	};
}

#endif __GK2_MESH_LOADER_H_
