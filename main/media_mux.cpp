

#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfapi.h>
#ifdef MF_GPU_TEXTURE
	#include <d3d11.h>
#endif
#include <atlbase.h>

#include "media_mux.h"

#pragma comment (lib, "Mfplat.lib")
#pragma comment (lib, "Mfreadwrite.lib")
#pragma comment (lib, "Mfuuid.lib")

class CMfVideoEncoder::Impl
{
public:
	Impl();
	~Impl();

	bool initialise(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int frameRate);
	bool hasBeenInitialised() const;

	bool commitCpuFrame(unsigned char* pPixels, unsigned long pixelSize, bool isRgba = true);
#ifdef MF_GPU_TEXTURE
	bool commitGpuFrame(void* pD3D11Texture2D);
#endif
	void finalise();
private:
	HRESULT m_hrComInit = E_FAIL;
	HRESULT m_hrMStartup = E_FAIL;

	IMFSinkWriter* m_pMfSinkWriter = nullptr;
	IMFMediaBuffer* m_pMfMediaBuffer = nullptr;

	LONGLONG m_llCurrentFrame = 0;
	UINT32 m_uiFrameRate = DefaultFrameRate;
	DWORD m_ulOutStreamIndex = 0;

	bool m_hasBeenInitialised = false;

	bool addFrame();

	void clear();

	void releaseMfSinkWriter();

	bool createMediaBuffer(DWORD ulPixelSize);
	bool checkMediaBufferSize(DWORD ulPixelSize);
	void releaseMediaBuffer();
};

CMfVideoEncoder::Impl::Impl()
{

}

CMfVideoEncoder::Impl::~Impl()
{
	clear();

	if (SUCCEEDED(m_hrMStartup))
	{
		::MFShutdown();
		m_hrMStartup = E_FAIL;
	}

	if (SUCCEEDED(m_hrComInit))
	{
		::CoUninitialize();
		m_hrComInit = E_FAIL;
	}
}

