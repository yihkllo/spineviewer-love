#include "d3d11_context.h"

#include <dxgi.h>

namespace sl_d3d11 {

bool D3D11Context::Initialize(HWND hwnd, int width, int height)
{
	Shutdown();
	m_width = width;
	m_height = height;

	DXGI_SWAP_CHAIN_DESC desc{};
	desc.BufferCount = 2;
	desc.BufferDesc.Width = static_cast<UINT>(width);
	desc.BufferDesc.Height = static_cast<UINT>(height);
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow = hwnd;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT flags = 0;
#if defined(_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL actualLevel{};
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		featureLevels,
		static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0])),
		D3D11_SDK_VERSION,
		&desc,
		m_swapChain.GetAddressOf(),
		m_device.GetAddressOf(),
		&actualLevel,
		m_context.GetAddressOf());

#if defined(_DEBUG)
	if (FAILED(hr) && (flags & D3D11_CREATE_DEVICE_DEBUG))
	{
		flags &= ~D3D11_CREATE_DEVICE_DEBUG;
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
			featureLevels, static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0])),
			D3D11_SDK_VERSION, &desc, m_swapChain.GetAddressOf(), m_device.GetAddressOf(),
			&actualLevel, m_context.GetAddressOf());
	}
#endif

	return SUCCEEDED(hr) && CreateRenderTarget();
}

bool D3D11Context::CreateRenderTarget()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	if (FAILED(hr))
		return false;
	hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTarget.GetAddressOf());
	return SUCCEEDED(hr);
}

bool D3D11Context::Resize(int width, int height)
{
	if (!m_swapChain || width <= 0 || height <= 0)
		return false;
	if (width == m_width && height == m_height && m_renderTarget)
		return true;

	m_context->OMSetRenderTargets(0, nullptr, nullptr);
	m_renderTarget.Reset();

	HRESULT hr = m_swapChain->ResizeBuffers(0, static_cast<UINT>(width), static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr))
		return false;

	m_width = width;
	m_height = height;
	return CreateRenderTarget();
}

void D3D11Context::BindBackBuffer()
{
	if (!m_context || !m_renderTarget)
		return;

	ID3D11RenderTargetView* target = m_renderTarget.Get();
	m_context->OMSetRenderTargets(1, &target, nullptr);

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<FLOAT>(m_width);
	viewport.Height = static_cast<FLOAT>(m_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_context->RSSetViewports(1, &viewport);
}

void D3D11Context::BeginFrame(const SlVec4& clearColor)
{
	if (!m_context || !m_renderTarget)
		return;

	BindBackBuffer();
	const FLOAT color[] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	m_context->ClearRenderTargetView(m_renderTarget.Get(), color);
}

void D3D11Context::EndFrame()
{
	if (m_swapChain)
		m_swapChain->Present(1, 0);
}

void D3D11Context::Shutdown() noexcept
{
	m_renderTarget.Reset();
	m_swapChain.Reset();
	m_context.Reset();
	m_device.Reset();
	m_width = 0;
	m_height = 0;
}

}
