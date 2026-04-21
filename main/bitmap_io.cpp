
#include <atlbase.h>
#include <wincodec.h>

#include "bitmap_io.h"

#pragma comment (lib,"Windowscodecs.lib")

namespace win_image
{
	enum class EOutputType
	{
		Png,
		Jpg
	};

	static GUID ToWicGuid(EOutputType eOutputType)
	{
		switch (eOutputType)
		{
		case EOutputType::Png: return GUID_ContainerFormatPng;
		case EOutputType::Jpg: return GUID_ContainerFormatJpeg;
		default:break;
		}

		return {};
	}

	static bool LoadImageWithTransformOption(const wchar_t* filePath, IWICBitmap** pOutWicBitmap, float fScale, ERotation rotation)
	{
		if (filePath == nullptr || pOutWicBitmap == nullptr)return false;

		CComPtr<IWICImagingFactory> pWicImageFactory;
		HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
		if (FAILED(hr)) return false;

		CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
		hr = pWicImageFactory->CreateDecoderFromFilename(filePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pWicBitmapDecoder);
		if (FAILED(hr)) return false;

		CComPtr<IWICBitmapFrameDecode> pWicFrameDecode;
		hr = pWicBitmapDecoder->GetFrame(0, &pWicFrameDecode);
		if (FAILED(hr)) return false;

		CComPtr<IWICBitmapSource> pWicCurrentSource;
		hr = pWicFrameDecode->QueryInterface(IID_PPV_ARGS(&pWicCurrentSource));
		if (FAILED(hr)) return false;

		if (fScale != 1.f)
		{
			UINT uiWidth = 0, uiHeight = 0;
			hr = pWicCurrentSource->GetSize(&uiWidth, &uiHeight);
			if (FAILED(hr)) return false;

			CComPtr<IWICBitmapScaler> pWicBmpScaler;
			hr = pWicImageFactory->CreateBitmapScaler(&pWicBmpScaler);
			if (FAILED(hr)) return false;

			hr = pWicBmpScaler->Initialize(pWicCurrentSource, static_cast<UINT>(uiWidth * fScale), static_cast<UINT>(uiHeight * fScale), WICBitmapInterpolationModeCubic);
			if (FAILED(hr)) return false;

			pWicCurrentSource = pWicBmpScaler;
		}

		if (rotation != ERotation::None)
		{
			WICBitmapTransformOptions ulRotation = WICBitmapTransformRotate0;
			switch (rotation)
			{
			case ERotation::Deg90:  ulRotation = WICBitmapTransformRotate90;  break;
			case ERotation::Deg180: ulRotation = WICBitmapTransformRotate180; break;
			case ERotation::Deg270: ulRotation = WICBitmapTransformRotate270; break;
			default: break;
			}

			CComPtr<IWICBitmapFlipRotator> pWicFlipRotator;
			hr = pWicImageFactory->CreateBitmapFlipRotator(&pWicFlipRotator);
			if (FAILED(hr)) return false;

			hr = pWicFlipRotator->Initialize(pWicCurrentSource, ulRotation);
			if (FAILED(hr)) return false;

			pWicCurrentSource = pWicFlipRotator;
		}

		CComPtr<IWICFormatConverter> pWicFormatConverter;
		hr = pWicImageFactory->CreateFormatConverter(&pWicFormatConverter);
		if (FAILED(hr)) return false;

		hr = pWicFormatConverter->Initialize(
			pWicCurrentSource,
			GUID_WICPixelFormat32bppBGRA,
			WICBitmapDitherTypeNone,
			nullptr,
			0.f,
			WICBitmapPaletteTypeCustom);
		if (FAILED(hr)) return false;

		hr = pWicImageFactory->CreateBitmapFromSource(pWicFormatConverter, WICBitmapCacheOnDemand, pOutWicBitmap);

		return SUCCEEDED(hr);
	}

	static bool SaveRgb32ImageWithoutExplicitMetadata(
		const wchar_t* filePath,
		unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels,
		bool hasAlpha, EOutputType eOutputType)
	{
		if (pixels == nullptr)return false;

		CComPtr<IWICImagingFactory> pWicImagingFactory;
		HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImagingFactory));
		if (FAILED(hr))return false;

		CComPtr<IWICBitmapEncoder> pWicBitmapEncoder;
		hr = pWicImagingFactory->CreateEncoder(ToWicGuid(eOutputType), nullptr, &pWicBitmapEncoder);
		if (FAILED(hr))return false;

		CComPtr<IWICStream> pWicStream;
		hr = pWicImagingFactory->CreateStream(&pWicStream);
		if (FAILED(hr))return false;

		hr = pWicStream->InitializeFromFilename(filePath, GENERIC_WRITE);
		if (FAILED(hr))return false;

		hr = pWicBitmapEncoder->Initialize(pWicStream, WICBitmapEncoderCacheOption::WICBitmapEncoderNoCache);
		if (FAILED(hr))return false;

		CComPtr<IWICBitmapFrameEncode> pWicBitmapFrameEncode;
		CComPtr<IPropertyBag2> pPropertyBag;
		hr = pWicBitmapEncoder->CreateNewFrame(&pWicBitmapFrameEncode, &pPropertyBag);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->Initialize(pPropertyBag);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->SetSize(width, height);
		if (FAILED(hr))return false;

		CComPtr<IWICBitmap> pWicBitmap;
		hr = pWicImagingFactory->CreateBitmapFromMemory
		(
			width,
			height,
			hasAlpha ? GUID_WICPixelFormat32bppRGBA : GUID_WICPixelFormat32bppRGB,
			stride,
			stride * height,
			pixels,
			&pWicBitmap
		);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->WriteSource(pWicBitmap, nullptr);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->Commit();
		if (FAILED(hr))return false;

		hr = pWicBitmapEncoder->Commit();

		return SUCCEEDED(hr);
	}
}