bool CMfVideoEncoder::Impl::initialise(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int frameRate)
{
	clear();


	if (FAILED(m_hrComInit))
	{
		m_hrComInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	}
	if (FAILED(m_hrMStartup))
	{
		m_hrMStartup = ::MFStartup(MF_VERSION);
		if (FAILED(m_hrMStartup)) return false;
	}

	m_uiFrameRate = frameRate;

	CComPtr<IMFAttributes> pMfAttriubutes;
	HRESULT hr = ::MFCreateAttributes(&pMfAttriubutes, 1);
	if (FAILED(hr))return false;

	hr = pMfAttriubutes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
	if (FAILED(hr))return false;
	hr = pMfAttriubutes->SetUINT32(MF_MT_DEFAULT_STRIDE, width * 4);
	if (FAILED(hr))return false;
	hr = pMfAttriubutes->SetGUID(MF_TRANSCODE_CONTAINERTYPE, MFTranscodeContainerType_MPEG4);
	if (FAILED(hr))return false;

	hr = ::MFCreateSinkWriterFromURL(filePath, nullptr, pMfAttriubutes, &m_pMfSinkWriter);
	if (FAILED(hr))return false;

	const auto SetOutStreamMediaType = [this, &width, &height]()
		-> bool
		{
			HRESULT hr = E_FAIL;
			CComPtr<IMFMediaType> pOutMfMediaType;
			hr = ::MFCreateMediaType(&pOutMfMediaType);
			if (FAILED(hr))return false;

			hr = pOutMfMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			if (FAILED(hr))return false;
			hr = pOutMfMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
			if (FAILED(hr))return false;

			UINT32 uiBitrate = width * height * 5;
			hr = pOutMfMediaType->SetUINT32(MF_MT_AVG_BITRATE, uiBitrate);
			if (FAILED(hr))return false;
			hr = pOutMfMediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
			if (FAILED(hr))return false;

			hr = ::MFSetAttributeSize(pOutMfMediaType, MF_MT_FRAME_SIZE, width, height);
			if (FAILED(hr))return false;
			hr = ::MFSetAttributeSize(pOutMfMediaType, MF_MT_FRAME_RATE, m_uiFrameRate, 1);
			if (FAILED(hr))return false;
			hr = ::MFSetAttributeSize(pOutMfMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			if (FAILED(hr))return false;

			hr = m_pMfSinkWriter->AddStream(pOutMfMediaType, &m_ulOutStreamIndex);
			if (FAILED(hr))return false;

			return true;
		};

	bool bRet = SetOutStreamMediaType();
	if (!bRet)return false;

	const auto SetInputMediaType = [this, &width, &height]()
		-> bool
		{
			HRESULT hr = E_FAIL;
			CComPtr<IMFMediaType> pInMfMediaType;
			hr = ::MFCreateMediaType(&pInMfMediaType);
			if (FAILED(hr))return false;

			hr = pInMfMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			if (FAILED(hr))return false;
			hr = pInMfMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
			if (FAILED(hr))return false;

			hr = pInMfMediaType->SetUINT32(MF_MT_DEFAULT_STRIDE, width * 4);
			if (FAILED(hr))return false;

			hr = ::MFSetAttributeSize(pInMfMediaType, MF_MT_FRAME_SIZE, width, height);
			if (FAILED(hr))return false;
			hr = ::MFSetAttributeSize(pInMfMediaType, MF_MT_FRAME_RATE, m_uiFrameRate, 1);
			if (FAILED(hr))return false;
			hr = ::MFSetAttributeSize(pInMfMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			if (FAILED(hr))return false;

			hr = m_pMfSinkWriter->SetInputMediaType(m_ulOutStreamIndex, pInMfMediaType, nullptr);
			if (FAILED(hr))return false;

			return true;
		};

	bRet = SetInputMediaType();
	if (!bRet)return false;

	hr = m_pMfSinkWriter->BeginWriting();

	m_hasBeenInitialised = SUCCEEDED(hr);

	return m_hasBeenInitialised;
}

bool CMfVideoEncoder::Impl::hasBeenInitialised() const
{
	return m_hasBeenInitialised;
}

bool CMfVideoEncoder::Impl::commitCpuFrame(unsigned char* pPixels, unsigned long pixelSize, bool isRgba)
{
	if (m_pMfSinkWriter == nullptr)return false;

	bool bRet = checkMediaBufferSize(pixelSize);
	if (!bRet)return false;

	BYTE* pBuffer = nullptr;
	HRESULT hr = m_pMfMediaBuffer->Lock(&pBuffer, nullptr, nullptr);
	if (FAILED(hr))return false;

	if (isRgba)
	{
		uint32_t* pSrc32 = reinterpret_cast<uint32_t*>(pPixels);
		uint32_t* pDst32 = reinterpret_cast<uint32_t*>(pBuffer);
		size_t nCount = pixelSize / 4;
		for (size_t i = 0; i < nCount; ++i)
		{
			pDst32[i] = ((pSrc32[i] & 0x000000ff) << 16) | ((pSrc32[i] & 0x00ff0000) >> 16) | ((pSrc32[i] & 0xff00ff00));
		}
	}
	else
	{
		memcpy(pBuffer, pPixels, pixelSize);
	}

	hr = m_pMfMediaBuffer->SetCurrentLength(pixelSize);
	if (FAILED(hr))return false;

	hr = m_pMfMediaBuffer->Unlock();
	if (FAILED(hr))return false;

	return addFrame();
}
#ifdef MF_GPU_TEXTURE
bool CMfVideoEncoder::Impl::commitGpuFrame(void* pD3D11Texture2D)
{
	if (pD3D11Texture2D == nullptr)return false;

	auto pFrameTexture = static_cast<ID3D11Texture2D*>(pD3D11Texture2D);

	releaseMediaBuffer();
	HRESULT hr = ::MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pFrameTexture, 0, FALSE, &m_pMfMediaBuffer);
	if (FAILED(hr))return false;

	CComPtr<IMF2DBuffer> pMf2dBuffer;
	hr = m_pMfMediaBuffer->QueryInterface(__uuidof(IMF2DBuffer), reinterpret_cast<void**>(&pMf2dBuffer));
	if (FAILED(hr))return false;

	DWORD ulLength = 0;
	hr = pMf2dBuffer->GetContiguousLength(&ulLength);
	if (FAILED(hr))return false;

	m_pMfMediaBuffer->SetCurrentLength(ulLength);

	return addFrame();
}
#endif

void CMfVideoEncoder::Impl::finalise()
{
	if (m_pMfSinkWriter == nullptr)return;

	m_pMfSinkWriter->Finalize();
	clear();
}

bool CMfVideoEncoder::Impl::addFrame()
{
	if (m_pMfSinkWriter == nullptr || m_pMfMediaBuffer == nullptr)return false;

	CComPtr<IMFSample> pMfSample;
	HRESULT hr = ::MFCreateSample(&pMfSample);
	if (FAILED(hr)) return false;

	hr = pMfSample->AddBuffer(m_pMfMediaBuffer);
	if (FAILED(hr)) return false;

	LONGLONG llDuration = 10LL * 1000 * 1000 / m_uiFrameRate;
	hr = pMfSample->SetSampleTime(llDuration * m_llCurrentFrame);
	if (FAILED(hr)) return false;
	++m_llCurrentFrame;

	hr = pMfSample->SetSampleDuration(llDuration);
	if (FAILED(hr)) return false;

	hr = m_pMfSinkWriter->WriteSample(m_ulOutStreamIndex, pMfSample);

	return SUCCEEDED(hr);
}

void CMfVideoEncoder::Impl::clear()
{
	releaseMfSinkWriter();
	releaseMediaBuffer();

	m_llCurrentFrame = 0;
	m_uiFrameRate = DefaultFrameRate;
	m_ulOutStreamIndex = 0;
}

void CMfVideoEncoder::Impl::releaseMfSinkWriter()
{
	if (m_pMfSinkWriter != nullptr)
	{
		m_pMfSinkWriter->Release();
		m_pMfSinkWriter = nullptr;
	}
}

bool CMfVideoEncoder::Impl::createMediaBuffer(DWORD ulPixelSize)
{
	releaseMediaBuffer();
	HRESULT hr = ::MFCreateMemoryBuffer(ulPixelSize, &m_pMfMediaBuffer);
	return SUCCEEDED(hr);
}

bool CMfVideoEncoder::Impl::checkMediaBufferSize(DWORD ulPixelSize)
{
	if (!m_pMfMediaBuffer)
		return createMediaBuffer(ulPixelSize);

	DWORD ulSize = 0;
	m_pMfMediaBuffer->GetMaxLength(&ulSize);
	return (ulPixelSize <= ulSize) || createMediaBuffer(ulPixelSize);
}

void CMfVideoEncoder::Impl::releaseMediaBuffer()
{
	if (m_pMfMediaBuffer)
	{
		m_pMfMediaBuffer->Release();
		m_pMfMediaBuffer = nullptr;
	}
}


CMfVideoEncoder::CMfVideoEncoder()  : m_pImpl(std::make_unique<Impl>()) {}
CMfVideoEncoder::~CMfVideoEncoder() = default;

bool CMfVideoEncoder::initialise(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int frameRate)
{
	return m_pImpl->initialise(filePath, width, height, frameRate);
}

bool CMfVideoEncoder::hasBeenInitialised() const
{
	return m_pImpl->hasBeenInitialised();
}

bool CMfVideoEncoder::commitCpuFrame(unsigned char* pPixels, unsigned long pixelSize, bool isRgba)
{
	return m_pImpl->commitCpuFrame(pPixels, pixelSize, isRgba);
}

#ifdef MF_GPU_TEXTURE
bool CMfVideoEncoder::commitGpuFrame(void* pD3D11Texture2D)
{
	return m_pImpl->commiGpuFrame(pD3D11Texture2D);
}
#endif

void CMfVideoEncoder::finalise()
{
	return m_pImpl->finalise();
}
