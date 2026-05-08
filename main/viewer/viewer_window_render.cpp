#include "viewer_window_common.h"

#include <CommCtrl.h>
#include <filesystem>
#include <iomanip>
#include <shellapi.h>
#include <sstream>

using namespace viewer_window_detail;

int DeleteTextureHandle(int handle) noexcept
{
	(void)handle;
	return 0;
}

sl_d3d11::D3D11Renderer* g_titleBarTextureRenderer = nullptr;

void* D3D11TitleBarTextureToSrv(int textureHandle)
{
	if (g_titleBarTextureRenderer == nullptr || textureHandle <= 0)
		return nullptr;
	return g_titleBarTextureRenderer->GetTextureSrv(static_cast<SlTextureId>(textureHandle));
}

bool SpineLoveWindow::PrepareD3D11SpineRenderer()
{
	if (m_d3d11SpineRendererReady)
		return true;

	ID3D11Device* device = m_d3d11Context.Device();
	ID3D11DeviceContext* context = m_d3d11Context.Context();
	if (device == nullptr || context == nullptr)
		return false;

	const std::wstring shaderPath = GetExecutableDirectory() + L"\\render_d3d11\\shaders\\sprite.hlsl";
	m_d3d11SpineRendererReady = m_d3d11SpineRenderer.Initialize(device, context, shaderPath.c_str());
	if (m_d3d11SpineRendererReady)
	{
		g_titleBarTextureRenderer = &m_d3d11SpineRenderer;
	}
	return m_d3d11SpineRendererReady;
}

bool SpineLoveWindow::StartD3D11Runtime()
{
	RECT rc{};
	::GetClientRect(m_windowHandle, &rc);
	const int width = rc.right - rc.left;
	const int height = rc.bottom - rc.top;
	if (width <= 0 || height <= 0)
		return false;

	m_pendingClientWidth = width;
	m_pendingClientHeight = height;
	m_hasPendingResize = false;
	if (!m_d3d11Context.Initialize(m_windowHandle, width, height))
		return false;
	return PrepareD3D11SpineRenderer();
}

ID3D11Device* SpineLoveWindow::NativeD3DDevice() const noexcept
{
	return m_d3d11Context.Device();
}

ID3D11DeviceContext* SpineLoveWindow::NativeD3DContext() const noexcept
{
	return m_d3d11Context.Context();
}

bool SpineLoveWindow::PrepareSpineSurface(int width, int height)
{
	if (width <= 0 || height <= 0)
		return false;
	if (!PrepareD3D11SpineRenderer())
		return false;
	if (m_d3d11SpineRenderTarget != 0 &&
		m_d3d11SpineRenderTargetWidth == width &&
		m_d3d11SpineRenderTargetHeight == height)
	{
		return true;
	}

	m_d3d11SpineRenderTarget = m_d3d11SpineRenderer.CreateRenderTarget(width, height);
	if (m_d3d11SpineRenderTarget == 0)
	{
		m_d3d11SpineRenderTargetWidth = 0;
		m_d3d11SpineRenderTargetHeight = 0;
		return false;
	}

	m_d3d11SpineRenderTargetWidth = width;
	m_d3d11SpineRenderTargetHeight = height;
	return true;
}

bool SpineLoveWindow::QuerySpineSurfaceSize(int& outWidth, int& outHeight) const noexcept
{
	outWidth = 0;
	outHeight = 0;
	if (m_d3d11SpineRenderTarget != 0)
	{
		outWidth = m_d3d11SpineRenderTargetWidth;
		outHeight = m_d3d11SpineRenderTargetHeight;
		return outWidth > 0 && outHeight > 0;
	}

	const int panelX = static_cast<int>(m_panelState.leftPanelEndX);
	outWidth = m_pendingClientWidth - panelX;
	outHeight = m_pendingClientHeight;
	return outWidth > 0 && outHeight > 0;
}

void SpineLoveWindow::StartRenderingStack()
{
	RECT rc;
	::GetClientRect(m_windowHandle, &rc);
	m_pendingClientWidth = rc.right - rc.left;
	m_pendingClientHeight = rc.bottom - rc.top;
	m_hasPendingResize = true;
	CommitPendingResize();
}

