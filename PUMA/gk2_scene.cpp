#include "gk2_scene.h"
#include "gk2_window.h"
#include "mtxlib.h"

using namespace std;
using namespace gk2;
float m_totalPosition = 0.0f;
float m_counter = 0.0f;
const XMFLOAT4 gk2::Scene::LIGHT_POS = XMFLOAT4(-10.0f, 5.0f, 10.0f, 1.0f);
const unsigned int Scene::BS_MASK = 0xffffffff;
const float Radius = 0.75f;
Scene::Scene(HINSTANCE hInstance)
: ApplicationBase(hInstance), m_camera()
{

}

Scene::~Scene()
{

}

void* Scene::operator new(size_t size)
{
	return Utils::New16Aligned(size);
}

void Scene::operator delete(void* ptr)
{
	Utils::Delete16Aligned(ptr);
}

void Scene::InitializeConstantBuffers()
{
	m_projCB.reset(new CBMatrix(m_device));
	m_viewCB.reset(new CBMatrix(m_device));
	m_worldCB.reset(new CBMatrix(m_device));
	m_lightPosCB.reset(new ConstantBuffer<XMFLOAT4, 2>(m_device));
	m_textureCB.reset(new CBMatrix(m_device));
	m_surfaceColorCB.reset(new ConstantBuffer<XMFLOAT4>(m_device));
	m_cameraPosCB.reset(new ConstantBuffer<XMFLOAT4>(m_device));
}

void Scene::InitializeCamera()
{
	SIZE s = getMainWindow()->getClientSize();
	float ar = static_cast<float>(s.cx) / s.cy;
	m_projMtx = XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f);
	m_projCB->Update(m_context, m_projMtx);
	UpdateCamera(XMMatrixIdentity());
}

void Scene::InitializeTextures()
{
	m_floorTexture = m_device.CreateShaderResourceView(L"resources/textures/metal_texture.jpg");
	//m_metalTexture = m_device.CreateShaderResourceView(L"resources/textures/scratch_texture.jpg");

	D3D11_SAMPLER_DESC sd = m_device.DefaultSamplerDesc();
	sd.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	m_samplerWrap = m_device.CreateSamplerState(sd);
	sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	m_samplerBorder = m_device.CreateSamplerState(sd);
}

