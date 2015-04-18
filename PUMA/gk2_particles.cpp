#include "gk2_particles.h"
#include <ctime>
#include "gk2_exceptions.h"
#include <vector>
#include <algorithm>

using namespace std;
using namespace gk2;

const D3D11_INPUT_ELEMENT_DESC ParticleVertex::Layout[ParticleVertex::LayoutElements] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

bool ParticleComparer::operator()(const ParticleVertex& p1, const ParticleVertex& p2)
{
	XMVECTOR p1Pos = XMLoadFloat3(&(p1.Pos));
	p1Pos.m128_f32[3] = 1.0f;
	XMVECTOR p2Pos = XMLoadFloat3(&(p2.Pos));
	p2Pos.m128_f32[3] = 1.0f;
	XMVECTOR camDir = XMLoadFloat4(&m_camDir);
	XMVECTOR camPos = XMLoadFloat4(&m_camPos);
	float d1 = XMVector3Dot(p1Pos - camPos, camDir).m128_f32[0];
	float d2 = XMVector3Dot(p2Pos - camPos, camDir).m128_f32[0];
	return d1 > d2;
}

const XMFLOAT3 ParticleSystem::EMITTER_DIR = XMFLOAT3(0.0f, 1.0f, 0.0f);
const float ParticleSystem::TIME_TO_LIVE = 4.0f;
const float ParticleSystem::EMISSION_RATE = 10.0f;
const float ParticleSystem::MAX_ANGLE = XM_PIDIV2 / 9.0f;
const float ParticleSystem::MIN_VELOCITY = 0.2f;
const float ParticleSystem::MAX_VELOCITY = 0.33f;
const float ParticleSystem::PARTICLE_SIZE = 0.08f;
const float ParticleSystem::PARTICLE_SCALE = 1.0f;
const float ParticleSystem::MIN_ANGLE_VEL = -XM_PI;
const float ParticleSystem::MAX_ANGLE_VEL = XM_PI;
const int ParticleSystem::MAX_PARTICLES = 500;

const unsigned int ParticleSystem::STRIDE = sizeof(ParticleVertex);
const unsigned int ParticleSystem::OFFSET = 0;

ParticleSystem::ParticleSystem(DeviceHelper& device, XMFLOAT3 emitterPos)
	: m_particlesCount(0), m_particlesToCreate(0.0f), m_emitterPos(emitterPos)
{
	srand(time(0));
	m_vertices = device.CreateVertexBuffer<ParticleVertex>(MAX_PARTICLES, D3D11_USAGE_DYNAMIC);
	shared_ptr<ID3DBlob> vsByteCode = device.CompileD3DShader(L"resources/shaders/Particles.hlsl", "VS_Main", "vs_4_0");
	shared_ptr<ID3DBlob> gsByteCode = device.CompileD3DShader(L"resources/shaders/Particles.hlsl", "GS_Main", "gs_4_0");
	shared_ptr<ID3DBlob> psByteCode = device.CompileD3DShader(L"resources/shaders/Particles.hlsl", "PS_Main", "ps_4_0");
	m_vs = device.CreateVertexShader(vsByteCode);
	m_gs = device.CreateGeometryShader(gsByteCode);
	m_ps = device.CreatePixelShader(psByteCode);
	m_layout = device.CreateInputLayout<ParticleVertex>(vsByteCode);
	m_cloudTexture = device.CreateShaderResourceView(L"resources/textures/smoke.png");
	m_opacityTexture = device.CreateShaderResourceView(L"resources/textures/smokecolors.png");
}

void ParticleSystem::SetViewMtxBuffer(const shared_ptr<CBMatrix>& view)
{
	if (view != nullptr)
		m_viewCB = view;
}

void ParticleSystem::SetProjMtxBuffer(const shared_ptr<CBMatrix>& proj)
{
	if (proj != nullptr)
		m_projCB = proj;
}

void ParticleSystem::SetSamplerState(const shared_ptr<ID3D11SamplerState>& samplerState)
{
	if (samplerState != nullptr)
		m_samplerState = samplerState;
}

XMFLOAT3 ParticleSystem::RandomVelocity()
{
	float x, y;
	do
	{
		x = 2.0f * static_cast<float>(rand()) / RAND_MAX - 1.0f;
		y = 2.0f * static_cast<float>(rand()) / RAND_MAX - 1.0f;
	} while (x*x + y*y > 1.0f);
	float a = tan(MAX_ANGLE);
	XMFLOAT3 v(x * a, 1.0f, y * a);
	XMVECTOR velocity = XMLoadFloat3(&v);
	float  len = MIN_VELOCITY + (MAX_VELOCITY - MIN_VELOCITY) *
		static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	velocity = len * XMVector3Normalize(velocity);
	XMStoreFloat3(&v, velocity);
	return v;
}