bool win_image::LoadImageToMemory(const wchar_t* filePath, SImageFrame* pImageFrame, float fScale, ERotation rotation)
{
	if (pImageFrame == nullptr)return false;

	SImageFrame* const s = pImageFrame;

	CComPtr<IWICBitmap> pWicBitmap;
	if (!LoadImageWithTransformOption(filePath, &pWicBitmap, fScale, rotation) || pWicBitmap == nullptr)
		return false;

	pWicBitmap->GetSize(&s->uiWidth, &s->uiHeight);

	CComPtr<IWICBitmapLock> pWicBitmapLock;
	WICRect wicRect{ 0, 0, static_cast<INT>(s->uiWidth), static_cast<INT>(s->uiHeight) };
	HRESULT hr = pWicBitmap->Lock(&wicRect, WICBitmapLockRead, &pWicBitmapLock);
	if (FAILED(hr))return false;

	hr = pWicBitmapLock->GetStride(&s->uiStride);
	if (FAILED(hr))return false;

	s->pixels.resize(static_cast<size_t>(s->uiStride * s->uiHeight));
	hr = pWicBitmap->CopyPixels(nullptr, s->uiStride, static_cast<UINT>(s->pixels.size()), s->pixels.data());
	if (FAILED(hr))return false;

	return true;
}

bool win_image::LoadImageToWicBitmap(const wchar_t* filePath, void** pWicBitmap, float fScale, ERotation rotation)
{
	IWICBitmap** p = reinterpret_cast<IWICBitmap**>(pWicBitmap);
	return LoadImageWithTransformOption(filePath, p, fScale, rotation);
}

bool win_image::SkimImageSize(const wchar_t* filePath, unsigned int* width, unsigned int* height)
{
	if (width == nullptr || height == nullptr)return false;

	CComPtr<IWICImagingFactory> pWicImageFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
	hr = pWicImageFactory->CreateDecoderFromFilename(filePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pWicBitmapDecoder);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapFrameDecode> pWicFrameDecode;
	hr = pWicBitmapDecoder->GetFrame(0, &pWicFrameDecode);
	if (FAILED(hr))return false;

	hr = pWicFrameDecode->GetSize(width, height);
	return SUCCEEDED(hr);
}

bool win_image::SaveImageAsPng(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha)
{
	return SaveRgb32ImageWithoutExplicitMetadata(filePath, width, height, stride, pixels, hasAlpha, EOutputType::Png);
}

bool win_image::SaveImageAsJpg(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha)
{
	return SaveRgb32ImageWithoutExplicitMetadata(filePath, width, height, stride, pixels, hasAlpha, EOutputType::Jpg);
}


namespace win_image
{
	class CWicGifEncoder::Impl
	{
	public:
		bool Initialise(const wchar_t* filePath);
		bool HasBeenInitialised() const { return m_hasBeenInitialised; }

		bool CommitFrame(unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha, unsigned short delay);

		bool Finalise();
	private:
		CComPtr<IWICImagingFactory> m_pWicImagingFactory;
		CComPtr<IWICBitmapEncoder> m_pWicBitmapEncoder;
		CComPtr<IWICStream> m_pWicStream;

		bool m_hasBeenInitialised = false;
	};


