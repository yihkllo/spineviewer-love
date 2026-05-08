#ifndef SPINELOVE_IMGUI_UI_CORE_H_
#define SPINELOVE_IMGUI_UI_CORE_H_

struct ID3D11Device;
struct ID3D11DeviceContext;

class CUiCore
{
public:
	struct InitParams
	{
		void* windowHandle = nullptr;
		ID3D11Device* d3d11Device = nullptr;
		ID3D11DeviceContext* d3d11Context = nullptr;
		const char* fontPath = nullptr;
		float fontSize = DefaultFontSize;
	};

	explicit CUiCore(const InitParams& params);
	~CUiCore();

	CUiCore(const CUiCore&) = delete;
	CUiCore& operator=(const CUiCore&) = delete;

	bool Ready() const { return m_bInitialised; }

	bool BeginFrame();
	void SubmitFrame();
private:
	static constexpr float DefaultFontSize = 20.f;
	bool m_bInitialised = false;
	bool m_imguiContextReady = false;
	bool m_win32BackendReady = false;
	bool m_d3d11BackendReady = false;
};

#endif
