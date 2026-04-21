#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#include "render_setup.h"

bool CDxLibContext::SetupBackend(void* pWindowHandle)
{
    if (DxLib::SetOutApplicationLogValidFlag(FALSE) == -1) return false;

#ifdef _WIN32
    HWND hWnd = static_cast<HWND>(pWindowHandle);
    if (hWnd)
    {
        if (DxLib::SetUserWindow(hWnd) == -1) return false;
        if (DxLib::SetUserWindowMessageProcessDXLibFlag(FALSE) == -1) return false;
    }
    if (DxLib::ChangeWindowMode(TRUE) == -1) return false;
    if (DxLib::SetMultiThreadFlag(TRUE) == -1) return false;
#endif
    if (DxLib::SetChangeScreenModeGraphicsSystemResetFlag(FALSE) == -1) return false;
    if (DxLib::SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8) == -1) return false;

    return true;
}

void CDxLibContext::ApplyDefaults()
{
    DxLib::SetDrawScreen(DX_SCREEN_BACK);
    DxLib::SetDrawMode(DX_DRAWMODE_BILINEAR);
    DxLib::SetTextureAddressMode(DX_TEXADDRESS_WRAP);
}

CDxLibContext::CDxLibContext(void* pWindowHandle)
{
    if (!SetupBackend(pWindowHandle)) return;
    if (DxLib::DxLib_Init() == -1) return;

    ApplyDefaults();
    m_bInitialised = true;
}

CDxLibContext::~CDxLibContext()
{
    if (m_bInitialised)
        DxLib::DxLib_End();
}