void SpineLoveWindow::CommitPendingResize()
{
	if (!m_hasPendingResize) return;
	m_hasPendingResize = false;

	int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

	int iGraphWidth = m_pendingClientWidth < iDesktopWidth ? m_pendingClientWidth : iDesktopWidth;
	int iGraphHeight = m_pendingClientHeight < iDesktopHeight ? m_pendingClientHeight : iDesktopHeight;
	if (iGraphWidth <= 0 || iGraphHeight <= 0) return;

	m_d3d11Context.Resize(iGraphWidth, iGraphHeight);
	m_renderTextureHandle.Reset(-1);
	m_d3d11SpineRenderTarget = 0;
	m_d3d11SpineRenderTargetWidth = 0;
	m_d3d11SpineRenderTargetHeight = 0;
	::InvalidateRect(m_windowHandle, nullptr, FALSE);

	
	
	DragAcceptFiles(m_windowHandle, TRUE);
	ChangeWindowMessageFilterEx(m_windowHandle, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(m_windowHandle, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(m_windowHandle, 0x0049, MSGFLT_ALLOW, nullptr);
	::EnumChildWindows(m_windowHandle, [](HWND hwndChild, LPARAM lParam) -> BOOL {
		DragAcceptFiles(hwndChild, TRUE);
		ChangeWindowMessageFilterEx(hwndChild, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
		ChangeWindowMessageFilterEx(hwndChild, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
		ChangeWindowMessageFilterEx(hwndChild, 0x0049, MSGFLT_ALLOW, nullptr);
		return TRUE;
	}, 0);
}

void SpineLoveWindow::RunFrame()
{
	if (m_isModalOpen) return;
	if (m_isLoading) return;
	if (m_uiCore == nullptr) return;


	if (m_hasPendingResize)
	{
		CommitPendingResize();
	}


	if (!m_hasDragDropEnabled)
	{
		m_hasDragDropEnabled = true;
		DragAcceptFiles(m_windowHandle, TRUE);


		if (m_panelState.titleBar.hIconTexture == -1)
		{
			wchar_t exePath[MAX_PATH]{};
			::GetModuleFileNameW(nullptr, exePath, MAX_PATH);
			wchar_t* pSlash = ::wcsrchr(exePath, L'\\');
			if (pSlash) { *(pSlash + 1) = L'\0'; }


			{
				int len = ::WideCharToMultiByte(CP_UTF8, 0, exePath, -1, nullptr, 0, nullptr, nullptr);
				std::string exeDir(len > 0 ? len - 1 : 0, '\0');
				if (len > 0) ::WideCharToMultiByte(CP_UTF8, 0, exePath, -1, &exeDir[0], len, nullptr, nullptr);
				i18n::init(exeDir);
			}

			::wcscat_s(exePath, L"app.png");
			if (PrepareD3D11SpineRenderer())
			{
				SlTextureId icon = m_d3d11SpineRenderer.LoadTexture(exePath, false);
				if (icon != 0) m_panelState.titleBar.hIconTexture = static_cast<int>(icon);
			}
		}

		if (m_panelState.titleBar.customFont == nullptr)
		{
			const float sc = custom_titlebar::GetScale();
			ImGuiIO& io = ImGui::GetIO();

			wchar_t winDir[MAX_PATH]{};
			::GetWindowsDirectoryW(winDir, MAX_PATH);
			std::wstring fontsDir = winDir;
			if (!fontsDir.empty() && fontsDir.back() != L'\\') fontsDir += L'\\';
			fontsDir += L"Fonts\\";
			const std::wstring bundledFontPath = path_util::GetBundledFontPath();
			m_panelState.titleBar.customFont = io.Fonts->AddFontFromFileTTF(
				sl_text::WideToUtf8(fontsDir + L"segoesc.ttf").c_str(), 40.0f * sc);
			const ImWchar* glyph = io.Fonts->GetGlyphRangesChineseFull();
			const std::wstring subTitleFontPath = bundledFontPath.empty() ? (fontsDir + L"malgun.ttf") : bundledFontPath;
			m_panelState.titleBar.subTitleFont = io.Fonts->AddFontFromFileTTF(
				sl_text::WideToUtf8(subTitleFontPath).c_str(), 26.7f * sc, nullptr, glyph);
			io.Fonts->Build();
		}

		ChangeWindowMessageFilterEx(m_windowHandle, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
		ChangeWindowMessageFilterEx(m_windowHandle, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
		ChangeWindowMessageFilterEx(m_windowHandle, 0x0049  , MSGFLT_ALLOW, nullptr);
		::EnumChildWindows(m_windowHandle, [](HWND hwndChild, LPARAM lParam) -> BOOL {
			DragAcceptFiles(hwndChild, TRUE);
			ChangeWindowMessageFilterEx(hwndChild, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
			ChangeWindowMessageFilterEx(hwndChild, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
			ChangeWindowMessageFilterEx(hwndChild, 0x0049, MSGFLT_ALLOW, nullptr);
			return TRUE;
		}, 0);
	}

	HandlePendingExportRequest();
	const bool exportWasActive = m_exportJob.kind != ExportJobKind::None;
	if (exportWasActive)
		AdvanceExportJob();

	if (!m_uiCore->BeginFrame())
		return;


		const int panelX = static_cast<int>(m_panelState.leftPanelEndX);
		const int playW = m_pendingClientWidth - panelX;
		const int playH = m_pendingClientHeight;

		if (PrepareD3D11SpineRenderer())
		{
			m_d3d11Context.BeginFrame(m_d3d11ClearColor);
		}

		if (playW > 0 && playH > 0)
		{
			if (HasBackgroundTexture())
			{
				int bgW = 0, bgH = 0;
				GetBackgroundTextureSize(bgW, bgH);
				const float drawW = bgW * m_bgScale;
				const float drawH = bgH * m_bgScale;
				if (m_d3d11BackgroundTexture != 0 && PrepareD3D11SpineRenderer())
				{
					m_d3d11SpineRenderer.BeginFrame(m_pendingClientWidth, m_pendingClientHeight);
					m_d3d11SpineRenderer.DrawSprite(
						m_d3d11BackgroundTexture,
						SlRect(static_cast<float>(panelX) + m_bgOffsetX, m_bgOffsetY, drawW, drawH),
						SlRect(0.0f, 0.0f, 1.0f, 1.0f),
						SlColor(1.0f, 1.0f, 1.0f, 1.0f));
					m_d3d11SpineRenderer.EndFrame();
				}
			}

			if (m_runtimeHub.CurrentRuntime()->ContainsDrawableContent() &&
				PrepareSpineSurface(playW, playH))
			{
				m_runtimeHub.CurrentRuntime()->SetViewportSize(playW, playH);
				if (m_d3d11SpineRenderer.BeginRenderTarget(m_d3d11SpineRenderTarget, SlVec4(0.0f, 0.0f, 0.0f, 0.0f)))
				{
					m_runtimeHub.RenderCurrentRuntimeD3D11(m_d3d11SpineRenderer);
					m_d3d11SpineRenderer.EndFrame();
					m_d3d11Context.BindBackBuffer();
					m_d3d11SpineRenderer.BeginFrame(m_pendingClientWidth, m_pendingClientHeight);
					DrawD3D11SpritePma(
						m_d3d11SpineRenderer,
						m_d3d11SpineRenderTarget,
						SlRect(static_cast<float>(panelX), 0.0f, static_cast<float>(playW), static_cast<float>(playH)));
					m_d3d11SpineRenderer.EndFrame();
				}
			}
		}

		if (m_runtimeHub.CurrentRuntime()->ContainsDrawableContent())
		{
			if (!exportWasActive && m_exportJob.kind == ExportJobKind::None)
			{
				m_runtimeHub.CurrentRuntime()->TickPlayback(m_frameClock.ElapsedSeconds());
				StepQueuePlay();
			}
			m_frameClock.Reset();
		}

		sl_d3d11::GetOverlayDrawer().SetTarget(
			m_d3d11SpineRendererReady ? &m_d3d11SpineRenderer : nullptr,
			m_pendingClientWidth,
			m_pendingClientHeight);
		DrawSpineControlPanel();

		m_uiCore->SubmitFrame();

		m_d3d11Context.EndFrame();
}

void SpineLoveWindow::DrawSpineControlPanel()
{
	if (m_d3d11SpineRenderTarget != 0)
	{
		m_panelState.iTextureWidth = m_d3d11SpineRenderTargetWidth;
		m_panelState.iTextureHeight = m_d3d11SpineRenderTargetHeight;

		if (!m_panelState.isFullscreen)
		{
			m_panelState.iTextureHeight -= static_cast<int>(custom_titlebar::TitleBarHeight());
			if (m_panelState.iTextureHeight < 0) m_panelState.iTextureHeight = 0;
		}
	}

	spine_panel::DrawControlPanel(m_panelState, &m_showControlPanel);
	if (m_panelState.requestContentResize)
	{
		ResizeToContentBounds();
		m_panelState.requestContentResize = false;
	}
}

void SpineLoveWindow::BrowseBackgroundImage()
{
	SModalGuard guard(m_isModalOpen);
	shell_dialogs::FileDialogRequest backgroundRequest{};
	backgroundRequest.title = L"Select background image";
	backgroundRequest.primaryFilter = { L"Image file", L"*.png;*.jpg;*.jpeg" };
	std::wstring path = m_fileDialogs.PickSingleFile(backgroundRequest, m_windowHandle);
	if (path.empty()) return;

	if (!PrepareD3D11SpineRenderer()) return;
	SlTextureId texture = m_d3d11SpineRenderer.LoadTexture(path.c_str(), false);
	if (texture == 0) return;
	if (m_d3d11BackgroundTexture != 0)
		m_d3d11SpineRenderer.ReleaseTexture(m_d3d11BackgroundTexture);
	m_d3d11BackgroundTexture = texture;
	m_d3d11SpineRenderer.GetTextureSize(texture, m_d3d11BackgroundWidth, m_d3d11BackgroundHeight);

	const int panelX = static_cast<int>(m_panelState.leftPanelEndX);
	const int playW = m_pendingClientWidth - panelX;
	m_bgScale = 1.f;
	m_bgOffsetX = (playW - m_d3d11BackgroundWidth) * 0.5f;
	m_bgOffsetY = (m_pendingClientHeight - m_d3d11BackgroundHeight) * 0.5f;
}

bool SpineLoveWindow::HasBackgroundTexture() const noexcept
{
	return m_d3d11BackgroundTexture != 0;
}

void SpineLoveWindow::GetBackgroundTextureSize(int& outWidth, int& outHeight) const
{
	outWidth = m_d3d11BackgroundWidth;
	outHeight = m_d3d11BackgroundHeight;
}

void SpineLoveWindow::HandlePendingExportRequest()
{
	if (m_pendingExport.kind == ExportRequestKind::None)
		return;

	PendingExportRequest request = m_pendingExport;
	m_pendingExport = PendingExportRequest{};
	StartExportRequest(request);
}

void SpineLoveWindow::StartExportRequest(const PendingExportRequest& request)
{
	switch (request.kind)
	{
	case ExportRequestKind::Snapshot:
		ExportCurrentView(request.imageFormat, request.keepAlpha);
		break;
	case ExportRequestKind::FrameSequence:
		ExportFrameSequence(request.imageFormat, request.keepAlpha, request.useAnimationQueue);
		break;
	case ExportRequestKind::Movie:
		ExportMovie(request.movieFormat, request.keepAlpha, request.useAnimationQueue);
		break;
	default:
		break;
	}
}

namespace
{
	int ClampExportFps(int fps) noexcept
	{
		return (std::max)(1, (std::min)(120, fps));
	}

	std::vector<std::string> SelectExportMotions(SlPlaybackRuntime* runtime, const std::vector<std::string>& queue, bool useQueue)
	{
		std::vector<std::string> motions;
		if (useQueue)
		{
			for (const std::string& name : queue)
			{
				if (!name.empty())
					motions.push_back(name);
			}
			return motions;
		}

		if (runtime == nullptr)
			return motions;
		std::string active = runtime->ActiveMotionName();
		if (active.empty() && !runtime->MotionNames().empty())
			active = runtime->MotionNames().front();
		if (!active.empty())
			motions.push_back(active);
		return motions;
	}

	int BuildMotionFrameCounts(SlPlaybackRuntime* runtime, const std::vector<std::string>& motions, int fps, std::vector<int>& counts)
	{
		counts.clear();
		int total = 0;
		for (const std::string& motion : motions)
		{
			float duration = runtime != nullptr && !motion.empty() ? runtime->MotionDuration(motion.c_str()) : 0.0f;
			if (!std::isfinite(duration) || duration < 0.0f)
				duration = 0.0f;
			const int frames = duration > 0.0f
				? (std::max)(1, static_cast<int>(std::ceil(duration * static_cast<float>(fps))))
				: 1;
			counts.push_back(frames);
			total += frames;
		}
		return total;
	}

	const wchar_t* ImageExtension(SlExportImageFormat format) noexcept
	{
		return format == SlExportImageFormat::Png ? L".png" : L".jpg";
	}

	const wchar_t* MovieExtension(SlExportMovieFormat format) noexcept
	{
		switch (format)
		{
		case SlExportMovieFormat::Mp4: return L".mp4";
		case SlExportMovieFormat::Gif: return L".gif";
		case SlExportMovieFormat::Webm:
		default: return L".webm";
		}
	}

	const wchar_t* MovieExportDialogTitle(SlExportMovieFormat format) noexcept
	{
		switch (format)
		{
		case SlExportMovieFormat::Mp4: return L"MP4 Export";
		case SlExportMovieFormat::Gif: return L"GIF Export";
		case SlExportMovieFormat::Webm:
		default: return L"WebM Export";
		}
	}

	int DialogScale(HWND owner, int value)
	{
		(void)owner;
		return value;
	}

	HFONT CreateDialogFont(HWND owner, bool underline, bool bold)
	{
		NONCLIENTMETRICSW metrics{};
		metrics.cbSize = sizeof(metrics);
		if (!::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0))
			return reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));

		LOGFONTW font = metrics.lfMessageFont;
		font.lfHeight = -24;
		font.lfUnderline = underline ? TRUE : FALSE;
		font.lfWeight = bold ? FW_SEMIBOLD : FW_NORMAL;
		return ::CreateFontIndirectW(&font);
	}

	struct FfmpegMissingDialogState
	{
		HWND window = nullptr;
		HWND owner = nullptr;
		HWND mainText = nullptr;
		HWND linkLine1 = nullptr;
		HWND linkLine2 = nullptr;
		HWND bottomPanel = nullptr;
		HFONT textFont = nullptr;
		HFONT mainFont = nullptr;
		HFONT linkFont = nullptr;
		HBRUSH background = nullptr;
		HBRUSH bottomBackground = nullptr;
		bool running = true;
	};

	LRESULT CALLBACK FfmpegMissingDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto* state = reinterpret_cast<FfmpegMissingDialogState*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (message == WM_NCCREATE)
		{
			auto* create = reinterpret_cast<LPCREATESTRUCTW>(lParam);
			state = static_cast<FfmpegMissingDialogState*>(create->lpCreateParams);
			::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
			state->window = hwnd;
		}

		switch (message)
		{
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK)
			{
				::DestroyWindow(hwnd);
				return 0;
			}
			const HWND sender = reinterpret_cast<HWND>(lParam);
			if (state != nullptr &&
				(sender == state->linkLine1 || sender == state->linkLine2) &&
				HIWORD(wParam) == STN_CLICKED)
			{
				::ShellExecuteW(nullptr, L"open", L"https://github.com/BtbN/FFmpeg-Builds/releases", nullptr, nullptr, SW_SHOWNORMAL);
				return 0;
			}
			break;
		}
		case WM_CTLCOLORSTATIC:
			if (state != nullptr && reinterpret_cast<HWND>(lParam) == state->bottomPanel)
			{
				::SetBkMode(reinterpret_cast<HDC>(wParam), OPAQUE);
				return reinterpret_cast<LRESULT>(state->bottomBackground);
			}
			if (state != nullptr && reinterpret_cast<HWND>(lParam) == state->mainText)
			{
				::SetTextColor(reinterpret_cast<HDC>(wParam), RGB(0, 61, 180));
				::SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
				return reinterpret_cast<LRESULT>(state->background);
			}
			if (state != nullptr &&
				(reinterpret_cast<HWND>(lParam) == state->linkLine1 ||
					reinterpret_cast<HWND>(lParam) == state->linkLine2))
			{
				::SetTextColor(reinterpret_cast<HDC>(wParam), RGB(0, 92, 190));
				::SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
				return reinterpret_cast<LRESULT>(state->background);
			}
			if (state != nullptr)
			{
				::SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
				return reinterpret_cast<LRESULT>(state->background);
			}
			break;
		case WM_SETCURSOR:
			if (state != nullptr &&
				(reinterpret_cast<HWND>(wParam) == state->linkLine1 ||
					reinterpret_cast<HWND>(wParam) == state->linkLine2))
			{
				::SetCursor(::LoadCursorW(nullptr, IDC_HAND));
				return TRUE;
			}
			break;
		case WM_CLOSE:
			::DestroyWindow(hwnd);
			return 0;
		case WM_DESTROY:
			if (state != nullptr)
				state->running = false;
			return 0;
		}
		return ::DefWindowProcW(hwnd, message, wParam, lParam);
	}

	void ShowClickableFfmpegFallback(HWND owner, const wchar_t* title)
	{
		const wchar_t* className = L"SpineLoveFfmpegMissingDialog";
		static bool registered = false;
		if (!registered)
		{
			WNDCLASSW wc{};
			wc.lpfnWndProc = FfmpegMissingDialogProc;
			wc.hInstance = ::GetModuleHandleW(nullptr);
			wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
			wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
			wc.lpszClassName = className;
			registered = ::RegisterClassW(&wc) != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
		}

		FfmpegMissingDialogState state{};
		state.owner = owner;
		state.textFont = CreateDialogFont(owner, false, false);
		state.mainFont = CreateDialogFont(owner, false, true);
		state.linkFont = CreateDialogFont(owner, true, false);
		state.background = ::CreateSolidBrush(RGB(255, 255, 255));
		state.bottomBackground = ::CreateSolidBrush(RGB(240, 240, 240));

		const int width = DialogScale(owner, 690);
		const int height = DialogScale(owner, 412);
		RECT ownerRect{};
		if (owner != nullptr)
			::GetWindowRect(owner, &ownerRect);
		else
		{
			ownerRect.left = 0;
			ownerRect.top = 0;
			ownerRect.right = ::GetSystemMetrics(SM_CXSCREEN);
			ownerRect.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		}
		const int x = ownerRect.left + ((ownerRect.right - ownerRect.left) - width) / 2;
		const int y = ownerRect.top + ((ownerRect.bottom - ownerRect.top) - height) / 2;

		HWND dialog = ::CreateWindowExW(
			WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE,
			className,
			title,
			WS_CAPTION | WS_SYSMENU | WS_POPUP,
			x,
			y,
			width,
			height,
			owner,
			nullptr,
			::GetModuleHandleW(nullptr),
			&state);
		if (dialog == nullptr)
		{
			::MessageBoxW(owner,
				L"ffmpeg.exe not found.\n\nPlease place ffmpeg.exe in the same folder as the application.\n\nDownload: https://github.com/BtbN/FFmpeg-Builds/releases",
				title,
				MB_OK | MB_ICONWARNING);
			if (state.background) ::DeleteObject(state.background);
			if (state.bottomBackground) ::DeleteObject(state.bottomBackground);
			if (state.textFont && state.textFont != reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT))) ::DeleteObject(state.textFont);
			if (state.mainFont && state.mainFont != reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT))) ::DeleteObject(state.mainFont);
			if (state.linkFont && state.linkFont != reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT))) ::DeleteObject(state.linkFont);
			return;
		}

		state.bottomPanel = ::CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE,
			DialogScale(owner, 0), DialogScale(owner, 328), width, DialogScale(owner, 84),
			dialog, nullptr, nullptr, nullptr);

		HWND icon = ::CreateWindowW(L"STATIC", nullptr, WS_CHILD | WS_VISIBLE | SS_ICON,
			DialogScale(owner, 22), DialogScale(owner, 76), DialogScale(owner, 64), DialogScale(owner, 64),
			dialog, nullptr, nullptr, nullptr);
		::SendMessageW(icon, STM_SETICON, reinterpret_cast<WPARAM>(::LoadIconW(nullptr, IDI_WARNING)), 0);

		state.mainText = ::CreateWindowW(L"STATIC", L"ffmpeg.exe not found.", WS_CHILD | WS_VISIBLE,
			DialogScale(owner, 102), DialogScale(owner, 72), DialogScale(owner, 540), DialogScale(owner, 38),
			dialog, nullptr, nullptr, nullptr);
		HWND line2 = ::CreateWindowW(L"STATIC", L"Please place ffmpeg.exe in the same folder as the application.", WS_CHILD | WS_VISIBLE,
			DialogScale(owner, 102), DialogScale(owner, 147), DialogScale(owner, 545), DialogScale(owner, 62),
			dialog, nullptr, nullptr, nullptr);
		HWND prefix = ::CreateWindowW(L"STATIC", L"Download:", WS_CHILD | WS_VISIBLE,
			DialogScale(owner, 102), DialogScale(owner, 243), DialogScale(owner, 125), DialogScale(owner, 34),
			dialog, nullptr, nullptr, nullptr);
		state.linkLine1 = ::CreateWindowW(L"STATIC", L"github.com/BtbN/FFmpeg-Builds/", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
			DialogScale(owner, 230), DialogScale(owner, 243), DialogScale(owner, 380), DialogScale(owner, 34),
			dialog, reinterpret_cast<HMENU>(1001), nullptr, nullptr);
		state.linkLine2 = ::CreateWindowW(L"STATIC", L"releases", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
			DialogScale(owner, 102), DialogScale(owner, 277), DialogScale(owner, 140), DialogScale(owner, 34),
			dialog, reinterpret_cast<HMENU>(1002), nullptr, nullptr);
		HWND ok = ::CreateWindowW(L"BUTTON", L"确定", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			DialogScale(owner, 556), DialogScale(owner, 348), DialogScale(owner, 128), DialogScale(owner, 52),
			dialog, reinterpret_cast<HMENU>(IDOK), nullptr, nullptr);
		::SetWindowTextW(ok, L"\x786E\x5B9A");

		for (HWND control : { line2, prefix, ok })
			::SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(state.textFont), TRUE);
		::SendMessageW(state.mainText, WM_SETFONT, reinterpret_cast<WPARAM>(state.mainFont), TRUE);
		::SendMessageW(state.linkLine1, WM_SETFONT, reinterpret_cast<WPARAM>(state.linkFont), TRUE);
		::SendMessageW(state.linkLine2, WM_SETFONT, reinterpret_cast<WPARAM>(state.linkFont), TRUE);

		::EnableWindow(owner, FALSE);
		::ShowWindow(dialog, SW_SHOW);
		::UpdateWindow(dialog);

		MSG msg{};
		while (state.running && ::GetMessageW(&msg, nullptr, 0, 0) > 0)
		{
			if (!::IsDialogMessageW(dialog, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
		}

		::EnableWindow(owner, TRUE);
		::SetActiveWindow(owner);
		if (state.background) ::DeleteObject(state.background);
		if (state.bottomBackground) ::DeleteObject(state.bottomBackground);
		if (state.textFont && state.textFont != reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT))) ::DeleteObject(state.textFont);
		if (state.mainFont && state.mainFont != reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT))) ::DeleteObject(state.mainFont);
		if (state.linkFont && state.linkFont != reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT))) ::DeleteObject(state.linkFont);
	}

	void ShowFfmpegMissingDialog(HWND owner, const wchar_t* title)
	{
		TASKDIALOGCONFIG dialog{};
		dialog.cbSize = sizeof(dialog);
		dialog.hwndParent = owner;
		dialog.dwFlags = TDF_ENABLE_HYPERLINKS;
		dialog.dwCommonButtons = TDCBF_OK_BUTTON;
		dialog.pszWindowTitle = title;
		dialog.pszMainIcon = TD_WARNING_ICON;
		dialog.pszMainInstruction = L"ffmpeg.exe not found.";
		dialog.pszContent =
			L"Please place ffmpeg.exe in the same folder as the application.\r\n\r\n"
			L"Download: <a href=\"https://github.com/BtbN/FFmpeg-Builds/releases\">github.com/BtbN/FFmpeg-Builds/</a>\r\n"
			L"<a href=\"https://github.com/BtbN/FFmpeg-Builds/releases\">releases</a>";
		dialog.pfCallback = [](HWND, UINT notification, WPARAM, LPARAM payload, LONG_PTR) -> HRESULT {
			if (notification == TDN_HYPERLINK_CLICKED)
				::ShellExecuteW(nullptr, L"open", reinterpret_cast<LPCWSTR>(payload), nullptr, nullptr, SW_SHOWNORMAL);
			return S_OK;
		};

		using TaskDialogIndirectProc = HRESULT(WINAPI*)(const TASKDIALOGCONFIG*, int*, int*, BOOL*);
		HMODULE controls = ::LoadLibraryW(L"comctl32.dll");
		TaskDialogIndirectProc taskDialog = controls != nullptr
			? reinterpret_cast<TaskDialogIndirectProc>(::GetProcAddress(controls, "TaskDialogIndirect"))
			: nullptr;
		const bool taskDialogShown = taskDialog != nullptr &&
			SUCCEEDED(taskDialog(&dialog, nullptr, nullptr, nullptr));
		if (controls != nullptr)
			::FreeLibrary(controls);

		if (!taskDialogShown)
		{
			ShowClickableFfmpegFallback(owner, title);
		}
	}

	std::wstring QuoteProcessArg(const std::wstring& text)
	{
		std::wstring quoted;
		quoted.reserve(text.size() + 2);
		quoted.push_back(L'"');
		for (wchar_t ch : text)
		{
			if (ch == L'"')
				quoted += L"\\\"";
			else
				quoted.push_back(ch);
		}
		quoted.push_back(L'"');
		return quoted;
	}

	std::wstring FramePath(const std::wstring& folderPath, int frameIndex, const wchar_t* extension)
	{
		std::wstringstream name;
		name << folderPath;
		if (!folderPath.empty() && folderPath.back() != L'\\' && folderPath.back() != L'/')
			name << L'\\';
		name << L"frame_" << std::setw(6) << std::setfill(L'0') << frameIndex << extension;
		return name.str();
	}

	std::wstring MakeTemporaryFrameFolder()
	{
		wchar_t tempPath[MAX_PATH]{};
		if (::GetTempPathW(MAX_PATH, tempPath) == 0)
			return {};

		wchar_t seedPath[MAX_PATH]{};
		if (::GetTempFileNameW(tempPath, L"slx", 0, seedPath) == 0)
			return {};

		::DeleteFileW(seedPath);
		if (!::CreateDirectoryW(seedPath, nullptr))
			return {};
		return seedPath;
	}

	std::wstring FindFfmpegExecutable()
	{
		const std::wstring exeDir = GetExecutableDirectory();
		const std::wstring besideExe = exeDir + L"\\ffmpeg.exe";
		if (FileExistsW(besideExe))
			return besideExe;

		const std::wstring toolsExe = exeDir + L"\\tools\\ffmpeg.exe";
		if (FileExistsW(toolsExe))
			return toolsExe;

		wchar_t pathBuffer[MAX_PATH]{};
		const DWORD length = ::SearchPathW(nullptr, L"ffmpeg.exe", nullptr, MAX_PATH, pathBuffer, nullptr);
		if (length > 0 && length < MAX_PATH)
			return pathBuffer;
		return {};
	}

	bool RunHiddenProcess(const std::wstring& commandLine, DWORD* exitCode)
	{
		std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
		mutableCommand.push_back(L'\0');

		STARTUPINFOW startup{};
		startup.cb = sizeof(startup);
		PROCESS_INFORMATION process{};
		if (!::CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, FALSE,
			CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process))
		{
			return false;
		}

		::WaitForSingleObject(process.hProcess, INFINITE);
		DWORD code = 1;
		::GetExitCodeProcess(process.hProcess, &code);
		::CloseHandle(process.hThread);
		::CloseHandle(process.hProcess);
		if (exitCode)
			*exitCode = code;
		return true;
	}

	bool StartHiddenProcessAsync(const std::wstring& commandLine, PROCESS_INFORMATION& process)
	{
		std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
		mutableCommand.push_back(L'\0');

		STARTUPINFOW startup{};
		startup.cb = sizeof(startup);
		return ::CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, FALSE,
			CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process) != FALSE;
	}

	std::vector<std::wstring> BuildMovieEncodeCommands(
		const std::wstring& frameFolder,
		const std::wstring& outputPath,
		SlExportMovieFormat format,
		bool keepAlpha,
		int fps,
		std::wstring* errorMessage)
	{
		std::vector<std::wstring> commands;
		const std::wstring ffmpeg = FindFfmpegExecutable();
		if (ffmpeg.empty())
		{
			if (errorMessage)
				*errorMessage = L"ffmpeg.exe was not found. Put ffmpeg.exe beside spine love.exe, in a tools folder, or add it to PATH.";
			return commands;
		}

		fps = ClampExportFps(fps);
		std::wstring inputPattern = frameFolder;
		if (!inputPattern.empty() && inputPattern.back() != L'\\' && inputPattern.back() != L'/')
			inputPattern += L'\\';
		inputPattern += L"frame_%06d.png";

		std::wstring command = QuoteProcessArg(ffmpeg) +
			L" -hide_banner -loglevel error -y -framerate " + std::to_wstring(fps) +
			L" -start_number 1 -i " + QuoteProcessArg(inputPattern);

		if (format == SlExportMovieFormat::Mp4)
		{
			const std::wstring mp4Base = command + L" -vf \"crop=trunc(iw/2)*2:trunc(ih/2)*2\" ";
			const std::wstring mp4Tail = L" -pix_fmt yuv420p -movflags +faststart " + QuoteProcessArg(outputPath);
			commands.push_back(mp4Base + L"-c:v libx264 -crf 17" + mp4Tail);
			commands.push_back(mp4Base + L"-c:v h264_mf -b:v 12M" + mp4Tail);
			commands.push_back(mp4Base + L"-c:v h264_nvenc -cq 18" + mp4Tail);
			if (errorMessage)
				*errorMessage = L"ffmpeg could not encode MP4 with libx264, h264_mf, or h264_nvenc. The rendered PNG frames were left in:\n" + frameFolder;
			return commands;
		}
		if (format == SlExportMovieFormat::Gif)
		{
			command += L" -vf \"split[s0][s1];[s0]palettegen=max_colors=256[p];[s1][p]paletteuse=dither=sierra2_4a\" -loop 0 ";
			command += QuoteProcessArg(outputPath);
			commands.push_back(command);
			if (errorMessage)
				*errorMessage = L"ffmpeg failed. The rendered PNG frames were left in:\n" + frameFolder;
			return commands;
		}

		if (keepAlpha)
			command += L" -c:v libvpx-vp9 -crf 17 -b:v 0 -pix_fmt yuva420p -auto-alt-ref 0 ";
		else
			command += L" -c:v libvpx-vp9 -crf 24 -b:v 0 -pix_fmt yuv420p ";
		command += QuoteProcessArg(outputPath);
		commands.push_back(command);
		if (errorMessage)
			*errorMessage = L"ffmpeg failed. The rendered PNG frames were left in:\n" + frameFolder;
		return commands;
	}
}

