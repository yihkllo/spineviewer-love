#include "d3d11_texture.h"

#include <Windows.h>
#include <chrono>
#include <cstdio>
#include <cwchar>
#include <iterator>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <wincodec.h>
#include <wrl/client.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma comment(lib, "windowscodecs.lib")

namespace sl_d3d11 {
namespace {

bool ReadFileBytes(const wchar_t* path, std::vector<unsigned char>& outBytes, std::string* outError)
{
	outBytes.clear();
	if (path == nullptr || path[0] == L'\0')
	{
		if (outError) *outError = "empty texture path";
		return false;
	}

	FILE* file = nullptr;
	if (_wfopen_s(&file, path, L"rb") != 0 || file == nullptr)
	{
		if (outError) *outError = "failed to open texture file";
		return false;
	}

	if (std::fseek(file, 0, SEEK_END) != 0)
	{
		std::fclose(file);
		if (outError) *outError = "failed to seek texture file";
		return false;
	}
	const long size = std::ftell(file);
	if (size <= 0)
	{
		std::fclose(file);
		if (outError) *outError = "empty texture file";
		return false;
	}
	std::rewind(file);

	outBytes.resize(static_cast<size_t>(size));
	const size_t read = std::fread(outBytes.data(), 1, outBytes.size(), file);
	std::fclose(file);
	if (read != outBytes.size())
	{
		outBytes.clear();
		if (outError) *outError = "failed to read texture file";
		return false;
	}
	return true;
}

bool CreateTextureFromRgba(ID3D11Device* device, const unsigned char* pixels, int width, int height, D3D11Texture& outTexture, bool generateMips = false)
{
	if (device == nullptr || pixels == nullptr || width <= 0 || height <= 0)
		return false;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = static_cast<UINT>(width);
	desc.Height = static_cast<UINT>(height);
	desc.MipLevels = generateMips ? 0 : 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (generateMips)
	{
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	HRESULT hr = S_OK;
	if (generateMips)
	{
		hr = device->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
	}
	else
	{
		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = pixels;
		data.SysMemPitch = static_cast<UINT>(width * 4);
		hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
	}
	if (FAILED(hr))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = generateMips ? static_cast<UINT>(-1) : 1;
	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, outTexture.srv.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return false;

	if (generateMips)
	{
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
		device->GetImmediateContext(context.GetAddressOf());
		if (!context)
			return false;
		context->UpdateSubresource(texture.Get(), 0, nullptr, pixels, static_cast<UINT>(width * 4), 0);
		context->GenerateMips(outTexture.srv.Get());
	}

	outTexture.texture = texture;
	outTexture.width = width;
	outTexture.height = height;
	return true;
}

unsigned char* LoadPixelsViaWic(const wchar_t* path, int& outWidth, int& outHeight)
{
	using Microsoft::WRL::ComPtr;

	ComPtr<IWICImagingFactory> factory;
	HRESULT hr = ::CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(factory.GetAddressOf()));
	if (FAILED(hr))
		return nullptr;

	ComPtr<IWICBitmapDecoder> decoder;
	hr = factory->CreateDecoderFromFilename(
		path,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		decoder.GetAddressOf());
	if (FAILED(hr))
		return nullptr;

	ComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, frame.GetAddressOf());
	if (FAILED(hr))
		return nullptr;

	ComPtr<IWICFormatConverter> converter;
	hr = factory->CreateFormatConverter(converter.GetAddressOf());
	if (FAILED(hr))
		return nullptr;

	hr = converter->Initialize(
		frame.Get(),
		GUID_WICPixelFormat32bppRGBA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0,
		WICBitmapPaletteTypeCustom);
	if (FAILED(hr))
		return nullptr;

	UINT width = 0;
	UINT height = 0;
	hr = converter->GetSize(&width, &height);
	if (FAILED(hr) || width == 0 || height == 0)
		return nullptr;

	const UINT stride = width * 4;
	const UINT byteCount = stride * height;
	unsigned char* pixels = new unsigned char[byteCount];
	hr = converter->CopyPixels(nullptr, stride, byteCount, pixels);
	if (FAILED(hr))
	{
		delete[] pixels;
		return nullptr;
	}

	outWidth = static_cast<int>(width);
	outHeight = static_cast<int>(height);
	return pixels;
}

bool TryApplyUnitySplitAlpha(const wchar_t* path, unsigned char* pixels, int width, int height)
{
	if (path == nullptr || pixels == nullptr || width <= 0 || height <= 0)
		return false;

	std::wstring source(path);
	const size_t extension = source.find_last_of(L'.');
	if (extension == std::wstring::npos)
		return false;
	const std::wstring stem = source.substr(0, extension);
	if (stem.size() >= 6 && _wcsicmp(stem.c_str() + stem.size() - 6, L"_alpha") == 0)
		return false;

	const std::wstring alphaPath = stem + L"_alpha" + source.substr(extension);
	if (::GetFileAttributesW(alphaPath.c_str()) == INVALID_FILE_ATTRIBUTES)
		return false;

	std::vector<unsigned char> alphaBytes;
	if (!ReadFileBytes(alphaPath.c_str(), alphaBytes, nullptr))
		return false;
	int alphaWidth = 0;
	int alphaHeight = 0;
	int alphaChannels = 0;
	unsigned char* alphaPixels = stbi_load_from_memory(alphaBytes.data(),
		static_cast<int>(alphaBytes.size()), &alphaWidth, &alphaHeight, &alphaChannels, STBI_rgb_alpha);
	if (alphaPixels == nullptr || alphaWidth != width || alphaHeight != height)
	{
		if (alphaPixels != nullptr) stbi_image_free(alphaPixels);
		return false;
	}

	const int pixelCount = width * height;
	for (int i = 0; i < pixelCount; ++i)
		pixels[i * 4 + 3] = alphaPixels[i * 4];
	stbi_image_free(alphaPixels);
	return true;
}

}

