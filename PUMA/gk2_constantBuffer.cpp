#include "gk2_constantBuffer.h"
#include <cassert>
#include "gk2_exceptions.h"
using namespace std;
using namespace gk2;

ConstantBufferBase::ConstantBufferBase(DeviceHelper& device, unsigned int dataSize, unsigned int dataCount)
	: m_dataSize(dataSize), m_dataCount(dataCount ? dataCount : 1), m_mapped(0)
{
	unsigned int bufferSize = m_dataCount * m_dataSize;
	unsigned int fill = 16 - (bufferSize%16);
	if (fill < 16)
		bufferSize += fill;
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = bufferSize;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_bufferObject = device.CreateBuffer(desc);
}

void ConstantBufferBase::Map(const shared_ptr<ID3D11DeviceContext>& context)
{
	if (m_mapped++)
		return;
	HRESULT hr = context->Map(m_bufferObject.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m_resource);
	if (FAILED(hr))
		THROW_DX11(hr);
}

void ConstantBufferBase::Unmap(const shared_ptr<ID3D11DeviceContext>& context)
{
	if (!m_mapped || --m_mapped)
		return;
	context->Unmap(m_bufferObject.get(), 0);
}

void ConstantBufferBase::Update(const shared_ptr<ID3D11DeviceContext>& context, const void* dataPtr, unsigned int dataCount)
{
	if (!dataCount)
		return;
	if (dataCount > m_dataCount)
		dataCount = m_dataCount;
	Map(context);
	memcpy(m_resource.pData, dataPtr, m_dataSize * dataCount);
	Unmap(context);
}