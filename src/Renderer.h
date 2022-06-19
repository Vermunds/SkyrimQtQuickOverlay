#pragma once
#include "Win.h"
#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")

class QGuiApplication;
class QWindow;
class QQuickRenderControl;
class QQuickWindow;
class QQmlEngine;
class QQmlComponent;
class QQuickItem;

namespace QSK
{
	class Renderer
	{
	public:
		static void InstallHook();
		static Renderer* GetSingleton();

		ID3D11Texture2D* GetTexture() { return m_texture; };
		ID3D11Device* GetDevice() { return m_device; };
		ID3D11DeviceContext* GetContext() { return m_context; };

		const std::uint32_t GetRenderWidth() { return m_renderWidth; };
		const std::uint32_t GetRenderHeight() { return m_renderHeight; };

	private:
		Renderer(){};
		~Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void Initialize(IDXGISwapChain1* a_swapChain);
		void Render();

		static std::uint32_t Present_Hook(void* a_oPresent, IDXGISwapChain1* a_this, std::uint32_t a_syncInterval, std::uint32_t a_flags);
		static void Main_Hook(void* a_unk);

		static void makeQuickDirty();

		std::uint32_t m_renderWidth;
		std::uint32_t m_renderHeight;

		IDXGISwapChain1* m_swapChain;

		ID3D11Device* m_device;
		ID3D11DeviceContext* m_context;

		ID3D11RenderTargetView* m_frameBufferView;

		ID3D11VertexShader* m_vertexShader;
		ID3D11PixelShader* m_pixelShader;

		ID3D11Buffer* m_vertexBuffer;
		ID3D11InputLayout* m_inputLayout;
		ID3D11Texture2D* m_texture;
		ID3D11ShaderResourceView* m_textureView;
		ID3D11BlendState* m_blendState;
		ID3D11SamplerState* m_samplerState;

		std::uint32_t m_numVerts;
		std::uint32_t m_stride;
		std::uint32_t m_offset;

		D3D11_VIEWPORT m_viewport;

		bool m_initialized = false;
	};
}
