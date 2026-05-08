#include "d3d11_texture.h"

#include <Windows.h>
#include <cstdio>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

bool CreateTextureFromRgba(ID3D11Device* device, const unsigned char* pixels, int width, int height, D3D11Texture& outTexture)
{
	if (device == nullptr || pixels == nullptr || width <= 0 || height <= 0)
		return false;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = static_cast<UINT>(width);
	desc.Height = static_cast<UINT>(height);
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = pixels;
	data.SysMemPitch = static_cast<UINT>(width * 4);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	HRESULT hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
	if (FAILED(hr))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, outTexture.srv.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return false;

	outTexture.texture = texture;
	outTexture.width = width;
	outTexture.height = height;
	return true;
}

}

bool LoadTextureFromFile(ID3D11Device* device, const wchar_t* path, D3D11Texture& outTexture, bool premultiplyAlpha, std::string* outError)
{
	std::vector<unsigned char> fileBytes;
	if (!ReadFileBytes(path, fileBytes, outError))
		return false;

	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* pixels = stbi_load_from_memory(fileBytes.data(), static_cast<int>(fileBytes.size()), &width, &height, &channels, STBI_rgb_alpha);
	if (pixels == nullptr)
	{
		if (outError) *outError = stbi_failure_reason() ? stbi_failure_reason() : "stb_image failed";
		return false;
	}

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

	const bool ok = CreateTextureFromRgba(device, pixels, width, height, outTexture);
	stbi_image_free(pixels);
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