bool SpineLoveWindow::RenderExportPixels(
	SlExportImageFormat format,
	bool keepAlpha,
	std::vector<unsigned char>& pixels,
	int& width,
	int& height,
	int& stride,
	SlColor& matteColor)
{
	pixels.clear();
	width = 0;
	height = 0;
	stride = 0;

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr || !runtime->ContainsDrawableContent())
		return false;
	if (!PrepareD3D11SpineRenderer())
		return false;

	int exportWidth = 0;
	int exportHeight = 0;
	if (!QuerySpineSurfaceSize(exportWidth, exportHeight))
		return false;

	const bool transparentOutput = keepAlpha && format == SlExportImageFormat::Png;
	const SlTextureId target = m_d3d11SpineRenderer.CreateRenderTarget(exportWidth, exportHeight);
	if (target == 0)
		return false;

	matteColor = SlColor(m_d3d11ClearColor.x, m_d3d11ClearColor.y, m_d3d11ClearColor.z, 1.0f);
	const SlVec4 clearColor = transparentOutput
		? SlVec4(0.0f, 0.0f, 0.0f, 0.0f)
		: SlVec4(matteColor.r, matteColor.g, matteColor.b, 1.0f);

	bool rendered = false;
	if (m_d3d11SpineRenderer.BeginRenderTarget(target, clearColor))
	{
		if (!transparentOutput && HasBackgroundTexture())
		{
			const float drawW = static_cast<float>(m_d3d11BackgroundWidth) * m_bgScale;
			const float drawH = static_cast<float>(m_d3d11BackgroundHeight) * m_bgScale;
			m_d3d11SpineRenderer.DrawSprite(
				m_d3d11BackgroundTexture,
				SlRect(m_bgOffsetX, m_bgOffsetY, drawW, drawH),
				SlRect(0.0f, 0.0f, 1.0f, 1.0f),
				SlColor(1.0f, 1.0f, 1.0f, 1.0f));
		}

		runtime->SetViewportSize(exportWidth, exportHeight);
		rendered = m_runtimeHub.RenderCurrentRuntimeD3D11(m_d3d11SpineRenderer);
		m_d3d11SpineRenderer.EndFrame();
	}

	m_d3d11Context.BindBackBuffer();
	const bool readback = rendered &&
		m_d3d11SpineRenderer.ReadTexturePixels(target, pixels, width, height, stride) &&
		width == exportWidth &&
		height == exportHeight;
	m_d3d11SpineRenderer.ReleaseTexture(target);
	m_d3d11SpineRenderer.BeginFrame(m_pendingClientWidth, m_pendingClientHeight);
	return readback;
}

