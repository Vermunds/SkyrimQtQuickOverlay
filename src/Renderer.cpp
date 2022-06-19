#include "Renderer.h"
#include "SkyrimQuickApplication.h"
#include "Util.h"

#include <xbyak/xbyak.h>

#include <codecvt>
#include <locale>
#include <string>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <QGuiApplication>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickGraphicsDevice>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickWindow>

namespace QSK
{
	Renderer::~Renderer()
	{
		m_device->Release();
		m_context->Release();
		m_frameBufferView->Release();
		m_vertexShader->Release();
		m_pixelShader->Release();
		m_vertexBuffer->Release();
		m_inputLayout->Release();
		m_texture->Release();
		m_textureView->Release();
		m_blendState->Release();
		m_samplerState->Release();
	}

	void Renderer::Initialize(IDXGISwapChain1* a_swapChain)
	{
		if (m_initialized)
		{
			return;
		}

		RE::INIPrefSettingCollection* prefs = RE::INIPrefSettingCollection::GetSingleton();
		m_renderWidth = prefs->GetSetting("iSize W:DISPLAY")->GetUInt();
		m_renderHeight = prefs->GetSetting("iSize H:DISPLAY")->GetUInt();

		m_swapChain = a_swapChain;
		HRESULT hr;

		// Get device and context
		hr = a_swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&m_device);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to get ID3D11Device from swap chain: " + GetComErrorString(hr));
		}
		m_device->GetImmediateContext(&m_context);

		// Create framebuffer render target
		ID3D11Texture2D* frameBuffer;
		hr = a_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to create framebuffer render target: " + GetComErrorString(hr));
		}

		hr = m_device->CreateRenderTargetView(frameBuffer, 0, &m_frameBufferView);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to create framebuffer render target view: " + GetComErrorString(hr));
		}
		frameBuffer->Release();

		// Create vertex shader
		ID3DBlob* vertexShaderBlob;
		ID3DBlob* vertexShaderCompileErrorsBlob;
		hr = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vertexShaderBlob, &vertexShaderCompileErrorsBlob);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Failed to compile vertex shader: " + GetComErrorString(hr));
		}
		hr = m_device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &m_vertexShader);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to create vertex shader: " + GetComErrorString(hr));
		}

		// Create pixel shader
		ID3DBlob* pixelShaderBlob;
		ID3DBlob* pixelShaderCompileErrorsBlob;
		hr = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &pixelShaderBlob, &pixelShaderCompileErrorsBlob);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Failed to compile pixel shader: " + GetComErrorString(hr));
		}

		hr = m_device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &m_pixelShader);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to create pixel shader: " + GetComErrorString(hr));
		}
		pixelShaderBlob->Release();

		// Create input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[]{
			{ "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		hr = m_device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &m_inputLayout);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to create pixel shader: " + GetComErrorString(hr));
		}
		vertexShaderBlob->Release();

		// Create vertex buffer
		float vertexData[] = {
			-1.0f, 1.0f, 0.f, 0.f,
			1.0f, -1.0f, 1.f, 1.f,
			-1.0f, -1.0f, 0.f, 1.f,
			-1.0f, 1.0f, 0.f, 0.f,
			1.0f, 1.0f, 1.f, 0.f,
			1.0f, -1.0f, 1.f, 1.f
		};
		m_stride = 4 * sizeof(float);
		m_numVerts = sizeof(vertexData) / m_stride;
		m_offset = 0;

		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.ByteWidth = sizeof(vertexData);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubresourceData = { vertexData };

		hr = m_device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &m_vertexBuffer);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Failed to create vertex buffer: " + GetComErrorString(hr));
		}

		// Create texture
		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = m_renderWidth;
		textureDesc.Height = m_renderHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_texture);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to create texture: " + GetComErrorString(hr));
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
		shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		hr = m_device->CreateShaderResourceView(m_texture, &shaderResourceViewDesc, &m_textureView);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to create shader resource view: " + GetComErrorString(hr));
		}

		// Create sampler state
		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MaxAnisotropy = 1;

		hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerState);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to create sampler state: " + GetComErrorString(hr));
		}

		// Create blend state
		D3D11_BLEND_DESC blendStateDesc{};
		ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
		blendStateDesc.AlphaToCoverageEnable = FALSE;
		blendStateDesc.IndependentBlendEnable = FALSE;
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		hr = m_device->CreateBlendState(&blendStateDesc, &m_blendState);
		if (FAILED(hr))
		{
			SKSE::stl::report_and_fail("Unable to create blend state: " + GetComErrorString(hr));
		}

		// Calculate viewport
		RECT winRect;
		GetClientRect((HWND)RE::Main::GetSingleton()->wnd, &winRect);
		m_viewport = { 0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f };
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		// Initialization completed
		m_initialized = true;
	}

	void Renderer::Render()
	{
		SkyrimQuickApplication* app = SkyrimQuickApplication::GetSingleton();
		app->UpdateAndRender();

		m_context->OMSetBlendState(m_blendState, NULL, 0xFFFFFF);

		m_context->RSSetViewports(1, &m_viewport);

		m_context->OMSetRenderTargets(1, &m_frameBufferView, nullptr);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout);

		m_context->VSSetShader(m_vertexShader, nullptr, 0);
		m_context->PSSetShader(m_pixelShader, nullptr, 0);

		m_context->PSSetShaderResources(0, 1, &m_textureView);
		m_context->PSSetSamplers(0, 1, &m_samplerState);

		m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_stride, &m_offset);

		m_context->Draw(m_numVerts, 0);
	}

	std::uint32_t Renderer::Present_Hook(void* a_oPresent, IDXGISwapChain1* a_this, std::uint32_t a_syncInterval, std::uint32_t a_flags)
	{
		static Renderer* renderer = Renderer::GetSingleton();
		SkyrimQuickApplication* app = SkyrimQuickApplication::GetSingleton();

		if (!renderer->m_initialized)
		{
			renderer->Initialize(a_this);
		}

		if (app->isInitialized())
		{
			renderer->Render();
		}

		return reinterpret_cast<HRESULT (*)(IDXGISwapChain*, std::uint32_t, std::uint32_t)>(a_oPresent)(a_this, a_syncInterval, a_flags);
	}
	void Renderer::Main_Hook(void* a_unk)
	{
		Renderer* renderer = Renderer::GetSingleton();
		SkyrimQuickApplication* app = SkyrimQuickApplication::GetSingleton();

		if (renderer->m_initialized && !app->isInitialized())
		{
			app->Initialize();
		}
		app->ProcessAllEvents();

		return reinterpret_cast<void (*)(void*)>(REL::ID{ static_cast<std::uint64_t>(36564) }.address())(a_unk);
	}

	void QSK::Renderer::makeQuickDirty()
	{
		SkyrimQuickApplication::GetSingleton()->SetDirty();
	}

	void Renderer::InstallHook()
	{
		REL::ID hook{ static_cast<std::uint64_t>(77246) };

		struct Hook_Code : Xbyak::CodeGenerator
		{
			Hook_Code(std::uintptr_t a_hookAddr, std::uintptr_t a_retAddress)
			{
				Xbyak::Label hookAddress;
				Xbyak::Label retAddress;

				mov(edx, dword[rax + 0x30]);

				push(r9);
				mov(r9, r8);
				mov(r8, rdx);
				mov(rdx, rcx);
				pop(rcx);
				mov(rcx, qword[rcx + 0x40]);

				call(ptr[rip + hookAddress]);

				//call(qword[r9 + 0x40]);
				jmp(ptr[rip + retAddress]);

				L(hookAddress);
				dq(a_hookAddr);

				L(retAddress);
				dq(a_retAddress);
			}
		};

		Hook_Code code{ std::uintptr_t(Present_Hook), hook.address() + 0xA6 };
		void* codeLoc = SKSE::GetTrampoline().allocate(code);

		SKSE::GetTrampoline().write_branch<5>(hook.address() + 0x9F, codeLoc);

		std::uint8_t buf[] = { 0x90, 0x90 };  // nop x2
		REL::safe_write(hook.address() + 0xA4, std::span<uint8_t>(buf));

		SKSE::GetTrampoline().write_call<5>(REL::ID{ static_cast<std::uint64_t>(36544) }.address() + 0x160, (uintptr_t)Renderer::Main_Hook);
	}

	Renderer* Renderer::GetSingleton()
	{
		static Renderer renderer;
		return &renderer;
	}
}
