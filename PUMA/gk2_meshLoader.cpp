#include "gk2_meshLoader.h"
#include <vector>
#include "gk2_vertices.h"
#include <xnamath.h>
#include <fstream>

using namespace std;
using namespace gk2;

Mesh MeshLoader::GetQuad(float side)
{
	side /= 2;
	VertexPosNormal vertices[] =
	{
		{ XMFLOAT3(-side, -side, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-side, side, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(side, side, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(side, -side, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	};
	unsigned short indices[] = { 0, 1, 2, 0, 2, 3, 1, 0, 2, 2, 0, 3 };
	return Mesh(m_device.CreateVertexBuffer(vertices, 4), sizeof(VertexPosNormal),
		m_device.CreateIndexBuffer(indices, 12), 12);
}

Mesh MeshLoader::GetCircle(float radius)
{
	XMVECTOR pos, pos2;
	m_meshVerticesPart = new VertexPosNormal[360];
	m_meshVertices = new VertexPosNormal[360];
	for (int i = 0; i < 360; i++)
	{
		pos = XMLoadFloat3(&XMFLOAT3(radius * cos(i *XM_2PI / 360.0f), radius * sin(i *XM_2PI / 360.0f), 0));
		pos2 = XMVector3Transform(pos, XMMatrixTranslation(0.0f, -0.1f, 1.78f)*XMMatrixRotationX(XM_PIDIV2 / 3.0f));
		pos = XMVector3Transform(pos, XMMatrixRotationY(XM_PIDIV2)* XMMatrixRotationZ(XM_PIDIV2 / 3.0f)*XMMatrixTranslationFromVector(m_center));
		m_meshVertices[i].Pos = XMFLOAT3(XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));
		m_meshVerticesPart[i].Pos = XMFLOAT3(XMVectorGetX(pos2), XMVectorGetY(pos2), XMVectorGetZ(pos2));
		m_meshVerticesPart[i].Normal = m_meshVertices[i].Normal = XMFLOAT3(sqrt(3) / 2.0f, 0.5f, 0.0f);
	}

	unsigned short* indices = new unsigned short[720];
	int counter = 0;
	for (int i = 0; i < 718; i += 2)
	{
		indices[i] = counter++;
		indices[i + 1] = counter;
	}
	indices[718] = counter;
	indices[719] = 0;
	return Mesh(m_device.CreateVertexBuffer(m_meshVerticesPart, 360), sizeof(VertexPosNormal),
		m_device.CreateIndexBuffer(indices, 720), 720);
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
	vector<unsigned short> indices(3 * m);
	for (int i = 0; i < m; ++i)
		input >> indices[3 * i] >> indices[3 * i + 1] >> indices[3 * i + 2];

	input.close();
	return Mesh(m_device.CreateVertexBuffer(vertices), sizeof(VertexPosNormal),
		m_device.CreateIndexBuffer(indices), 3 * m);
}

XMFLOAT3 MeshLoader::GetCircleVertex(int i)
{
	return m_meshVertices[i].Pos;
}

XMFLOAT3 MeshLoader::GetCirclePartVertex(int i)
{
	return m_meshVerticesPart[i<=180? 180-i : 540-i].Pos;
}