void SpineLoveWindow::ExportCurrentView(SlExportImageFormat format, bool keepAlpha)
{
	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr || !runtime->ContainsDrawableContent())
	{
		m_fileDialogs.ShowOwnerError(L"Nothing is loaded to export.", m_windowHandle);
		return;
	}
	if (!PrepareD3D11SpineRenderer())
	{
		m_fileDialogs.ShowOwnerError(L"Renderer is not ready.", m_windowHandle);
		return;
	}

	int exportWidth = 0;
	int exportHeight = 0;
	if (!QuerySpineSurfaceSize(exportWidth, exportHeight))
	{
		m_fileDialogs.ShowOwnerError(L"Export size is invalid.", m_windowHandle);
		return;
	}

	const bool png = format == SlExportImageFormat::Png;
	const wchar_t* extension = png ? L".png" : L".jpg";
	std::wstring defaultName = m_panelState.currentFileName.empty()
		? L"spinelove_export"
		: sl_text::Utf8ToWide(m_panelState.currentFileName);
	defaultName += extension;

	SModalGuard guard(m_isModalOpen);
	shell_dialogs::FileDialogRequest request{};
	request.title = png ? L"Export PNG" : L"Export JPG";
	request.primaryFilter = png ? shell_dialogs::FileTypeFilter{ L"PNG image", L"*.png" }
		: shell_dialogs::FileTypeFilter{ L"JPG image", L"*.jpg;*.jpeg" };
	request.allowAnyFile = true;
	std::wstring savePath = m_fileDialogs.PickSavePath(request, defaultName.c_str(), m_windowHandle);
	if (savePath.empty())
		return;
	if (!EndsWithExtension(savePath, extension) && !(format == SlExportImageFormat::Jpeg && EndsWithExtension(savePath, L".jpeg")))
		savePath += extension;

	std::vector<unsigned char> pixels;
	int readWidth = 0;
	int readHeight = 0;
	int stride = 0;
	SlColor matteColor;
	if (!RenderExportPixels(format, keepAlpha, pixels, readWidth, readHeight, stride, matteColor))
	{
		m_fileDialogs.ShowOwnerError(L"Could not read export pixels.", m_windowHandle);
		return;
	}

	std::wstring errorMessage;
	const bool written = SaveExportImage(
		savePath.c_str(),
		format,
		pixels.data(),
		readWidth,
		readHeight,
		stride,
		png && keepAlpha,
		matteColor,
		&errorMessage);
	if (!written)
	{
		if (errorMessage.empty())
			errorMessage = L"Export failed.";
		m_fileDialogs.ShowOwnerError(errorMessage.c_str(), m_windowHandle);
	}
}

