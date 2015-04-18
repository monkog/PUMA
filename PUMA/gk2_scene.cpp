#include "gk2_scene.h"
#include "gk2_window.h"
#include "gk2_textureGenerator.h"

using namespace std;
using namespace gk2;

const XMFLOAT4 Scene::LIGHT_POS[2] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f) };
const unsigned int Scene::BS_MASK = 0xffffffff;

Scene::Scene(HINSTANCE hInstance)
	: ApplicationBase(hInstance), m_camera(0.01f, 100.0f)
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
	m_camera.Zoom(5);
	UpdateCamera();
}

void Scene::InitializeTextures()
{
	m_floorTexture = m_device.CreateShaderResourceView(L"resources/textures/brick_wall.jpg");

	D3D11_SAMPLER_DESC sd = m_device.DefaultSamplerDesc();
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_samplerWrap = m_device.CreateSamplerState(sd);
	sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	m_samplerBorder = m_device.CreateSamplerState(sd);

	D3D11_TEXTURE2D_DESC texDesc = m_device.DefaultTexture2DDesc();
	texDesc.Width = 64;
	texDesc.Height = 512;
	texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	shared_ptr<ID3D11Texture2D> woodTexture = m_device.CreateTexture2D(texDesc);
	shared_ptr<BYTE> data(new BYTE[64 * 512 * 4], Utils::DeleteArray<BYTE>);
	BYTE *d = data.get();
	TextureGenerator txGen(6, 0.35f);
	for (int i = 0; i < 512; ++i)
	{
		float x = i / 512.0f;
		for (int j = 0; j < 64; ++j)
		{
			float y = j / 64.0f;
			float c = txGen.Wood(x, y);
			BYTE ic = static_cast<BYTE>(c * 239);
			*(d++) = ic;
			ic = static_cast<BYTE>(c * 200);
			*(d++) = ic;
			ic = static_cast<BYTE>(c * 139);
			*(d++) = ic;
			*(d++) = 255;
		}
	}
	m_context->UpdateSubresource(woodTexture.get(), 0, 0, data.get(), 64*4, 64*512*4);
}

void Scene::CreateScene()
{
	m_floor = m_meshLoader.GetQuad(4.0f);
	float a = 0;
	m_floor.setWorldMatrix(XMMatrixTranslation(0.0f, 0.0f, 2.0f) * XMMatrixRotationX(XM_PIDIV2));
	m_lightPosCB->Update(m_context, LIGHT_POS);
}

void Scene::InitializeRenderStates()
{
	D3D11_RASTERIZER_DESC rsDesc = m_device.DefaultRasterizerDesc();
	rsDesc.CullMode = D3D11_CULL_FRONT;
	m_rsCullFront = m_device.CreateRasterizerState(rsDesc);

	D3D11_BLEND_DESC bsDesc = m_device.DefaultBlendDesc();
	bsDesc.RenderTarget[0].BlendEnable = true;
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_bsAlpha = m_device.CreateBlendState(bsDesc);

	D3D11_DEPTH_STENCIL_DESC dssDesc = m_device.DefaultDepthStencilDesc();
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dssNoWrite = m_device.CreateDepthStencilState(dssDesc);
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

	m_particles.reset(new ParticleSystem(m_device,  XMFLOAT3(-1.3f, -0.6f, -0.14f)));
	m_particles->SetViewMtxBuffer(m_viewCB);
	m_particles->SetProjMtxBuffer(m_projCB);
	m_particles->SetSamplerState(m_samplerWrap);
	return true;
}

void Scene::UnloadContent()
{

}

void Scene::UpdateCamera()
{
	XMMATRIX view;
	m_camera.GetViewMatrix(view);
	m_viewCB->Update(m_context, view);
	m_cameraPosCB->Update(m_context, m_camera.GetPosition());
}

void Scene::Update(float dt)
{
	static MouseState prevState;
	MouseState currentState;
	if (m_mouse->GetState(currentState))
	{
		bool change = true;
		if (prevState.isButtonDown(0))
		{
			POINT d = currentState.getMousePositionChange();
			m_camera.Rotate(d.y/300.f, d.x/300.f);
		}
		else if (prevState.isButtonDown(1))
		{
			POINT d = currentState.getMousePositionChange();
			m_camera.Zoom(d.y/10.0f);
		}
		else
			change = false;
		prevState = currentState;
		if (change)
			UpdateCamera();
	}
	m_particles->Update(m_context, dt, m_camera.GetPosition());
}

void Scene::DrawWalls()
{
	//Draw floor
	m_textureCB->Update(m_context, XMMatrixScaling(0.25f, 4.0f, 1.0f) * XMMatrixTranslation(0.5f, 0.5f, 0.0f));
	m_textureEffect->SetTexture(m_floorTexture);
	m_textureEffect->Begin(m_context);
	m_worldCB->Update(m_context, m_floor.getWorldMatrix());
	m_floor.Render(m_context);
	m_textureEffect->End();
}

void Scene::DrawTransparentObjects()
{
}

void Scene::DrawScene()
{

	DrawWalls();
	m_phongEffect->Begin(m_context);
	m_phongEffect->End();
	DrawTransparentObjects();
}

void Scene::Render()
{
	if (m_context == nullptr)
		return;

	//TODO: Render scene to each environment cube map face
	for(int i = 0; i < 6; i++)
	{
		m_environmentMapper->SetupFace(m_context, static_cast<D3D11_TEXTURECUBE_FACE>(i));
		DrawScene();
		m_environmentMapper->EndFace();
	}

	ResetRenderTarget();
	m_projCB->Update(m_context, m_projMtx);
	UpdateCamera();
	//Clear buffers
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_context->ClearRenderTargetView(m_backBuffer.get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	DrawScene();
	m_swapChain->Present(0, 0);
}
