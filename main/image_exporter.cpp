#include "image_exporter.h"

#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace {

using Microsoft::WRL::ComPtr;

class ComScope
{
public:
	ComScope()
	{
		const HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		m_shouldUninitialize = SUCCEEDED(hr);
		m_ready = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;
	}

	~ComScope()
	{
		if (m_shouldUninitialize)
			::CoUninitialize();
	}

	bool Ready() const noexcept { return m_ready; }

private:
	bool m_ready = false;
	bool m_shouldUninitialize = false;
};

unsigned char ToByte(float value) noexcept
{
	if (!std::isfinite(value))
		return 0;
	const float clamped = (std::max)(0.0f, (std::min)(1.0f, value));
	return static_cast<unsigned char>(clamped * 255.0f + 0.5f);
}

unsigned char UnpackStraightColor(unsigned char value, unsigned char alpha) noexcept
{
	if (alpha == 0)
		return 0;
	const int expanded = static_cast<int>(value) * 255 + alpha / 2;
	return static_cast<unsigned char>((std::min)(255, expanded / alpha));
}

std::vector<unsigned char> MakePngPixels(const unsigned char* rgba, int width, int height, int stride, bool keepAlpha, const SlColor& matteColor)
{
	const int bgR = ToByte(matteColor.r);
	const int bgG = ToByte(matteColor.g);
	const int bgB = ToByte(matteColor.b);

	std::vector<unsigned char> bgra(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
	for (int y = 0; y < height; ++y)
	{
		const unsigned char* src = rgba + static_cast<size_t>(y) * stride;
		unsigned char* dst = bgra.data() + static_cast<size_t>(y) * width * 4;
		for (int x = 0; x < width; ++x)
		{
			const unsigned char r = src[x * 4 + 0];
			const unsigned char g = src[x * 4 + 1];
			const unsigned char b = src[x * 4 + 2];
			const unsigned char a = src[x * 4 + 3];
			if (keepAlpha)
			{
				dst[x * 4 + 0] = UnpackStraightColor(b, a);
				dst[x * 4 + 1] = UnpackStraightColor(g, a);
				dst[x * 4 + 2] = UnpackStraightColor(r, a);
				dst[x * 4 + 3] = a;
			}
			else
			{
				const int invA = 255 - a;
				dst[x * 4 + 0] = static_cast<unsigned char>((std::min)(255, static_cast<int>(b) + (bgB * invA + 127) / 255));
				dst[x * 4 + 1] = static_cast<unsigned char>((std::min)(255, static_cast<int>(g) + (bgG * invA + 127) / 255));
				dst[x * 4 + 2] = static_cast<unsigned char>((std::min)(255, static_cast<int>(r) + (bgR * invA + 127) / 255));
				dst[x * 4 + 3] = 255;
			}
		}
	}
	return bgra;
}

std::vector<unsigned char> MakeJpegPixels(const unsigned char* rgba, int width, int height, int stride, const SlColor& background)
{
	const int bgR = ToByte(background.r);
	const int bgG = ToByte(background.g);
	const int bgB = ToByte(background.b);

	std::vector<unsigned char> bgr(static_cast<size_t>(width) * static_cast<size_t>(height) * 3);
	for (int y = 0; y < height; ++y)
	{
		const unsigned char* src = rgba + static_cast<size_t>(y) * stride;
		unsigned char* dst = bgr.data() + static_cast<size_t>(y) * width * 3;
		for (int x = 0; x < width; ++x)
		{
			const int a = src[x * 4 + 3];
			const int invA = 255 - a;
			const int r = (std::min)(255, static_cast<int>(src[x * 4 + 0]) + (bgR * invA + 127) / 255);
			const int g = (std::min)(255, static_cast<int>(src[x * 4 + 1]) + (bgG * invA + 127) / 255);
			const int b = (std::min)(255, static_cast<int>(src[x * 4 + 2]) + (bgB * invA + 127) / 255);
			dst[x * 3 + 0] = static_cast<unsigned char>(b);
			dst[x * 3 + 1] = static_cast<unsigned char>(g);
			dst[x * 3 + 2] = static_cast<unsigned char>(r);
		}
	}
	return bgr;
}

void SetError(std::wstring* errorMessage, const wchar_t* message)
{
	if (errorMessage != nullptr)
		*errorMessage = message ? message : L"Export failed.";
}

bool WriteWithWic(const wchar_t* path, SlExportImageFormat format, const unsigned char* pixels, int width, int height, int stride, std::wstring* errorMessage)
{
	ComScope com;
	if (!com.Ready())
	{
		SetError(errorMessage, L"Windows image service is not available.");
		return false;
	}

	ComPtr<IWICImagingFactory> factory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));
	if (FAILED(hr))
	{
		SetError(errorMessage, L"Could not start image encoder.");
		return false;
	}

	const GUID container = format == SlExportImageFormat::Png ? GUID_ContainerFormatPng : GUID_ContainerFormatJpeg;
	ComPtr<IWICBitmapEncoder> encoder;
	hr = factory->CreateEncoder(container, nullptr, encoder.GetAddressOf());
	if (FAILED(hr))
	{
		SetError(errorMessage, L"Could not create image encoder.");
		return false;
	}

	ComPtr<IWICStream> stream;
	hr = factory->CreateStream(stream.GetAddressOf());
	if (FAILED(hr) || FAILED(stream->InitializeFromFilename(path, GENERIC_WRITE)))
	{
		SetError(errorMessage, L"Could not open export file.");
		return false;
	}

	if (FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache)))
	{
		SetError(errorMessage, L"Could not initialize export file.");
		return false;
	}

	ComPtr<IWICBitmapFrameEncode> frame;
	ComPtr<IPropertyBag2> properties;
	if (FAILED(encoder->CreateNewFrame(frame.GetAddressOf(), properties.GetAddressOf())))
	{
		SetError(errorMessage, L"Could not create image frame.");
		return false;
	}

	if (FAILED(frame->Initialize(properties.Get())) ||
		FAILED(frame->SetSize(static_cast<UINT>(width), static_cast<UINT>(height))))
	{
		SetError(errorMessage, L"Could not prepare image frame.");
		return false;
	}

	WICPixelFormatGUID pixelFormat = format == SlExportImageFormat::Png
		? GUID_WICPixelFormat32bppBGRA
		: GUID_WICPixelFormat24bppBGR;
	if (FAILED(frame->SetPixelFormat(&pixelFormat)))
	{
		SetError(errorMessage, L"Could not set image pixel format.");
		return false;
	}

	if (FAILED(frame->WritePixels(static_cast<UINT>(height), static_cast<UINT>(stride),
		static_cast<UINT>(stride * height), const_cast<BYTE*>(pixels))) ||
		FAILED(frame->Commit()) ||
		FAILED(encoder->Commit()))
	{
		SetError(errorMessage, L"Could not write image pixels.");
		return false;
	}

	return true;
}

}

bool SaveExportImage(
	const wchar_t* path,
	SlExportImageFormat format,
	const unsigned char* premultipliedRgba,
	int width,
	int height,
	int stride,
	bool keepAlpha,
	const SlColor& matteColor,
	std::wstring* errorMessage)
{
	if (path == nullptr || path[0] == L'\0')
	{
		SetError(errorMessage, L"Export path is empty.");
		return false;
	}
	if (premultipliedRgba == nullptr || width <= 0 || height <= 0 || stride < width * 4)
	{
		SetError(errorMessage, L"Nothing is available to export.");
		return false;
	}

	if (format == SlExportImageFormat::Png)
	{
		std::vector<unsigned char> pixels = MakePngPixels(premultipliedRgba, width, height, stride, keepAlpha, matteColor);
		return WriteWithWic(path, format, pixels.data(), width, height, width * 4, errorMessage);
	}

	std::vector<unsigned char> pixels = MakeJpegPixels(premultipliedRgba, width, height, stride, matteColor);
	return WriteWithWic(path, format, pixels.data(), width, height, width * 3, errorMessage);
}
