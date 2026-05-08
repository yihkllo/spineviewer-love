#ifndef SPINELOVE_RENDER_D3D11_CONTEXT_H_
#define SPINELOVE_RENDER_D3D11_CONTEXT_H_

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "../sl_gfx_types.h"

namespace sl_d3d11 {

class D3D11Context
{
public:
	bool Initialize(HWND hwnd, int width, int height);
	bool Resize(int width, int height);
	void BindBackBuffer();
	void BeginFrame(const SlVec4& clearColor);
	void EndFrame();
	void Shutdown() noexcept;

	ID3D11Device* Device() const noexcept { return m_device.Get(); }
	ID3D11DeviceContext* Context() const noexcept { return m_context.Get(); }
	int Width() const noexcept { return m_width; }
	int Height() const noexcept { return m_height; }

private:
	bool CreateRenderTarget();

	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTarget;
	int m_width = 0;
	int m_height = 0;
};

}

#endif
