#include "gk2_deviceHelper.h"
#include "gk2_utils.h"
#include "gk2_exceptions.h"
#include <cassert>
using namespace std;
using namespace gk2;


DeviceHelper::DeviceHelper(const shared_ptr<ID3D11Device>& deviceObject)
	: m_deviceObject(deviceObject)
{

}

DeviceHelper::DeviceHelper(const DeviceHelper& right)
	: m_deviceObject(right.m_deviceObject)
{

}

DeviceHelper& DeviceHelper::operator = (const DeviceHelper& right)
{
	m_deviceObject = right.m_deviceObject;
	return *this;
}

shared_ptr<ID3DBlob> DeviceHelper::CompileD3DShader(const wstring& filePath, const string& entry, const string& shaderModel)
{
	assert(m_deviceObject);
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* eb = nullptr, *b = nullptr;
	HRESULT result = D3DX11CompileFromFileW(filePath.c_str(), 0, 0, entry.c_str(), shaderModel.c_str(), shaderFlags,
		0, 0, &b, &eb, 0);
	shared_ptr<ID3DBlob> errorBuffer(eb, Utils::COMRelease);
	shared_ptr<ID3DBlob> buffer(b, Utils::COMRelease);
	if (FAILED(result))
	{
		if (errorBuffer)
		{
			char* msg = reinterpret_cast<char*>(errorBuffer->GetBufferPointer());
			OutputDebugStringA(msg);
		}
		THROW_DX11(result);
	}
	return buffer;
}