bool SpineLoveWindow::WriteAnimationFrames(
	const std::wstring& folderPath,
	SlExportImageFormat format,
	bool keepAlpha,
	int fps,
	std::wstring* errorMessage)
{
	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr || !runtime->ContainsDrawableContent())
	{
		if (errorMessage) *errorMessage = L"Nothing is loaded to export.";
		return false;
	}

	std::error_code ec;
	std::filesystem::create_directories(folderPath, ec);
	if (ec)
	{
		if (errorMessage) *errorMessage = L"Could not create frame folder.";
		return false;
	}

	fps = ClampExportFps(fps);
	std::string exportMotion = runtime->ActiveMotionName();
	if (exportMotion.empty() && !runtime->MotionNames().empty())
		exportMotion = runtime->MotionNames().front();

	const std::string restoreMotion = runtime->ActiveMotionName();
	const float restoreTimeScale = runtime->TimeScale();
	float restoreTrack = 0.0f;
	float restoreLast = 0.0f;
	runtime->ReadMotionClock(&restoreTrack, &restoreLast, nullptr, nullptr);

	float duration = exportMotion.empty() ? 0.0f : runtime->MotionDuration(exportMotion.c_str());
	if (!std::isfinite(duration) || duration < 0.0f)
		duration = 0.0f;
	const int frameCount = duration > 0.0f
		? (std::max)(1, static_cast<int>(std::ceil(duration * static_cast<float>(fps))))
		: 1;

	if (!exportMotion.empty())
		runtime->PlayMotionByName(exportMotion.c_str());
	runtime->SetTimeScale(1.0f);
	runtime->TickPlayback(0.0f);

	const wchar_t* extension = ImageExtension(format);
	for (int frame = 0; frame < frameCount; ++frame)
	{
		if (frame > 0)
			runtime->TickPlayback(1.0f / static_cast<float>(fps));

		std::vector<unsigned char> pixels;
		int width = 0;
		int height = 0;
		int stride = 0;
		SlColor matteColor;
		if (!RenderExportPixels(format, keepAlpha, pixels, width, height, stride, matteColor))
		{
			if (errorMessage) *errorMessage = L"Could not render animation frame.";
			break;
		}

		std::wstring writeError;
		const std::wstring path = FramePath(folderPath, frame + 1, extension);
		if (!SaveExportImage(path.c_str(), format, pixels.data(), width, height, stride,
			format == SlExportImageFormat::Png && keepAlpha, matteColor, &writeError))
		{
			if (errorMessage) *errorMessage = writeError.empty() ? L"Could not save animation frame." : writeError;
			break;
		}
	}

	if (!restoreMotion.empty())
	{
		runtime->PlayMotionByName(restoreMotion.c_str());
		if (restoreLast > 0.0f)
			runtime->TickPlayback(restoreLast);
	}
	runtime->SetTimeScale(restoreTimeScale);

	return errorMessage == nullptr || errorMessage->empty();
}

