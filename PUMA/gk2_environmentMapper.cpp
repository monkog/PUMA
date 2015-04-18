#include "gk2_environmentMapper.h"

using namespace std;
using namespace gk2;

const wstring EnvironmentMapper::ShaderFile = L"resources/shaders/EnvMapShader.hlsl";
const int EnvironmentMapper::TEXTURE_SIZE = 256;

EnvironmentMapper::EnvironmentMapper(DeviceHelper& device, shared_ptr<ID3D11InputLayout>& layout,
									 const shared_ptr<ID3D11DeviceContext>& context, float nearPlane, float farPlane, XMFLOAT3 pos)
									 : EffectBase(context)
{
	Initialize(device, layout, ShaderFile);
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	m_position = XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
	m_face = (D3D11_TEXTURECUBE_FACE)-1;
	InitializeTextures(device);
}

void EnvironmentMapper::InitializeTextures(DeviceHelper& device)
{
	D3D11_TEXTURE2D_DESC texDesc = device.DefaultTexture2DDesc();

	//TODO: setup texture's width, height, mipLevels and bindflags

	texDesc.Width = TEXTURE_SIZE;
	texDesc.Height = TEXTURE_SIZE;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.MipLevels = 1;

	m_envFaceRenderTexture = device.CreateTexture2D(texDesc);
	m_envFaceRenderTarget = device.CreateRenderTargetView(m_envFaceRenderTexture);
	SIZE s;
	s.cx = s.cy = TEXTURE_SIZE;
	m_envFaceDepthTexture = device.CreateDepthStencilTexture(s);
	m_envFaceDepthView = device.CreateDepthStencilView(m_envFaceDepthTexture);

	//TODO: Create description for empty texture used as environment cube map
	//TODO: setup texture's width, height, mipLevels, bindflags, array size and miscFlags

	texDesc.Width = TEXTURE_SIZE;
	texDesc.Height = TEXTURE_SIZE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	texDesc.ArraySize = 6;

	m_envTexture = device.CreateTexture2D(texDesc);
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = device.DefaultShaderResourceDesc();

	//TODO: Create description of shader resource view for cube map

	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;

	m_envTextureView = device.CreateShaderResourceView(m_envTexture, srvDesc);
}

void EnvironmentMapper::SetupFace(const shared_ptr<ID3D11DeviceContext>& context, D3D11_TEXTURECUBE_FACE face)
{
	if (context != nullptr && context != m_context)
		m_context = context;
	if (m_context == nullptr)
		return;

	//TODO: Setup view and proj matrices

	XMVECTOR upVector, eyeDirection;

	switch(face)
	{
	case D3D11_TEXTURECUBE_FACE_POSITIVE_X :
		upVector = XMLoadFloat3(&XMFLOAT3(0,1,0));
		eyeDirection = XMLoadFloat3(&XMFLOAT3(1,0,0));
		break;
	case D3D11_TEXTURECUBE_FACE_NEGATIVE_X :
		upVector = XMLoadFloat3(&XMFLOAT3(0,1,0));
		eyeDirection = XMLoadFloat3(&XMFLOAT3(-1,0,0));
		break;
	case D3D11_TEXTURECUBE_FACE_POSITIVE_Y :
		upVector = XMLoadFloat3(&XMFLOAT3(0,0,-1));
		eyeDirection = XMLoadFloat3(&XMFLOAT3(0,1,0));
		break;
	case D3D11_TEXTURECUBE_FACE_NEGATIVE_Y :
		upVector = XMLoadFloat3(&XMFLOAT3(0,0,1));
		eyeDirection = XMLoadFloat3(&XMFLOAT3(0,-1,0));
		break;
	case D3D11_TEXTURECUBE_FACE_POSITIVE_Z :
		upVector = XMLoadFloat3(&XMFLOAT3(0,1,0));
		eyeDirection = XMLoadFloat3(&XMFLOAT3(0,0,1));
		break;
	case D3D11_TEXTURECUBE_FACE_NEGATIVE_Z :
		upVector = XMLoadFloat3(&XMFLOAT3(0,1,0));
		eyeDirection = XMLoadFloat3(&XMFLOAT3(0,0,-1));
		break;
	}
	XMMATRIX viewMatrix = XMMatrixLookToLH(XMLoadFloat4(&m_position), eyeDirection, upVector);

	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1, m_nearPlane, m_farPlane);

	//TODO: Replace with correct implementation
	m_viewCB->Update(m_context, viewMatrix);
	//TODO: Replace with correct implementation
	m_projCB->Update(m_context, perspectiveMatrix);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Height = TEXTURE_SIZE;
	viewport.Width = TEXTURE_SIZE;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	//TODO: Setup viewport

	m_context->RSSetViewports(1, &viewport);
	ID3D11RenderTargetView* targets[1] = { m_envFaceRenderTarget.get() };
	m_context->OMSetRenderTargets(1, targets, m_envFaceDepthView.get());
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_context->ClearRenderTargetView(m_envFaceRenderTarget.get(), clearColor);
	m_context->ClearDepthStencilView(m_envFaceDepthView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_face = face;
}

void EnvironmentMapper::EndFace()
{
	if (m_face < 0 || m_face > 5)
		return;
	//TODO: copy face texture to the environment cube map
	m_context->CopySubresourceRegion(m_envTexture.get(), m_face, 0, 0, 0, m_envFaceRenderTexture.get(), 0, nullptr);
}

void EnvironmentMapper::SetSamplerState(const shared_ptr<ID3D11SamplerState>& samplerState)
{
	if (samplerState != nullptr)
		m_samplerState = samplerState;
}

void EnvironmentMapper::SetCameraPosBuffer(const shared_ptr<ConstantBuffer<XMFLOAT4>>& cameraPos)
{
	if (cameraPos != nullptr)
		m_cameraPosCB = cameraPos;
}

void EnvironmentMapper::SetSurfaceColorBuffer(const shared_ptr<ConstantBuffer<XMFLOAT4>>& surfaceColor)
{
	if (surfaceColor != nullptr)
		m_surfaceColorCB = surfaceColor;
}

void EnvironmentMapper::SetVertexShaderData()
{
	ID3D11Buffer* vsb[4] = { m_worldCB->getBufferObject().get(), m_viewCB->getBufferObject().get(),
		m_projCB->getBufferObject().get(),  m_cameraPosCB->getBufferObject().get() };
	m_context->VSSetConstantBuffers(0, 4, vsb);
}

void EnvironmentMapper::SetPixelShaderData()
{
	ID3D11Buffer* psb[1] = { m_surfaceColorCB->getBufferObject().get() };
	m_context->PSSetConstantBuffers(0, 1, psb);
	ID3D11SamplerState* ss[1] = { m_samplerState.get() };
	m_context->PSSetSamplers(0, 1, ss);
	ID3D11ShaderResourceView* srv[1] = { m_envTextureView.get() };
	m_context->PSSetShaderResources(0, 1, srv);
}