#ifndef SPINELOVE_RENDER_D3D11_TEXTURE_H_
#define SPINELOVE_RENDER_D3D11_TEXTURE_H_

#include <d3d11.h>
#include <string>
#include <wrl/client.h>

namespace sl_d3d11 {

struct D3D11Texture
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	int width = 0;
	int height = 0;
};

bool LoadTextureFromFile(ID3D11Device* device, const wchar_t* path, D3D11Texture& outTexture, bool premultiplyAlpha = false, std::string* outError = nullptr);
bool CreateSolidTexture(ID3D11Device* device, unsigned char r, unsigned char g, unsigned char b, unsigned char a, D3D11Texture& outTexture);

}

#endif
