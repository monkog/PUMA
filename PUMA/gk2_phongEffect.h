#ifndef __GK2_PHONG_EFFECT_H_
#define __GK2_PHONG_EFFECT_H_

#include "gk2_effectBase.h"

namespace gk2
{
	class PhongEffect : public EffectBase
	{
	public:
		PhongEffect(gk2::DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
					std::shared_ptr<ID3D11DeviceContext> context = nullptr);

		void SetLightPosBuffer(const std::shared_ptr<gk2::ConstantBuffer<XMFLOAT4, 2>>& lightPos);
		void SetSurfaceColorBuffer(const std::shared_ptr<gk2::ConstantBuffer<XMFLOAT4>>& surfaceColor);

	protected:
		virtual void SetVertexShaderData();
		virtual void SetPixelShaderData();

	private:
		static const std::wstring ShaderFile;

		std::shared_ptr<gk2::ConstantBuffer<XMFLOAT4, 2>> m_lightPosCB;
		std::shared_ptr<gk2::ConstantBuffer<XMFLOAT4>> m_surfaceColorCB;
	};
}

#endif __GK2_PHONG_EFFECT_H_