namespace {

struct PixelRelease
{
	bool stb = true;
	void operator()(unsigned char* pixels) const noexcept
	{
		if (stb) stbi_image_free(pixels);
		else delete[] pixels;
	}
};

struct DecodedPixels
{
	std::unique_ptr<unsigned char, PixelRelease> pixels;
	int width = 0;
	int height = 0;
	FILETIME written{};
	std::chrono::steady_clock::time_point parked;
};

std::mutex g_prefetchMutex;
std::unordered_map<std::wstring, DecodedPixels> g_prefetched;
constexpr auto kPrefetchLifetime = std::chrono::seconds(30);

std::wstring PrefetchKey(const wchar_t* path)
{
	wchar_t full[MAX_PATH * 4];
	const DWORD length = ::GetFullPathNameW(path, static_cast<DWORD>(std::size(full)), full, nullptr);
	std::wstring key = length > 0 && length < std::size(full) ? std::wstring(full, length) : std::wstring(path);
	if (!key.empty())
		::CharLowerBuffW(key.data(), static_cast<DWORD>(key.size()));
	return key;
}

bool LastWriteTime(const wchar_t* path, FILETIME& outTime)
{
	WIN32_FILE_ATTRIBUTE_DATA data{};
	if (!::GetFileAttributesExW(path, GetFileExInfoStandard, &data))
		return false;
	outTime = data.ftLastWriteTime;
	return true;
}

bool DecodeTexturePixels(const wchar_t* path, DecodedPixels& out, std::string* outError)
{
	std::vector<unsigned char> fileBytes;
	if (!ReadFileBytes(path, fileBytes, outError))
		return false;

	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* pixels = stbi_load_from_memory(fileBytes.data(), static_cast<int>(fileBytes.size()), &width, &height, &channels, STBI_rgb_alpha);
	const bool usingStb = pixels != nullptr;
	if (!usingStb)
	{
		pixels = LoadPixelsViaWic(path, width, height);
		if (pixels == nullptr)
		{
			if (outError) *outError = stbi_failure_reason() ? stbi_failure_reason() : "texture decode failed";
			return false;
		}
	}

	TryApplyUnitySplitAlpha(path, pixels, width, height);

	out.pixels = std::unique_ptr<unsigned char, PixelRelease>(pixels, PixelRelease{ usingStb });
	out.width = width;
	out.height = height;
	return true;
}

bool TakePrefetchedPixels(const wchar_t* path, DecodedPixels& out)
{
	FILETIME written{};
	const bool known = LastWriteTime(path, written);
	const auto key = PrefetchKey(path);
	std::lock_guard<std::mutex> lock(g_prefetchMutex);
	const auto found = g_prefetched.find(key);
	if (found == g_prefetched.end())
		return false;
	const bool current = known && ::CompareFileTime(&found->second.written, &written) == 0;
	if (current)
		out = std::move(found->second);
	g_prefetched.erase(found);
	return current;
}

}

bool PrefetchTexturePixels(const wchar_t* path)
{
	DecodedPixels decoded;
	if (path == nullptr || !LastWriteTime(path, decoded.written) || !DecodeTexturePixels(path, decoded, nullptr))
		return false;
	decoded.parked = std::chrono::steady_clock::now();
	const auto key = PrefetchKey(path);
	std::lock_guard<std::mutex> lock(g_prefetchMutex);
	for (auto it = g_prefetched.begin(); it != g_prefetched.end();)
		it = decoded.parked - it->second.parked > kPrefetchLifetime ? g_prefetched.erase(it) : std::next(it);
	g_prefetched[key] = std::move(decoded);
	return true;
}

bool LoadTextureFromFile(ID3D11Device* device, const wchar_t* path, D3D11Texture& outTexture, bool premultiplyAlpha, std::string* outError, bool generateMips)
{
	DecodedPixels decoded;
	if (!TakePrefetchedPixels(path, decoded) && !DecodeTexturePixels(path, decoded, outError))
		return false;
	unsigned char* pixels = decoded.pixels.get();
	const int width = decoded.width;
	const int height = decoded.height;

	if (premultiplyAlpha)
	{
		const int pixelCount = width * height;
		for (int i = 0; i < pixelCount; ++i)
		{
			unsigned char* pixel = pixels + i * 4;
			const unsigned int alpha = pixel[3];
			pixel[0] = static_cast<unsigned char>((static_cast<unsigned int>(pixel[0]) * alpha + 127u) / 255u);
			pixel[1] = static_cast<unsigned char>((static_cast<unsigned int>(pixel[1]) * alpha + 127u) / 255u);
			pixel[2] = static_cast<unsigned char>((static_cast<unsigned int>(pixel[2]) * alpha + 127u) / 255u);
		}
	}

	const bool ok = CreateTextureFromRgba(device, pixels, width, height, outTexture, generateMips);
	if (!ok && outError)
		*outError = "CreateTexture2D failed";
	return ok;
}

bool CreateSolidTexture(ID3D11Device* device, unsigned char r, unsigned char g, unsigned char b, unsigned char a, D3D11Texture& outTexture)
{
	const unsigned char pixel[] = { r, g, b, a };
	return CreateTextureFromRgba(device, pixel, 1, 1, outTexture);
}

}