void ParticleSystem::AddNewParticle()
{
	Particle p;
	//TODO: Setup initial values
	p.Vertex.Age = 0;
	p.Vertex.Size = PARTICLE_SIZE;
	p.Velocities.Velocity = RandomVelocity();
	p.Vertex.Pos = m_emitterPos;
	p.Vertex.Angle = 0;
	p.Velocities.AngleVelocity = MIN_ANGLE_VEL + (MAX_ANGLE_VEL - MIN_ANGLE_VEL) * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	m_particles.push_back(p);
}

XMFLOAT3 operator *(const XMFLOAT3& v1, float d)
{
	return XMFLOAT3(v1.x * d, v1.y * d, v1.z * d);
}

XMFLOAT3 operator +(const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	return XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

XMFLOAT4 operator -(const XMFLOAT4& v1, const XMFLOAT4& v2)
{
	XMFLOAT4 res;
	res.x = v1.x - v2.x;
	res.y = v1.y - v2.y;
	res.z = v1.z - v2.z;
	res.w = v1.w - v2.w;
	return res;
}

void ParticleSystem::UpdateParticle(Particle& p, float dt)
{
	//TODO: Update particle's fields;
	p.Vertex.Age += dt;
	p.Vertex.Pos = p.Vertex.Pos + p.Velocities.Velocity * dt;
	p.Vertex.Angle += p.Velocities.AngleVelocity * dt;
	p.Vertex.Size += PARTICLE_SCALE * PARTICLE_SIZE * dt;
}

void ParticleSystem::UpdateVertexBuffer(shared_ptr<ID3D11DeviceContext>& context, XMFLOAT4 cameraPos)
{
	vector<ParticleVertex> vertices(MAX_PARTICLES);
	XMFLOAT4 cameraTarget(0.0f, 0.0f, 0.0f, 1.0f);
	//TODO: copy ParticleVertex values form the list to the vector and sort them using ParticleComparer
	int index = 0;
	for (list<gk2::Particle>::iterator it = m_particles.begin(); it != m_particles.end(); it++)
	{
		vertices.at(index) = it->Vertex;
		index++;
	}
	sort(vertices.begin(), vertices.end(), ParticleComparer(cameraTarget, cameraPos));

	D3D11_MAPPED_SUBRESOURCE resource;
	HRESULT hr = context->Map(m_vertices.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	if (FAILED(hr))
		THROW_DX11(hr);
	memcpy(resource.pData, vertices.data(), MAX_PARTICLES * sizeof(ParticleVertex));
	context->Unmap(m_vertices.get(), 0);
}

void ParticleSystem::Update(shared_ptr<ID3D11DeviceContext>& context, float dt, XMFLOAT4 cameraPos)
{
	//TODO: Update existing particles and remove them if necessary
	for (list<gk2::Particle>::iterator it = m_particles.begin(); it != m_particles.end(); it++)
		UpdateParticle(*it, dt);

	m_particles.erase(remove_if(m_particles.begin(), m_particles.end(), [](gk2::Particle particle){  return particle.Vertex.Age > TIME_TO_LIVE; }), m_particles.end());
	
	//TODO: Create new particles if needed.
	m_particlesToCreate = EMISSION_RATE * dt;
	for (; m_particles.size() < MAX_PARTICLES && m_particlesToCreate > 0; m_particlesToCreate--)
		AddNewParticle();

	m_particlesCount = m_particles.size();
	UpdateVertexBuffer(context, cameraPos);
}

void ParticleSystem::Render(shared_ptr<ID3D11DeviceContext>& context)
{
	context->VSSetShader(m_vs.get(), nullptr, 0);
	context->GSSetShader(m_gs.get(), nullptr, 0);
	context->PSSetShader(m_ps.get(), nullptr, 0);
	context->IASetInputLayout(m_layout.get());
	ID3D11Buffer* vsb[1] = { m_viewCB->getBufferObject().get() };
	context->VSSetConstantBuffers(0, 1, vsb);
	ID3D11Buffer* gsb[1] = { m_projCB->getBufferObject().get() };
	context->GSSetConstantBuffers(0, 1, gsb);
	ID3D11ShaderResourceView* psv[2] = { m_cloudTexture.get(), m_opacityTexture.get() };
	context->PSSetShaderResources(0, 2, psv);
	ID3D11SamplerState* pss[1] = { m_samplerState.get() };
	context->PSSetSamplers(0, 1, pss);
	ID3D11Buffer* vb[1] = { m_vertices.get() };
	unsigned int offset = 0;
	context->IASetVertexBuffers(0, 1, vb, &STRIDE, &OFFSET);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->Draw(m_particlesCount, 0);
	context->GSSetShader(nullptr, nullptr, 0);
}