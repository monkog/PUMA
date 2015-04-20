#ifndef __GK2_ROOM_H_
#define __GK2_ROOM_H_

#include "gk2_applicationBase.h"
#include "gk2_meshLoader.h"
#include "gk2_camera.h"
#include "gk2_phongEffect.h"
#include "gk2_textureEffect.h"
#include "gk2_constantBuffer.h"
#include "gk2_colorTexEffect.h"
#include "gk2_multiTexEffect.h"
#include "gk2_environmentMapper.h"
#include "gk2_particles.h"

namespace gk2
{
	class Scene : public gk2::ApplicationBase
	{
	public:
		Scene(HINSTANCE hInstance);
		virtual ~Scene();

		static void* operator new(size_t size);
		static void operator delete(void* ptr);

	protected:
		virtual bool LoadContent();
		virtual void UnloadContent();

		virtual void Update(float dt);
		virtual void Render();

	private:
		static const XMFLOAT4 LIGHT_POS[2];
		static const unsigned int BS_MASK;

		gk2::Mesh m_floor;
		gk2::Mesh m_metal;
		gk2::Mesh m_robot[6];

		XMMATRIX m_projMtx;

		gk2::Camera m_camera;
		gk2::MeshLoader m_meshLoader;

		std::shared_ptr<gk2::CBMatrix> m_worldCB;
		std::shared_ptr<gk2::CBMatrix> m_viewCB;
		std::shared_ptr<gk2::CBMatrix> m_projCB;
		std::shared_ptr<gk2::ConstantBuffer<XMFLOAT4, 2>> m_lightPosCB;
		std::shared_ptr<gk2::CBMatrix> m_textureCB;
		std::shared_ptr<gk2::ConstantBuffer<XMFLOAT4>> m_surfaceColorCB;
		std::shared_ptr<gk2::ConstantBuffer<XMFLOAT4>> m_cameraPosCB;

		std::shared_ptr<ID3D11SamplerState> m_samplerWrap;
		std::shared_ptr<ID3D11SamplerState> m_samplerBorder;
		std::shared_ptr<ID3D11ShaderResourceView> m_floorTexture;

		std::shared_ptr<gk2::PhongEffect> m_phongEffect;
		std::shared_ptr<gk2::TextureEffect> m_textureEffect;
		std::shared_ptr<gk2::ColorTexEffect> m_colorTexEffect;
		std::shared_ptr<gk2::MultiTexEffect> m_multiTexEffect;
		std::shared_ptr<gk2::EnvironmentMapper> m_environmentMapper;
		std::shared_ptr<gk2::ParticleSystem> m_particles;
		std::shared_ptr<ID3D11InputLayout> m_layout;

		std::shared_ptr<ID3D11RasterizerState> m_rsCullFront;
		std::shared_ptr<ID3D11BlendState> m_bsAlpha;
		std::shared_ptr<ID3D11DepthStencilState> m_dssNoWrite;

		void InitializeConstantBuffers();
		void InitializeTextures();
		void InitializeCamera();
		void InitializeRenderStates();
		void CreateScene();
		void UpdateCamera();

		void DrawScene();
		void DrawQuads();
		void DrawRobot();
		void DrawMetal();

		//void inverse_kinematics(vector3 pos, vector3 normal, float &a1, float &a2, float &a3, float &a4, float &a5);
	};
}

#endif __GK2_ROOM_H_