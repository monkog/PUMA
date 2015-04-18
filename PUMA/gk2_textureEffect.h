#ifndef __GK2_TEXTURE_EFFECT_H_
#define __GK2_TEXTURE_EFFECT_H_

#include "gk2_effectBase.h"

namespace gk2
{
	class TextureEffect : public EffectBase
	{
	public:
		TextureEffect(gk2::DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
					  std::shared_ptr<ID3D11DeviceContext> context = nullptr);

		void SetTextureMtxBuffer(const std::shared_ptr<gk2::CBMatrix>& textureMtx);
		void SetSamplerState(const std::shared_ptr<ID3D11SamplerState>& samplerState);
		void SetTexture(const std::shared_ptr<ID3D11ShaderResourceView>& texture);

	protected:
		virtual void SetVertexShaderData();
		virtual void SetPixelShaderData();

	private:
		static const std::wstring ShaderFile;

		std::shared_ptr<gk2::CBMatrix> m_textureMtxCB;
		std::shared_ptr<ID3D11SamplerState> m_samplerState;
		std::shared_ptr<ID3D11ShaderResourceView> m_texture;
	};
}

#endif __GK2_TEXTURE_EFFECT_H_
