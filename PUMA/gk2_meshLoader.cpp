#include "gk2_meshLoader.h"
#include <vector>
#include "gk2_vertices.h"
#include <xnamath.h>
#include <fstream>

using namespace std;
using namespace gk2;

Mesh MeshLoader::GetQuad(float side /* = 1.0f */)
{
	side /= 2;
	VertexPosNormal vertices[] =
	{
		{ XMFLOAT3(-side, -side, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-side, side, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(side, side, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(side, -side, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
	};
	unsigned short indices[] = { 0, 1, 2, 0, 2, 3, 1, 0, 2, 2, 0, 3 };
	return Mesh(m_device.CreateVertexBuffer(vertices, 4), sizeof(VertexPosNormal),
		m_device.CreateIndexBuffer(indices, 12), 12);
}

Mesh MeshLoader::LoadMesh(const wstring& fileName)
{
	ifstream input;
	input.exceptions(ios::badbit | ios::failbit | ios::eofbit);

	int n, in, m, index;
	input.open(fileName);
	input >> n;
	vector<VertexPosNormal> vert(n);
	for (int i = 0; i < n; ++i)
		input >> vert[i].Pos.x >> vert[i].Pos.y >> vert[i].Pos.z;

	input >> in;
	vector<VertexPosNormal> vertices(in);
	for (int i = 0; i < in; ++i)
	{
		input >> index;
		vertices[i].Pos = vert[index].Pos;
		input >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	input >> m;
	vector<unsigned short> indices(m);
	for (int i = 0; i < m; ++i)
		input >> indices[i];

	input.close();
	return Mesh(m_device.CreateVertexBuffer(vertices), sizeof(VertexPosNormal),
		m_device.CreateIndexBuffer(indices), in);
}
