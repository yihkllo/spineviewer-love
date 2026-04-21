#ifndef RENDER_SETUP_H_
#define RENDER_SETUP_H_


class CDxLibContext
{
public:
	explicit CDxLibContext(void* pWindowHandle = nullptr);
	~CDxLibContext();

	CDxLibContext(const CDxLibContext&) = delete;
	CDxLibContext& operator=(const CDxLibContext&) = delete;

	bool IsReady() const noexcept { return m_bInitialised; }

private:
	bool m_bInitialised = false;

	static bool SetupBackend(void* pWindowHandle);
	static void ApplyDefaults();
};


using SDxLibInit = CDxLibContext;

#endif