void Scene::CreateScene()
{
	m_circle = m_meshLoader.GetCircle(Radius);
	m_circle.setWorldMatrix(XMMatrixIdentity());

	m_floor = m_meshLoader.GetQuad(4.0f);
	m_floor.setWorldMatrix(XMMatrixTranslation(0.0f, 0.0f, 2.0f) * XMMatrixRotationX(XM_PIDIV2));

	m_metal = m_meshLoader.GetQuad(2.0f);
	m_metal.setWorldMatrix(XMMatrixTranslation(0.0f, 0.0f, 1.79f) *XMMatrixRotationX(XM_PIDIV2 / 3));

	m_lightPosCB->Update(m_context, LIGHT_POS);

	m_robot[0] = m_meshLoader.LoadMesh(L"resources/meshes/mesh1.txt");
	m_robot[1] = m_meshLoader.LoadMesh(L"resources/meshes/mesh2.txt");
	m_robot[2] = m_meshLoader.LoadMesh(L"resources/meshes/mesh3.txt");
	m_robot[3] = m_meshLoader.LoadMesh(L"resources/meshes/mesh4.txt");
	m_robot[4] = m_meshLoader.LoadMesh(L"resources/meshes/mesh5.txt");
	m_robot[5] = m_meshLoader.LoadMesh(L"resources/meshes/mesh6.txt");

	for (int i = 0; i < 6; i++)
	{
		m_robot[i].setWorldMatrix(XMMatrixTranslation(0.0f, -1.0f, 0.0f) * XMMatrixRotationY(XM_PIDIV2));
		m_robotRotationMatrix[i] = XMMatrixIdentity();
	}
	m_surfaceColorCB->Update(m_context, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
}

vector3 Scene::GetPositionOnCircle(bool ifParticle)
{
	XMFLOAT3 p = ifParticle ? m_meshLoader.GetCirclePartVertex(((int)m_counter) % 360) : m_meshLoader.GetCircleVertex(((int)m_counter) % 360);
	m_counter += 0.1f;
	return vector3(p.x, p.y, p.z);
}

void Scene::InitializeRenderStates()
{
	D3D11_DEPTH_STENCIL_DESC dssDesc = m_device.DefaultDepthStencilDesc();
	dssDesc.StencilEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;

	m_dssWrite = m_device.CreateDepthStencilState(dssDesc);
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	m_dssTest = m_device.CreateDepthStencilState(dssDesc);

	D3D11_RASTERIZER_DESC rsDesc = m_device.DefaultRasterizerDesc();
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.FrontCounterClockwise = true;
	m_rsCullFront = m_device.CreateRasterizerState(rsDesc);

	D3D11_BLEND_DESC bsDesc = m_device.DefaultBlendDesc();
	bsDesc.RenderTarget[0].BlendEnable = true;
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	m_bsAlpha = m_device.CreateBlendState(bsDesc);
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_bsAdd = m_device.CreateBlendState(bsDesc);
}

bool Scene::LoadContent()
{
	m_meshLoader.setDevice(m_device);
	InitializeConstantBuffers();
	InitializeCamera();
	InitializeTextures();
	InitializeRenderStates();
	m_meshLoader.setDevice(m_device);
	CreateScene();

	m_phongEffect.reset(new PhongEffect(m_device, m_layout));
	m_phongEffect->SetProjMtxBuffer(m_projCB);
	m_phongEffect->SetViewMtxBuffer(m_viewCB);
	m_phongEffect->SetWorldMtxBuffer(m_worldCB);
	m_phongEffect->SetLightPosBuffer(m_lightPosCB);
	m_phongEffect->SetSurfaceColorBuffer(m_surfaceColorCB);

	m_textureEffect.reset(new TextureEffect(m_device, m_layout));
	m_textureEffect->SetProjMtxBuffer(m_projCB);
	m_textureEffect->SetViewMtxBuffer(m_viewCB);
	m_textureEffect->SetWorldMtxBuffer(m_worldCB);
	m_textureEffect->SetTextureMtxBuffer(m_textureCB);
	m_textureEffect->SetSamplerState(m_samplerWrap);
	m_textureEffect->SetTexture(m_floorTexture);

	m_environmentMapper.reset(new EnvironmentMapper(m_device, m_layout, m_context, 0.4f, 8.0f,
		XMFLOAT3(-1.3f, -0.74f, -0.6f)));
	m_environmentMapper->SetProjMtxBuffer(m_projCB);
	m_environmentMapper->SetViewMtxBuffer(m_viewCB);
	m_environmentMapper->SetWorldMtxBuffer(m_worldCB);
	m_environmentMapper->SetSamplerState(m_samplerWrap);
	m_environmentMapper->SetCameraPosBuffer(m_cameraPosCB);
	m_environmentMapper->SetSurfaceColorBuffer(m_surfaceColorCB);

	m_particles.reset(new ParticleSystem(m_device, XMFLOAT3(-1.3f, -0.6f, -0.14f)));
	m_particles->SetViewMtxBuffer(m_viewCB);
	m_particles->SetProjMtxBuffer(m_projCB);
	return true;
}

void Scene::UnloadContent()
{

}

void Scene::UpdateCamera(XMMATRIX& viewM)
{
	m_camera.GetViewMatrix(viewM);
	m_viewCB->Update(m_context, &viewM);
	m_cameraPosCB->Update(m_context, m_camera.GetPosition());
}

void Scene::ReadKeyboard(bool isRotate)
{
	static KeyboardState state;
	if (!m_keyboard->GetState(state))
		return;
	float factor = 0.01f;
	if (state.isKeyDown(DIK_RIGHT))
	{
		if (isRotate)
			m_camera.Rotate(1, 0);
		else
			m_camera.Move(0, 0, -1);
	}
	if (state.isKeyDown(DIK_LEFT))
	{
		if (isRotate)
			m_camera.Rotate(-1, 0);
		else
			m_camera.Move(0, 0, 1);
	}
	if (state.isKeyDown(DIK_UP))
	{
		if (isRotate)
			m_camera.Rotate(0, 1);
		else
			m_camera.Move(1, 0, 0);
	}
	if (state.isKeyDown(DIK_DOWN))
	{
		if (isRotate)
			m_camera.Rotate(0, -1);
		else
			m_camera.Move(-1, 0, 0);
	}
	if (state.isKeyDown(DIK_RSHIFT))
		m_camera.Move(0, 1, 0);
	if (state.isKeyDown(DIK_RCONTROL))
		m_camera.Move(0, -1, 0);
}

void Scene::Update(float dt)
{
	MoveSheldersMatrix();
	static MouseState prevState;
	MouseState currentState;
	if (m_mouse->GetState(currentState))
	{
		if (prevState.isButtonDown(0))
			ReadKeyboard(true);
		else
			ReadKeyboard(false);
		prevState = currentState;
	}
	UpdateCamera(XMMatrixIdentity());
	m_particles->Update(m_context, dt, m_camera.GetPosition());
}

void Scene::DrawMirroredWorld()
{
	XMVECTOR v;
	m_context->OMSetDepthStencilState(m_dssWrite.get(), 1);
	m_worldCB->Update(m_context, m_metal.getWorldMatrix());
	m_metal.Render(m_context);
	m_context->OMSetDepthStencilState(m_dssTest.get(), 1);

	XMMATRIX m = XMMatrixTranslation(0.0f, 0.0f, 1.79f) *XMMatrixRotationX(XM_PIDIV2 / 3);
	XMMATRIX m_mirrorMtx = XMMatrixInverse(&v, m)* XMMatrixScaling(1, 1, -1) *m;

	XMMATRIX viewMtx = m_camera.GetViewMatrix();
	UpdateCamera(m_mirrorMtx*viewMtx);
	m_context->RSSetState(m_rsCullFront.get());

	m_phongEffect->Begin(m_context);
	DrawRobot();
	m_phongEffect->End();
	DrawQuads();

	m_context->RSSetState(NULL);
	UpdateCamera(viewMtx);
	m_context->OMSetDepthStencilState(NULL, 0);
}

void Scene::DrawQuads()
{
	m_textureCB->Update(m_context, XMMatrixScaling(0.25f, 4.0f, 1.0f) * XMMatrixTranslation(0.5f, 0.5f, 0.0f));
	m_textureEffect->SetTexture(m_floorTexture);
	m_textureEffect->Begin(m_context);
	m_worldCB->Update(m_context, m_floor.getWorldMatrix());
	m_floor.Render(m_context);
	m_textureEffect->End();
}

void Scene::DrawRobot()
{
	XMMATRIX tmp;
	m_context->RSSetState(m_rsCullFront.get());
	for (int i = 0; i < 6; ++i)
	{
		m_worldCB->Update(m_context, m_robotRotationMatrix[i] * m_robot[i].getWorldMatrix());
		m_robot[i].Render(m_context);
	}
}

void Scene::DrawMetal(bool b)
{
	m_surfaceColorCB->Update(m_context, XMFLOAT4(0.8f, 0.7f, 0.65f, 0.5f));

	m_worldCB->Update(m_context, m_metal.getWorldMatrix());
	m_metal.Render(m_context);

	m_surfaceColorCB->Update(m_context, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
}

void Scene::DrawCircleOnMetal()
{
	m_worldCB->Update(m_context, m_circle.getWorldMatrix());
	m_circle.RenderCircle(m_context);
}

void Scene::DrawScene(bool b)
{
	if (b)
		DrawMirroredWorld();

	m_context->OMSetBlendState(m_bsAlpha.get(), 0, BS_MASK);
	DrawMetal(false);
	m_context->OMSetBlendState(0, 0, BS_MASK);

	DrawRobot();
	DrawCircleOnMetal();
	DrawQuads();
}

void Scene::MoveSheldersMatrix()
{
	vector3 norm = vector3(sqrtf(3) / 2.0f, 0.5f, 0.0f);
	vector3 pos = GetPositionOnCircle(false);
	float a1, a2, a3, a4, a5;
	inverse_kinematics(pos, norm, a1, a2, a3, a4, a5);

	XMMATRIX Rotations[6];
	m_robotRotationMatrix[1] = Rotations[1] = XMMatrixRotationY(a1);
	Rotations[2] = XMMatrixRotationZ(a2)* XMMatrixTranslation(0.0f, 0.27f, 0.0f)* Rotations[1];
	m_robotRotationMatrix[2] = XMMatrixTranslation(0.0f, -0.27f, 0.0f)* Rotations[2];
	Rotations[3] = XMMatrixRotationZ(a3)* XMMatrixTranslation(-0.91f, 0.0f, 0.0f)* Rotations[2];
	m_robotRotationMatrix[3] = XMMatrixTranslation(0.91f, -0.27f, 0.0f)*Rotations[3];
	Rotations[4] = XMMatrixRotationX(a4)* XMMatrixTranslation(0.91f, 0.0f, -0.26f)* Rotations[3];
	m_robotRotationMatrix[4] = XMMatrixTranslation(0.0f, -0.27f, 0.26f)*Rotations[4];
	Rotations[5] = XMMatrixRotationZ(a5)* XMMatrixTranslation(-1.72f, 0.0f, 0.26f)* Rotations[4];
	m_robotRotationMatrix[5] = XMMatrixTranslation(1.72f, -0.27f, 0.0f)*Rotations[5];

	pos = GetPositionOnCircle(true);
	m_particles.get()->m_emitterPos = XMFLOAT3(pos.x, pos.y, pos.z);
	m_particles.get()->m_startPosition = XMFLOAT3(pos.x, pos.y, pos.z);
}

void Scene::Render()
{
	if (m_context == nullptr)
		return;

	m_phongEffect->Begin(m_context);
	DrawScene(true);
	m_phongEffect->End();

	ResetRenderTarget();
	m_projCB->Update(m_context, m_projMtx);
	UpdateCamera(XMMatrixIdentity());

	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_context->ClearRenderTargetView(m_backBuffer.get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_phongEffect->Begin(m_context);
	DrawScene(false);
	m_phongEffect->End();

	m_particles->Render(m_context);
	m_swapChain->Present(0, 0);
}

void Scene::inverse_kinematics(vector3 pos, vector3 normal, float &a1, float &a2, float &a3, float &a4, float &a5)
{
	float l1 = .91f, l2 = .81f, l3 = .33f, dy = .27f, dz = .26f;
	normal.normalize();
	vector3 pos1 = pos + normal * l3;
	float e = sqrtf(pos1.z*pos1.z + pos1.x*pos1.x - dz*dz);
	a1 = atan2(pos1.z, -pos1.x) + atan2(dz, e);
	vector3 pos2(e, pos1.y - dy, .0f);
	a3 = -acosf(min(1.0f, (pos2.x*pos2.x + pos2.y*pos2.y - l1*l1 - l2*l2)
		/ (2.0f*l1*l2)));
	float k = l1 + l2 * cosf(a3), l = l2 * sinf(a3);
	a2 = -atan2(pos2.y, sqrtf(pos2.x*pos2.x + pos2.z*pos2.z)) - atan2(l, k);
	vector3 normal1;
	normal1 = vector3(RotateRadMatrix44('y', -a1) *
		vector4(normal.x, normal.y, normal.z, .0f));
	normal1 = vector3(RotateRadMatrix44('z', -(a2 + a3)) *
		vector4(normal1.x, normal1.y, normal1.z, .0f));
	a5 = acosf(normal1.x);
	a4 = atan2(normal1.z, normal1.y);
}