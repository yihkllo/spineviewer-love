#ifndef SPINELOVE_RENDER_D3D11_OVERLAY_DRAWER_H_
#define SPINELOVE_RENDER_D3D11_OVERLAY_DRAWER_H_

#include "../sl_overlay.h"

namespace sl_d3d11 {

class D3D11Renderer;

using OverlayStateRefreshFn = void(*)();

class D3D11OverlayDrawer final : public IOverlayDrawer
{
public:
	void SetTarget(D3D11Renderer* renderer, int width, int height) noexcept;
	void SetStateRefreshCallback(OverlayStateRefreshFn callback) noexcept;
	void DrawBox(const SlOverlayBox& box) override;
	void DrawReadSlotMeshOutline(const SlReadSlotMeshOutlineRequest& request) override;

private:
	bool BeginOverlay();
	void EndOverlay();

	D3D11Renderer* m_renderer = nullptr;
	OverlayStateRefreshFn m_refreshCallback = nullptr;
	int m_width = 0;
	int m_height = 0;
};

D3D11OverlayDrawer& GetOverlayDrawer();

}

#endif