void SpineLoveWindow::ExportFrameSequence(SlExportImageFormat format, bool keepAlpha, bool useAnimationQueue)
{
	SModalGuard guard(m_isModalOpen);
	std::wstring folderPath = m_fileDialogs.PickFolder(m_windowHandle, L"Export animation frames");
	if (folderPath.empty())
		return;

	if (m_exportJob.kind != ExportJobKind::None)
	{
		m_fileDialogs.ShowOwnerError(L"Another export is already running.", m_windowHandle);
		return;
	}

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr || !runtime->ContainsDrawableContent())
	{
		m_fileDialogs.ShowOwnerError(L"Nothing is loaded to export.", m_windowHandle);
		return;
	}

	std::error_code ec;
	std::filesystem::create_directories(folderPath, ec);
	if (ec)
	{
		m_fileDialogs.ShowOwnerError(L"Could not create frame folder.", m_windowHandle);
		return;
	}

	ExportJobState job{};
	job.kind = ExportJobKind::FrameSequence;
	job.phase = ExportJobPhase::RenderingFrames;
	job.imageFormat = format;
	job.keepAlpha = keepAlpha;
	job.fps = ClampExportFps(m_panelState.exportImageFps);
	job.frameFolder = folderPath;
	job.useAnimationQueue = useAnimationQueue;
	job.exportMotions = SelectExportMotions(runtime, m_animQueue, useAnimationQueue);
	if (job.exportMotions.empty())
	{
		m_fileDialogs.ShowOwnerError(useAnimationQueue ? L"Animation queue is empty." : L"No animation is available to export.", m_windowHandle);
		return;
	}
	job.exportMotion = job.exportMotions.front();
	job.restoreMotion = runtime->ActiveMotionName();
	job.restoreTimeScale = runtime->TimeScale();
	runtime->ReadMotionClock(nullptr, &job.restoreLast, nullptr, nullptr);
	job.frameCount = BuildMotionFrameCounts(runtime, job.exportMotions, job.fps, job.motionFrameCounts);

	if (!job.exportMotion.empty())
		runtime->PlayMotionByName(job.exportMotion.c_str());
	runtime->SetTimeScale(1.0f);
	runtime->TickPlayback(0.0f);

	m_exportJob = job;
	m_panelState.exportRunning = true;
	m_panelState.exportFailed = false;
	m_panelState.exportDone = 0;
	m_panelState.exportTotal = job.frameCount;
	m_panelState.exportQueueActive = useAnimationQueue;
	m_panelState.exportQueueIndex = 0;
	m_panelState.exportStatus = useAnimationQueue ? "Rendering queue frames..." : "Rendering frames...";
}

