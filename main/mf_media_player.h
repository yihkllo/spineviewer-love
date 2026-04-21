#ifndef MF_MEDIA_PLAYER_H_
#define MF_MEDIA_PLAYER_H_

#include <Windows.h>
#include <mfmediaengine.h>

class CMfMediaPlayerNotify : public IMFMediaEngineNotify
{
public:
	CMfMediaPlayerNotify(void* pMediaPlayer);
	~CMfMediaPlayerNotify();
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		if (riid == __uuidof(IMFMediaEngineNotify))
		{
			*ppv = static_cast<IMFMediaEngineNotify*>(this);
		}
		else
		{
			*ppv = nullptr;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}
	STDMETHODIMP EventNotify(DWORD Event, DWORD_PTR param1, DWORD param2)
	{
		if (Event == MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE)
		{
			::SetEvent(reinterpret_cast<HANDLE>(param1));
		}
		else
		{
			OnMediaEngineEvent(Event, param1, param2);
		}
		return S_OK;
	}
	STDMETHODIMP_(ULONG) AddRef() { return ::InterlockedIncrement(&m_lRef); }
	STDMETHODIMP_(ULONG) Release()
	{
		LONG lRef = ::InterlockedDecrement(&m_lRef);
		if (!lRef)delete this;
		return lRef;
	}

private:
	LONG m_lRef = 0;
	void* m_pPlayer = nullptr;

	void OnMediaEngineEvent(DWORD Event, DWORD_PTR param1, DWORD param2);
};

class CMfMediaPlayer
{
public:
	CMfMediaPlayer();
	~CMfMediaPlayer();

	bool Play(const wchar_t* pwzFilePath);
	BOOL SwitchLoop();
	BOOL SwitchMute();
	BOOL SwitchPause();
	double GetCurrentVolume();
	double GetCurrentRate();
	long long GetCurrentTimeInMilliSeconds();
	bool SetCurrentVolume(double dbVolume);
	bool SetCurrentRate(double dbRate);
	bool IsEnded();

	virtual bool SetPlaybackWindow(HWND hWnd, UINT uMsg = 0);
	bool GetVideoSize(DWORD* dwWidth, DWORD* dwHeight);
	void SetDisplayArea(const RECT absoluteRect);
	virtual bool ResizeBuffer();

	HWND GetRetHwnd()const { return m_hRetWnd; }
	UINT GetRetMsg() const { return m_uRetMsg; };
protected:
	HWND m_hRetWnd = nullptr;
	UINT m_uRetMsg = 0;

	HRESULT m_hrComInit = E_FAIL;
	HRESULT m_hrMfStart = E_FAIL;
	CMfMediaPlayerNotify* m_pmfNotify = nullptr;
	IMFMediaEngineEx* m_pmfEngineEx = nullptr;
	IMFAttributes* m_pMfAttributes = nullptr;

	BOOL m_iLoop = FALSE;
	BOOL m_iMute = FALSE;
	BOOL m_iPause = FALSE;

	MFVideoNormalizedRect m_normalisedRect{};

	void WorkOutNormalisedRect(const RECT absoluteRect, MFVideoNormalizedRect* normaisedRect);
};

#endif