	bool CWicGifEncoder::Impl::Initialise(const wchar_t* filePath)
	{
		HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWicImagingFactory));
		if (FAILED(hr))return false;

		hr = m_pWicImagingFactory->CreateEncoder(GUID_ContainerFormatGif, nullptr, &m_pWicBitmapEncoder);
		if (FAILED(hr))return false;

		hr = m_pWicImagingFactory->CreateStream(&m_pWicStream);
		if (FAILED(hr))return false;

		hr = m_pWicStream->InitializeFromFilename(filePath, GENERIC_WRITE);
		if (FAILED(hr))return false;

		hr = m_pWicBitmapEncoder->Initialize(m_pWicStream, WICBitmapEncoderCacheOption::WICBitmapEncoderNoCache);
		if (FAILED(hr))return false;

		CComPtr<IWICMetadataQueryWriter> pWicMetadataQueryWriter;
		hr = m_pWicBitmapEncoder->GetMetadataQueryWriter(&pWicMetadataQueryWriter);
		if (FAILED(hr))return false;

		const auto SetApplicationMetadata = [&pWicMetadataQueryWriter]()
			-> bool
			{
				PROPVARIANT sPropVariant{};
				sPropVariant.vt = VT_UI1 | VT_VECTOR;
				char szName[] = "NETSCAPE2.0";
				sPropVariant.cac.cElems = sizeof(szName) - 1;
				sPropVariant.cac.pElems = szName;
				HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/appext/application", &sPropVariant);
				return SUCCEEDED(hr);
			};
		const auto SetDataMetadata = [&pWicMetadataQueryWriter]()
			-> bool
			{
				PROPVARIANT sPropVariant{};
				sPropVariant.vt = VT_UI1 | VT_VECTOR;
				char szLoopCount[] = { 0x03, 0x01, 0x00, 0x00, 0x00 };
				sPropVariant.cac.cElems = sizeof(szLoopCount);
				sPropVariant.cac.pElems = szLoopCount;
				HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/appext/data", &sPropVariant);
				return SUCCEEDED(hr);
			};
		m_hasBeenInitialised = SetApplicationMetadata() && SetDataMetadata();

		return m_hasBeenInitialised;
	}

	bool CWicGifEncoder::Impl::CommitFrame(unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha, unsigned short delay)
	{
		CComPtr<IWICBitmapFrameEncode> pWicBitmapFrameEncode;
		CComPtr<IPropertyBag2> pPropertyBag;
		HRESULT hr = m_pWicBitmapEncoder->CreateNewFrame(&pWicBitmapFrameEncode, &pPropertyBag);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->Initialize(pPropertyBag);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->SetSize(width, height);
		if (FAILED(hr))return false;

		CComPtr<IWICBitmap> pWicBitmap;
		hr = m_pWicImagingFactory->CreateBitmapFromMemory
		(
			width,
			height,
			hasAlpha ? GUID_WICPixelFormat32bppRGBA : GUID_WICPixelFormat32bppRGB,
			stride,
			stride * height,
			pixels,
			&pWicBitmap
		);
		if (FAILED(hr))return false;

		CComPtr<IWICMetadataQueryWriter> pWicMetadataQueryWriter;
		hr = pWicBitmapFrameEncode->GetMetadataQueryWriter(&pWicMetadataQueryWriter);
		if (FAILED(hr))return false;

		const auto SetDelayMetadata = [&pWicMetadataQueryWriter, &delay]()
			-> bool
			{
				PROPVARIANT sPropVariant{};
				sPropVariant.vt = VT_UI2;
				sPropVariant.uiVal = delay;
				HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/grctlext/Delay", &sPropVariant);
				return SUCCEEDED(hr);
			};

		const auto SetDisposalMetaData = [&pWicMetadataQueryWriter]()
			-> bool
			{
				PROPVARIANT sPropVariant{};
				sPropVariant.vt = VT_UI1;
				sPropVariant.bVal = 2;
				HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/grctlext/Disposal", &sPropVariant);
				return SUCCEEDED(hr);
			};
		const auto SetTransparencyFlag = [&pWicMetadataQueryWriter]()
			-> bool
			{
				PROPVARIANT sPropVariant{};
				sPropVariant.vt = VT_BOOL;
				sPropVariant.boolVal = 1;
				HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/grctlext/TransparencyFlag", &sPropVariant);
				return SUCCEEDED(hr);
			};
		const auto SetTransparentColorIndex = [&pWicMetadataQueryWriter]()
			-> bool
			{
				PROPVARIANT sPropVariant{};
				sPropVariant.vt = VT_UI1;
				sPropVariant.bVal = 255;
				HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/grctlext/TransparentColorIndex", &sPropVariant);
				return SUCCEEDED(hr);
			};

		SetDisposalMetaData() && SetTransparencyFlag() && SetDelayMetadata() && SetTransparentColorIndex();

		hr = pWicBitmapFrameEncode->WriteSource(pWicBitmap, nullptr);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->Commit();

		return SUCCEEDED(hr);
	}

	bool CWicGifEncoder::Impl::Finalise()
	{
		HRESULT hr = m_pWicBitmapEncoder->Commit();

		if (SUCCEEDED(hr))
		{
			m_hasBeenInitialised = false;
		}

		return SUCCEEDED(hr);
	}


	CWicGifEncoder::CWicGifEncoder()  : m_pImpl(std::make_unique<Impl>()) {}
	CWicGifEncoder::~CWicGifEncoder() = default;

	bool CWicGifEncoder::Initialise(const wchar_t* filePath)
	{
		return m_pImpl->Initialise(filePath);
	}

	bool CWicGifEncoder::HasBeenInitialised() const
	{
		return m_pImpl->HasBeenInitialised();
	}

	bool CWicGifEncoder::CommitFrame(unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha, float delayInSeconds)
	{
		unsigned short delayInHundredths = static_cast<unsigned short>(delayInSeconds * 100.f);
		return m_pImpl->CommitFrame(width, height, stride, pixels, hasAlpha, delayInHundredths == 0 ? 1 : delayInHundredths);
	}

	bool CWicGifEncoder::Finalise()
	{
		return m_pImpl->Finalise();
	}
}