bool SpineLoveWindow::EncodeExportMovie(
	const std::wstring& frameFolder,
	const std::wstring& outputPath,
	SlExportMovieFormat format,
	bool keepAlpha,
	int fps,
	std::wstring* errorMessage)
{
	const std::wstring ffmpeg = FindFfmpegExecutable();
	if (ffmpeg.empty())
	{
		if (errorMessage) *errorMessage = L"ffmpeg.exe was not found. Put ffmpeg.exe beside spine love.exe, in a tools folder, or add it to PATH.";
		return false;
	}

	fps = ClampExportFps(fps);
	std::wstring inputPattern = frameFolder;
	if (!inputPattern.empty() && inputPattern.back() != L'\\' && inputPattern.back() != L'/')
		inputPattern += L'\\';
	inputPattern += L"frame_%06d.png";

	std::wstring command = QuoteProcessArg(ffmpeg) +
		L" -hide_banner -loglevel error -y -framerate " + std::to_wstring(fps) +
		L" -start_number 1 -i " + QuoteProcessArg(inputPattern);

	if (format == SlExportMovieFormat::Mp4)
	{
		const std::wstring mp4Base = command + L" -vf \"crop=trunc(iw/2)*2:trunc(ih/2)*2\" ";
		const std::wstring mp4Tail = L" -pix_fmt yuv420p -movflags +faststart " + QuoteProcessArg(outputPath);
		const std::wstring encoderCommands[] =
		{
			mp4Base + L"-c:v libx264 -crf 17" + mp4Tail,
			mp4Base + L"-c:v h264_mf -b:v 12M" + mp4Tail,
			mp4Base + L"-c:v h264_nvenc -cq 18" + mp4Tail,
		};

		for (const std::wstring& candidate : encoderCommands)
		{
			::DeleteFileW(outputPath.c_str());
			DWORD exitCode = 1;
			if (RunHiddenProcess(candidate, &exitCode) && exitCode == 0)
				return true;
		}

		if (errorMessage)
			*errorMessage = L"ffmpeg could not encode MP4 with libx264, h264_mf, or h264_nvenc. The rendered PNG frames were left in:\n" + frameFolder;
		return false;
	}
	else if (keepAlpha)
	{
		command += L" -c:v libvpx-vp9 -crf 17 -b:v 0 -pix_fmt yuva420p -auto-alt-ref 0 ";
	}
	else
	{
		command += L" -c:v libvpx-vp9 -crf 24 -b:v 0 -pix_fmt yuv420p ";
	}
	command += QuoteProcessArg(outputPath);

	DWORD exitCode = 1;
	if (!RunHiddenProcess(command, &exitCode))
	{
		if (errorMessage) *errorMessage = L"Could not start ffmpeg.exe.";
		return false;
	}
	if (exitCode != 0)
	{
		if (errorMessage)
			*errorMessage = L"ffmpeg failed. The rendered PNG frames were left in:\n" + frameFolder;
		return false;
	}
	return true;
}

void SpineLoveWindow::AdvanceExportJob()
{
	if (m_exportJob.kind == ExportJobKind::None)
		return;

	auto restoreRuntime = [&]()
	{
		SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
		if (runtime == nullptr)
			return;
		if (!m_exportJob.restoreMotion.empty())
		{
			runtime->PlayMotionByName(m_exportJob.restoreMotion.c_str());
			if (m_exportJob.restoreLast > 0.0f)
				runtime->TickPlayback(m_exportJob.restoreLast);
		}
		runtime->SetTimeScale(m_exportJob.restoreTimeScale);
	};

	auto finishJob = [&](bool success, const char* status, const std::wstring& userError)
	{
		if (m_exportJob.encodeProcess != nullptr)
		{
			::CloseHandle(m_exportJob.encodeProcess);
			m_exportJob.encodeProcess = nullptr;
		}
		if (m_exportJob.encodeThread != nullptr)
		{
			::CloseHandle(m_exportJob.encodeThread);
			m_exportJob.encodeThread = nullptr;
		}

		const bool keepTempFrames = !success && m_exportJob.kind == ExportJobKind::Movie;
		if (success && m_exportJob.temporaryFrames)
		{
			std::error_code ignored;
			std::filesystem::remove_all(m_exportJob.frameFolder, ignored);
		}

		m_panelState.exportRunning = false;
		m_panelState.exportFailed = !success;
		m_panelState.exportDone = m_panelState.exportTotal;
		m_panelState.exportStatus = status ? status : (success ? "Export complete." : "Export failed.");
		m_panelState.exportQueueActive = false;
		m_panelState.exportQueueIndex = 0;

		if (!userError.empty())
			m_fileDialogs.ShowOwnerError(userError.c_str(), m_windowHandle);

		if (!keepTempFrames && !success && m_exportJob.temporaryFrames)
		{
			std::error_code ignored;
			std::filesystem::remove_all(m_exportJob.frameFolder, ignored);
		}
		m_exportJob = ExportJobState{};
	};

	auto startNextEncodeCommand = [&]() -> bool
	{
		while (m_exportJob.encodeCommandIndex < m_exportJob.encodeCommands.size())
		{
			::DeleteFileW(m_exportJob.outputPath.c_str());
			PROCESS_INFORMATION process{};
			if (StartHiddenProcessAsync(m_exportJob.encodeCommands[m_exportJob.encodeCommandIndex], process))
			{
				m_exportJob.encodeProcess = process.hProcess;
				m_exportJob.encodeThread = process.hThread;
				m_exportJob.phase = ExportJobPhase::Encoding;
				m_panelState.exportDone = m_panelState.exportTotal;
				m_panelState.exportStatus = "Encoding video...";
				return true;
			}
			++m_exportJob.encodeCommandIndex;
		}
		return false;
	};

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr || !runtime->ContainsDrawableContent())
	{
		restoreRuntime();
		finishJob(false, "Export failed.", L"Nothing is loaded to export.");
		return;
	}

	if (m_exportJob.phase == ExportJobPhase::ReadyToEncode)
	{
		m_panelState.exportStatus = "Encoding video...";
		m_exportJob.encodeCommands = BuildMovieEncodeCommands(
			m_exportJob.frameFolder,
			m_exportJob.outputPath,
			m_exportJob.movieFormat,
			m_exportJob.keepAlpha,
			m_exportJob.fps,
			&m_exportJob.encodeFailureMessage);
		m_exportJob.encodeCommandIndex = 0;
		if (!startNextEncodeCommand())
			finishJob(false, "Video export failed.", m_exportJob.encodeFailureMessage.empty() ? L"Could not start ffmpeg.exe." : m_exportJob.encodeFailureMessage);
		return;
	}

	if (m_exportJob.phase == ExportJobPhase::Encoding)
	{
		if (m_exportJob.encodeProcess == nullptr)
		{
			if (!startNextEncodeCommand())
				finishJob(false, "Video export failed.", m_exportJob.encodeFailureMessage);
			return;
		}

		const DWORD waitResult = ::WaitForSingleObject(m_exportJob.encodeProcess, 0);
		if (waitResult == WAIT_TIMEOUT)
		{
			m_panelState.exportDone = m_panelState.exportTotal;
			m_panelState.exportStatus = "Encoding video...";
			return;
		}

		DWORD exitCode = 1;
		::GetExitCodeProcess(m_exportJob.encodeProcess, &exitCode);
		::CloseHandle(m_exportJob.encodeProcess);
		::CloseHandle(m_exportJob.encodeThread);
		m_exportJob.encodeProcess = nullptr;
		m_exportJob.encodeThread = nullptr;

		if (exitCode == 0)
		{
			finishJob(true, "Video export complete.", {});
			return;
		}

		++m_exportJob.encodeCommandIndex;
		if (!startNextEncodeCommand())
			finishJob(false, "Video export failed.", m_exportJob.encodeFailureMessage);
		return;
	}

	if (m_exportJob.phase != ExportJobPhase::RenderingFrames)
		return;

	std::vector<unsigned char> pixels;
	int width = 0;
	int height = 0;
	int stride = 0;
	SlColor matteColor;
	if (!RenderExportPixels(m_exportJob.imageFormat, m_exportJob.keepAlpha, pixels, width, height, stride, matteColor))
	{
		restoreRuntime();
		finishJob(false, "Export failed.", L"Could not render animation frame.");
		return;
	}

	std::wstring writeError;
	const std::wstring path = FramePath(
		m_exportJob.frameFolder,
		m_exportJob.frameIndex + 1,
		ImageExtension(m_exportJob.imageFormat));
	if (!SaveExportImage(path.c_str(),
		m_exportJob.imageFormat,
		pixels.data(),
		width,
		height,
		stride,
		m_exportJob.imageFormat == SlExportImageFormat::Png && m_exportJob.keepAlpha,
		matteColor,
		&writeError))
	{
		restoreRuntime();
		finishJob(false, "Export failed.", writeError.empty() ? L"Could not save animation frame." : writeError);
		return;
	}

	++m_exportJob.frameIndex;
	++m_exportJob.frameInMotion;
	m_panelState.exportDone = m_exportJob.frameIndex;
	m_panelState.exportTotal = m_exportJob.frameCount;
	if (m_exportJob.useAnimationQueue)
	{
		std::ostringstream status;
		status << "Rendering queue ";
		status << (m_exportJob.kind == ExportJobKind::Movie ? "video" : "frames");
		status << " (" << (m_exportJob.exportMotionIndex + 1) << "/" << m_exportJob.exportMotions.size() << ")";
		if (m_exportJob.exportMotionIndex < m_exportJob.exportMotions.size())
			status << " " << m_exportJob.exportMotions[m_exportJob.exportMotionIndex];
		status << "...";
		m_panelState.exportStatus = status.str();
		m_panelState.exportQueueActive = true;
		m_panelState.exportQueueIndex = m_exportJob.exportMotionIndex;
	}
	else
	{
		m_panelState.exportStatus = (m_exportJob.kind == ExportJobKind::Movie)
			? "Rendering video frames..."
			: "Rendering frames...";
	}

	if (m_exportJob.frameIndex < m_exportJob.frameCount)
	{
		const int currentMotionFrames = m_exportJob.exportMotionIndex < m_exportJob.motionFrameCounts.size()
			? m_exportJob.motionFrameCounts[m_exportJob.exportMotionIndex]
			: m_exportJob.frameCount;
		if (m_exportJob.frameInMotion < currentMotionFrames)
		{
			runtime->TickPlayback(1.0f / static_cast<float>(m_exportJob.fps));
		}
		else if (m_exportJob.exportMotionIndex + 1 < m_exportJob.exportMotions.size())
		{
			++m_exportJob.exportMotionIndex;
			m_exportJob.frameInMotion = 0;
			m_exportJob.exportMotion = m_exportJob.exportMotions[m_exportJob.exportMotionIndex];
			m_panelState.exportQueueIndex = m_exportJob.exportMotionIndex;
			runtime->PlayMotionByName(m_exportJob.exportMotion.c_str());
			runtime->TickPlayback(0.0f);
		}
		return;
	}

	restoreRuntime();
	if (m_exportJob.kind == ExportJobKind::Movie)
	{
		m_exportJob.phase = ExportJobPhase::ReadyToEncode;
		m_panelState.exportStatus = "Encoding video...";
	}
	else
	{
		finishJob(true, "Frame export complete.", {});
	}
}

