#ifndef UI_CORE_H_
#define UI_CORE_H_

class CUiCore
{
public:
	CUiCore(const char* defaultFontfilePath = nullptr, float fontSize = DefaultFontSize);
	~CUiCore();

	bool HasBeenInitialised() const { return m_bInitialised; }

	static void NewFrame();
	static void Render();

	static void UpdateAndRenderViewPorts();
private:
	static constexpr float DefaultFontSize = 20.f;
	bool m_bInitialised = false;
};

#endif
