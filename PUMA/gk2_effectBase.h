#ifndef __GK2_EFFECT_BASE_H_
#define __GK2_EFFECT_BASE_H_

#include <d3d11.h>
#include "gk2_deviceHelper.h"
#include "gk2_constantBuffer.h"
#include <memory>
#include <string>

namespace gk2
{
	class EffectBase
	{
	public:
		void SetWorldMtxBuffer(const std::shared_ptr<gk2::CBMatrix>& world);
		void SetViewMtxBuffer(const std::shared_ptr<gk2::CBMatrix>& view);
		void SetProjMtxBuffer(const std::shared_ptr<gk2::CBMatrix>& proj);

		void Begin(std::shared_ptr<ID3D11DeviceContext> context = nullptr);
		void End();

	protected:
		EffectBase(std::shared_ptr<ID3D11DeviceContext> context = nullptr);

		virtual void SetVertexShaderData() = 0;
		virtual void SetPixelShaderData() = 0;

		std::shared_ptr<gk2::CBMatrix> m_worldCB;
		std::shared_ptr<gk2::CBMatrix> m_viewCB;
		std::shared_ptr<gk2::CBMatrix> m_projCB;
		std::shared_ptr<ID3D11DeviceContext> m_context;

		void Initialize(gk2::DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
						const std::wstring& shaderFile);

	private:
		std::shared_ptr<ID3D11VertexShader> m_vs;
		std::shared_ptr<ID3D11PixelShader> m_ps;
		std::shared_ptr<ID3D11InputLayout> m_layout;
	};
}

#endif __GK2_EFFECT_BASE_H_
