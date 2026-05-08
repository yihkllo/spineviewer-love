#include "d3d11_test_app.h"

#include <cstdint>
#include <cwchar>
#include <unordered_map>

#include "d3d11_context.h"
#include "d3d11_renderer.h"

namespace {

struct TestApp
{
	HWND hwnd = nullptr;
	sl_d3d11::D3D11Context context;
	sl_d3d11::D3D11Renderer renderer;
	SlTextureId texture = 0;
	SlTextureId renderTarget = 0;
	bool ready = false;
};

TestApp* GetApp(HWND hwnd)
{
	return reinterpret_cast<TestApp*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

std::wstring ExeDir()
{
	wchar_t path[MAX_PATH] = {};
	::GetModuleFileNameW(nullptr, path, MAX_PATH);
	wchar_t* slash = std::wcsrchr(path, L'\\');
	if (slash)
		*slash = L'\0';
	return path;
}

LRESULT CALLBACK TestWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_NCCREATE)
	{
		auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);
		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
	}

	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			::DestroyWindow(hwnd);
			return 0;
		}
		break;
	default:
		break;
	}
	return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

bool Init(TestApp& app, HINSTANCE instance, int showCommand)
{
	const wchar_t* className = L"SpineLoveD3D11Smoke";
	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(wc);
	wc.hInstance = instance;
	wc.lpfnWndProc = TestWndProc;
	wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
	wc.lpszClassName = className;
	::RegisterClassExW(&wc);

	RECT rect{ 0, 0, 960, 540 };
	::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	app.hwnd = ::CreateWindowExW(0, className, L"SpineLove D3D11 Sprite Smoke",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr, nullptr, instance, &app);
	if (app.hwnd == nullptr)
		return false;

	if (!app.context.Initialize(app.hwnd, 960, 540))
		return false;

	const std::wstring dir = ExeDir();
	const std::wstring shaderPath = dir + L"\\render_d3d11\\shaders\\sprite.hlsl";
	if (!app.renderer.Initialize(app.context.Device(), app.context.Context(), shaderPath.c_str()))
		return false;

	const std::wstring texturePath = dir + L"\\app.png";
	app.texture = app.renderer.LoadTexture(texturePath.c_str());
	if (app.texture == 0)
		app.texture = 1;
	app.renderTarget = app.renderer.CreateRenderTarget(512, 384);
	if (app.renderTarget == 0)
		return false;

	app.ready = true;
	::ShowWindow(app.hwnd, showCommand);
	::UpdateWindow(app.hwnd);
	return true;
}

void Render(TestApp& app)
{
	SlDrawList drawList;
	drawList.width = 512;
	drawList.height = 384;
	SlDrawCommand command;
	command.textureId = 1;
	command.blendMode = SlBlendMode::Normal;
	command.vertices.resize(4);
	const float size = 256.0f;
	const float x = 128.0f;
	const float y = 64.0f;
	command.vertices[0].pos = SlVec3(x, y, 0.0f);
	command.vertices[1].pos = SlVec3(x + size, y, 0.0f);
	command.vertices[2].pos = SlVec3(x + size, y + size, 0.0f);
	command.vertices[3].pos = SlVec3(x, y + size, 0.0f);
	command.vertices[0].uv = SlVec2(0.0f, 0.0f);
	command.vertices[1].uv = SlVec2(1.0f, 0.0f);
	command.vertices[2].uv = SlVec2(1.0f, 1.0f);
	command.vertices[3].uv = SlVec2(0.0f, 1.0f);
	for (int i = 0; i < 4; ++i)
		command.vertices[i].color = SlColor(1.0f, 1.0f, 1.0f, 1.0f);
	const unsigned short indices[] = { 0, 1, 2, 2, 3, 0 };
	command.indices.assign(indices, indices + 6);
	drawList.commands.push_back(command);
	std::unordered_map<std::uint64_t, SlTextureId> textureMap;
	textureMap[1] = app.texture;

	app.renderer.BeginRenderTarget(app.renderTarget, SlVec4(0.02f, 0.03f, 0.04f, 0.0f));
	app.renderer.Submit(drawList, textureMap);
	app.renderer.EndFrame();

	app.context.BeginFrame(SlVec4(0.08f, 0.09f, 0.10f, 1.0f));
	app.renderer.BeginFrame(app.context.Width(), app.context.Height());
	const float outW = 512.0f;
	const float outH = 384.0f;
	const float outX = (static_cast<float>(app.context.Width()) - outW) * 0.5f;
	const float outY = (static_cast<float>(app.context.Height()) - outH) * 0.5f;
	app.renderer.DrawSprite(app.renderTarget, SlRect(outX, outY, outW, outH), SlRect(0.0f, 0.0f, 1.0f, 1.0f), SlColor(1.0f, 1.0f, 1.0f, 1.0f));
	app.renderer.EndFrame();

	app.context.EndFrame();
}

}

int RunD3D11SpriteSmoke(HINSTANCE instance, int showCommand)
{
	TestApp app;
	if (!Init(app, instance, showCommand))
	{
		::MessageBoxW(nullptr, L"Failed to start D3D11 sprite smoke test.", L"spinelove", MB_ICONERROR);
		return 1;
	}

	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		else
		{
			Render(app);
		}
	}
	return static_cast<int>(msg.wParam);
}
