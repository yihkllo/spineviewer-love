#ifndef SPINELOVE_RENDER_D3D11_RENDERER_H_
#define SPINELOVE_RENDER_D3D11_RENDERER_H_

#include <d3d11.h>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <wrl/client.h>

#include "../sl_gfx_draw.h"
#include "../sl_gfx_types.h"
#include "d3d11_texture.h"

namespace sl_d3d11 {

class D3D11Renderer
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* shaderPath);
	void ClearCurrentTarget(const SlVec4& clearColor);
	void BeginFrame(int width, int height);
	void EndFrame();
	void DrawSprite(SlTextureId texture, const SlRect& dst, const SlRect& uv, const SlColor& tint);
	void DrawTriangles(SlTextureId texture, const SlVertex2D* vertices, int vertexCount, const unsigned short* indices, int indexCount, SlBlendMode blendMode, bool premultipliedAlpha = false);
	void Submit(const SlDrawList& drawList, const std::unordered_map<std::uint64_t, SlTextureId>& textureMap);
	SlTextureId LoadTexture(const wchar_t* path, bool premultiplyAlpha = false);
	void ReleaseTexture(SlTextureId texture) noexcept;
	void* GetTextureSrv(SlTextureId texture) const noexcept;
	bool GetTextureSize(SlTextureId texture, int& outWidth, int& outHeight) const noexcept;
	bool ReadTexturePixels(SlTextureId texture, std::vector<unsigned char>& outRgba, int& outWidth, int& outHeight, int& outStride);
	bool RasterizeToPixels(SlTextureId texture, const SlVertex2D* vertices, int vertexCount, const unsigned short* indices, int indexCount,
		int width, int height, bool premultipliedAlpha, std::vector<unsigned char>& outRgba, int& outStride);
	SlTextureId CreateFallbackTexture();
	SlTextureId CreateRenderTarget(int width, int height);
	bool BeginRenderTarget(SlTextureId renderTarget, const SlVec4& clearColor);

private:
	struct TextureSlot
	{
		D3D11Texture texture;
	};

	struct RenderTargetSlot
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
		int width = 0;
		int height = 0;
	};

	bool CreateShaders(const wchar_t* shaderPath);
	bool CreateStates();
	bool CreateBlendState(D3D11_BLEND srcColor, D3D11_BLEND dstColor, D3D11_BLEND srcAlpha, D3D11_BLEND dstAlpha, ID3D11BlendState** outState);
	bool EnsureVertexBuffer(size_t vertexCount);
	bool EnsureIndexBuffer(size_t indexCount);
	void ApplyBlendMode(SlBlendMode blendMode, bool premultipliedAlpha);

	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_context = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_viewBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaBlend;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_pmaAlphaBlend;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_addBlend;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_pmaAddBlend;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_multiplyBlend;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_pmaMultiplyBlend;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_screenBlend;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizer;
	std::unordered_map<SlTextureId, TextureSlot> m_textures;
	std::unordered_map<SlTextureId, RenderTargetSlot> m_renderTargets;
	SlTextureId m_nextTextureId = 1;
	size_t m_vertexBufferCapacity = 0;
	size_t m_indexBufferCapacity = 0;
};

}

#endif