void SpineLoveWindow::ExportMovie(SlExportMovieFormat format, bool keepAlpha, bool useAnimationQueue)
{
	const bool mp4 = format == SlExportMovieFormat::Mp4;
	const bool gif = format == SlExportMovieFormat::Gif;
	const wchar_t* extension = MovieExtension(format);
	const wchar_t* dialogTitle = MovieExportDialogTitle(format);
	std::wstring defaultName = m_panelState.currentFileName.empty()
		? L"spinelove_export"
		: sl_text::Utf8ToWide(m_panelState.currentFileName);
	defaultName += extension;

	if (FindFfmpegExecutable().empty())
	{
		SModalGuard guard(m_isModalOpen);
		ShowFfmpegMissingDialog(m_windowHandle, dialogTitle);
		return;
	}

	SModalGuard guard(m_isModalOpen);
	shell_dialogs::FileDialogRequest request{};
	request.title = mp4 ? L"Export MP4" : (gif ? L"Export GIF" : L"Export WebM");
	request.primaryFilter = mp4 ? shell_dialogs::FileTypeFilter{ L"MP4 video", L"*.mp4" }
		: (gif ? shell_dialogs::FileTypeFilter{ L"GIF image", L"*.gif" }
			: shell_dialogs::FileTypeFilter{ L"WebM video", L"*.webm" });
	request.allowAnyFile = true;

	std::wstring savePath = m_fileDialogs.PickSavePath(request, defaultName.c_str(), m_windowHandle);
	if (savePath.empty())
		return;
	if (!EndsWithExtension(savePath, extension))
		savePath += extension;

	std::wstring frameFolder = MakeTemporaryFrameFolder();
	if (frameFolder.empty())
	{
		m_fileDialogs.ShowOwnerError(L"Could not create temporary frame folder.", m_windowHandle);
		return;
	}

	if (m_exportJob.kind != ExportJobKind::None)
	{
		std::error_code ignored;
		std::filesystem::remove_all(frameFolder, ignored);
		m_fileDialogs.ShowOwnerError(L"Another export is already running.", m_windowHandle);
		return;
	}

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr || !runtime->ContainsDrawableContent())
	{
		std::error_code ignored;
		std::filesystem::remove_all(frameFolder, ignored);
		m_fileDialogs.ShowOwnerError(L"Nothing is loaded to export.", m_windowHandle);
		return;
	}

	ExportJobState job{};
	job.kind = ExportJobKind::Movie;
	job.phase = ExportJobPhase::RenderingFrames;
	job.imageFormat = SlExportImageFormat::Png;
	job.movieFormat = format;
	job.keepAlpha = format == SlExportMovieFormat::Webm && keepAlpha;
	job.fps = ClampExportFps(m_panelState.exportVideoFps);
	job.frameFolder = frameFolder;
	job.outputPath = savePath;
	job.temporaryFrames = true;
	job.useAnimationQueue = useAnimationQueue;
	job.exportMotions = SelectExportMotions(runtime, m_animQueue, useAnimationQueue);
	if (job.exportMotions.empty())
	{
		std::error_code ignored;
		std::filesystem::remove_all(frameFolder, ignored);
		m_fileDialogs.ShowOwnerError(useAnimationQueue ? L"Animation queue is empty." : L"No animation is available to export.", m_windowHandle);
		return;
	}
	job.exportMotion = job.exportMotions.front();
	job.restoreMotion = runtime->ActiveMotionName();
	job.restoreTimeScale = runtime->TimeScale();
	runtime->ReadMotionClock(nullptr, &job.restoreLast, nullptr, nullptr);
	job.frameCount = BuildMotionFrameCounts(runtime, job.exportMotions, job.fps, job.motionFrameCounts);

	if (!job.exportMotion.empty())
		runtime->PlayMotionByName(job.exportMotion.c_str());
	runtime->SetTimeScale(1.0f);
	runtime->TickPlayback(0.0f);

	m_exportJob = job;
	m_panelState.exportRunning = true;
	m_panelState.exportFailed = false;
	m_panelState.exportDone = 0;
	m_panelState.exportTotal = job.frameCount;
	m_panelState.exportQueueActive = useAnimationQueue;
	m_panelState.exportQueueIndex = 0;
	m_panelState.exportStatus = useAnimationQueue ? "Rendering queue video frames..." : "Rendering video frames...";
}