shared_ptr<ID3D11VertexShader> DeviceHelper::CreateVertexShader(shared_ptr<ID3DBlob> byteCode)
{
	assert(m_deviceObject);
	ID3D11VertexShader* vs;
	HRESULT result = m_deviceObject->CreateVertexShader(byteCode->GetBufferPointer(),
														byteCode->GetBufferSize(), 0, &vs);
	shared_ptr<ID3D11VertexShader> vertexShader(vs, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return vertexShader;
}

shared_ptr<ID3D11GeometryShader> DeviceHelper::CreateGeometryShader(shared_ptr<ID3DBlob> byteCode)
{
	assert(m_deviceObject);
	ID3D11GeometryShader* gs;
	HRESULT result = m_deviceObject->CreateGeometryShader(byteCode->GetBufferPointer(),
														  byteCode->GetBufferSize(), 0, &gs);
	shared_ptr<ID3D11GeometryShader> geometryShader(gs, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return geometryShader;
}

shared_ptr<ID3D11PixelShader> DeviceHelper::CreatePixelShader(shared_ptr<ID3DBlob> byteCode)
{
	assert(m_deviceObject);
	ID3D11PixelShader* ps;
	HRESULT result = m_deviceObject->CreatePixelShader(byteCode->GetBufferPointer(),
													   byteCode->GetBufferSize(), 0, &ps);
	shared_ptr<ID3D11PixelShader> pixelShader(ps, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return pixelShader;
}

shared_ptr<ID3D11InputLayout> DeviceHelper::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout,
	unsigned int layoutElements,
	shared_ptr<ID3DBlob> vsByteCode)
{
	assert(m_deviceObject);
	ID3D11InputLayout* il;
	HRESULT result = m_deviceObject->CreateInputLayout(layout, layoutElements, vsByteCode->GetBufferPointer(),
		vsByteCode->GetBufferSize(), &il);
	shared_ptr<ID3D11InputLayout> inputLayout(il, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return inputLayout;
}

shared_ptr<ID3D11Buffer> DeviceHelper::CreateBuffer(const D3D11_BUFFER_DESC& desc, const void* pData)
{
	assert(m_deviceObject);
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = pData;
	ID3D11Buffer* b;
	HRESULT result = m_deviceObject->CreateBuffer(&desc, pData?&data:nullptr, &b);
	shared_ptr<ID3D11Buffer> buffer(b, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return buffer;
}

shared_ptr<ID3D11Buffer> DeviceHelper::_CreateBufferInternal(const void* pData, unsigned int byteWidth,
	D3D11_BIND_FLAG bindFlags, D3D11_USAGE usage)
{
	assert(m_deviceObject);
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = usage;
	desc.BindFlags = bindFlags;
	desc.ByteWidth = byteWidth;
	if ((usage & D3D11_USAGE_DYNAMIC) != 0 || (usage & D3D11_USAGE_STAGING) != 0)
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	return CreateBuffer(desc, pData);
}

D3D11_TEXTURE2D_DESC DeviceHelper::DefaultTexture2DDesc()
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = 0;
	desc.Height = 0;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	return desc;
}

shared_ptr<ID3D11Texture2D> DeviceHelper::CreateTexture2D(const D3D11_TEXTURE2D_DESC& desc)
{
	assert(m_deviceObject);
	ID3D11Texture2D* t;
	HRESULT result = m_deviceObject->CreateTexture2D(&desc, nullptr, &t);
	shared_ptr<ID3D11Texture2D> texture(t, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return texture;
}

D3D11_SHADER_RESOURCE_VIEW_DESC DeviceHelper::DefaultShaderResourceDesc()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = -1;
	desc.Texture2D.MostDetailedMip = 0;
	return desc;
}

shared_ptr<ID3D11ShaderResourceView> DeviceHelper::CreateShaderResourceView(const shared_ptr<ID3D11Texture2D>& texture)
{
	return CreateShaderResourceView(texture, nullptr);
}

shared_ptr<ID3D11ShaderResourceView> DeviceHelper::CreateShaderResourceView(const shared_ptr<ID3D11Texture2D>& texture,
																			const D3D11_SHADER_RESOURCE_VIEW_DESC& d)
{
	return CreateShaderResourceView(texture, &d);
}

shared_ptr<ID3D11ShaderResourceView> DeviceHelper::CreateShaderResourceView(const shared_ptr<ID3D11Texture2D>& texture,
																			const D3D11_SHADER_RESOURCE_VIEW_DESC* d)
{
	assert(m_deviceObject);
	ID3D11ShaderResourceView* rv;
	HRESULT result = m_deviceObject->CreateShaderResourceView(texture.get(), d, &rv);
	shared_ptr<ID3D11ShaderResourceView> resourceView(rv, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return resourceView;
}
shared_ptr<ID3D11ShaderResourceView> DeviceHelper::CreateShaderResourceView(const wstring& fileName)
{
	assert(m_deviceObject);
	ID3D11ShaderResourceView* rv;
	HRESULT result = D3DX11CreateShaderResourceViewFromFileW(m_deviceObject.get(), fileName.c_str(), 0, 0, &rv, 0);
	shared_ptr<ID3D11ShaderResourceView> resourceView(rv, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return resourceView;
}

D3D11_SAMPLER_DESC DeviceHelper::DefaultSamplerDesc()
{
	D3D11_SAMPLER_DESC desc;
	desc.Filter = D3D11_FILTER_ANISOTROPIC;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MinLOD = -D3D11_FLOAT32_MAX;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 16;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.BorderColor[0] = 0.0f;
	desc.BorderColor[1] = 0.0f;
	desc.BorderColor[2] = 0.0f;
	desc.BorderColor[3] = 0.0f;
	return desc;
}

shared_ptr<ID3D11SamplerState> DeviceHelper::CreateSamplerState(const D3D11_SAMPLER_DESC& desc)
{
	assert(m_deviceObject);
	ID3D11SamplerState* s;
	HRESULT result = m_deviceObject->CreateSamplerState(&desc, &s);
	shared_ptr<ID3D11SamplerState> sampler(s, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return sampler;
}

shared_ptr<ID3D11Texture2D> DeviceHelper::CreateDepthStencilTexture(SIZE size)
{
	assert(m_deviceObject);
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = size.cx;
	desc.Height = size.cy;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	ID3D11Texture2D* dst;
	HRESULT result = m_deviceObject->CreateTexture2D(&desc, nullptr, &dst);
	shared_ptr<ID3D11Texture2D> depthStencilTexture(dst, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return depthStencilTexture;
}

shared_ptr<ID3D11RenderTargetView> DeviceHelper::CreateRenderTargetView(shared_ptr<ID3D11Texture2D> backBufferTexture)
{
	assert(m_deviceObject);
	ID3D11RenderTargetView* bb;
	HRESULT result = m_deviceObject->CreateRenderTargetView(backBufferTexture.get(), 0, &bb);
	shared_ptr<ID3D11RenderTargetView> backBuffer(bb, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return backBuffer;
}

shared_ptr<ID3D11DepthStencilView> DeviceHelper::CreateDepthStencilView(shared_ptr<ID3D11Texture2D> depthStencilTexture)
{
	assert(m_deviceObject);
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	ID3D11DepthStencilView* dsv;
	HRESULT result = m_deviceObject->CreateDepthStencilView(depthStencilTexture.get(), &desc, &dsv);
	shared_ptr<ID3D11DepthStencilView> depthStencilView(dsv, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return depthStencilView;
}

D3D11_DEPTH_STENCIL_DESC DeviceHelper::DefaultDepthStencilDesc()
{
	assert(m_deviceObject);
	D3D11_DEPTH_STENCIL_DESC desc;
	//Depth test parameters
	desc.DepthEnable = true; //Enable depth test
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS; //Depth test is passed if pixel's z value is less than current depth buffer value

	//Stencil test parameters
	desc.StencilEnable = false; //Disable depth stencil test
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	//Stencil operations if pixel is front-facing
	//Operation performed when pixel fails stencil test
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//Operation performed when pixel passes the stencil test but fails depth test
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	//Operation performed when pixel passes both stencil and depth test
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; //stencil test is always passed

	//Stencil operations if pixel is back-facing
	//Operation performed when pixel fails stencil test
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//Operation performed when pixel passes the stencil test but fails depth test
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	//Operation performed when pixel passes both stencil and depth test
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS; //stencil test is always passed
	return desc;
}

shared_ptr<ID3D11DepthStencilState> DeviceHelper::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc)
{
	assert(m_deviceObject);
	ID3D11DepthStencilState* s;
	HRESULT result = m_deviceObject->CreateDepthStencilState(&desc, &s);
	shared_ptr<ID3D11DepthStencilState> state(s, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return state;
}

D3D11_RASTERIZER_DESC DeviceHelper::DefaultRasterizerDesc()
{
	assert(m_deviceObject);
	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = D3D11_FILL_SOLID; //Determines the solid fill mode (as opposed to wireframe)
	desc.CullMode = D3D11_CULL_BACK; //Indicates that back facing triangles are not drawn
	desc.FrontCounterClockwise = false; //Indicates that vertices of a front facing triangle are counter-clockwise on the render target
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = true;
	desc.ScissorEnable = false;
	desc.MultisampleEnable = false;
	desc.AntialiasedLineEnable = false;
	return desc;
}

shared_ptr<ID3D11RasterizerState> DeviceHelper::CreateRasterizerState(const D3D11_RASTERIZER_DESC& desc)
{
	assert(m_deviceObject);
	ID3D11RasterizerState* s;
	HRESULT result = m_deviceObject->CreateRasterizerState(&desc, &s);
	shared_ptr<ID3D11RasterizerState> state(s, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return state;
}

D3D11_BLEND_DESC DeviceHelper::DefaultBlendDesc()
{
	assert(m_deviceObject);
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	return desc;
}

shared_ptr<ID3D11BlendState> DeviceHelper::CreateBlendState(const D3D11_BLEND_DESC& desc)
{
	assert(m_deviceObject);
	ID3D11BlendState* s;
	HRESULT result = m_deviceObject->CreateBlendState(&desc, &s);
	shared_ptr<ID3D11BlendState> state(s, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	return state;
}
