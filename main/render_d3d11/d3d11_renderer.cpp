#include "d3d11_renderer.h"

#include <d3dcompiler.h>
#include <cstring>
#include <vector>

namespace sl_d3d11 {
namespace {

struct ViewConstants
{
	float viewport[2];
	float padding[2];
};

}

bool D3D11Renderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* shaderPath)
{
	m_device = device;
	m_context = context;
	if (m_device == nullptr || m_context == nullptr)
		return false;

	return CreateShaders(shaderPath) && CreateStates() && CreateFallbackTexture() != 0;
}

bool D3D11Renderer::CreateShaders(const wchar_t* shaderPath)
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3DCompileFromFile(shaderPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VSMain", "vs_4_0", flags, 0, vertexBlob.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr))
		return false;
	hr = D3DCompileFromFile(shaderPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PSMain", "ps_4_0", flags, 0, pixelBlob.GetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return false;

	hr = m_device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
	if (FAILED(hr))
		return false;
	hr = m_device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
	if (FAILED(hr))
		return false;

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = m_device->CreateInputLayout(layout, static_cast<UINT>(sizeof(layout) / sizeof(layout[0])),
		vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), m_inputLayout.GetAddressOf());
	if (FAILED(hr))
		return false;

	D3D11_BUFFER_DESC viewDesc{};
	viewDesc.ByteWidth = sizeof(ViewConstants);
	viewDesc.Usage = D3D11_USAGE_DYNAMIC;
	viewDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	viewDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_device->CreateBuffer(&viewDesc, nullptr, m_viewBuffer.GetAddressOf());
	return SUCCEEDED(hr);
}

bool D3D11Renderer::CreateStates()
{
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(m_device->CreateSamplerState(&samplerDesc, m_sampler.GetAddressOf())))
		return false;

	if (!CreateBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, m_alphaBlend.GetAddressOf()))
		return false;
	if (!CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, m_pmaAlphaBlend.GetAddressOf()))
		return false;
	if (!CreateBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, m_addBlend.GetAddressOf()))
		return false;
	if (!CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, m_pmaAddBlend.GetAddressOf()))
		return false;
	if (!CreateBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, m_multiplyBlend.GetAddressOf()))
		return false;
	if (!CreateBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, m_pmaMultiplyBlend.GetAddressOf()))
		return false;
	if (!CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, m_screenBlend.GetAddressOf()))
		return false;

	D3D11_RASTERIZER_DESC rasterDesc{};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthClipEnable = TRUE;
	return SUCCEEDED(m_device->CreateRasterizerState(&rasterDesc, m_rasterizer.GetAddressOf()));
}

bool D3D11Renderer::CreateBlendState(D3D11_BLEND srcColor, D3D11_BLEND dstColor, D3D11_BLEND srcAlpha, D3D11_BLEND dstAlpha, ID3D11BlendState** outState)
{
	D3D11_BLEND_DESC desc{};
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].SrcBlend = srcColor;
	desc.RenderTarget[0].DestBlend = dstColor;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = srcAlpha;
	desc.RenderTarget[0].DestBlendAlpha = dstAlpha;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	return SUCCEEDED(m_device->CreateBlendState(&desc, outState));
}

void D3D11Renderer::ClearCurrentTarget(const SlVec4& clearColor)
{
	if (m_context == nullptr)
		return;

	ID3D11RenderTargetView* target = nullptr;
	m_context->OMGetRenderTargets(1, &target, nullptr);
	if (target == nullptr)
		return;

	const FLOAT color[] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	m_context->ClearRenderTargetView(target, color);
	target->Release();
}

void D3D11Renderer::BeginFrame(int width, int height)
{
	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_context->RSSetViewports(1, &viewport);

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (SUCCEEDED(m_context->Map(m_viewBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		ViewConstants constants{};
		constants.viewport[0] = static_cast<float>(width);
		constants.viewport[1] = static_cast<float>(height);
		std::memcpy(mapped.pData, &constants, sizeof(constants));
		m_context->Unmap(m_viewBuffer.Get(), 0);
	}

	UINT stride = sizeof(SlVertex2D);
	UINT offset = 0;
	ID3D11Buffer* vb = m_vertexBuffer.Get();
	m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	m_context->IASetInputLayout(m_inputLayout.Get());
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	ID3D11Buffer* cb = m_viewBuffer.Get();
	m_context->VSSetConstantBuffers(0, 1, &cb);
	m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	ID3D11SamplerState* sampler = m_sampler.Get();
	m_context->PSSetSamplers(0, 1, &sampler);
	m_context->RSSetState(m_rasterizer.Get());
}

void D3D11Renderer::EndFrame()
{
	ID3D11ShaderResourceView* nullSrv = nullptr;
	m_context->PSSetShaderResources(0, 1, &nullSrv);
	m_context->Flush();
}

void D3D11Renderer::DrawSprite(SlTextureId texture, const SlRect& dst, const SlRect& uv, const SlColor& tint)
{
	SlVertex2D vertices[4];
	vertices[0].pos = SlVec3(dst.x, dst.y, 0.0f);
	vertices[1].pos = SlVec3(dst.x + dst.w, dst.y, 0.0f);
	vertices[2].pos = SlVec3(dst.x + dst.w, dst.y + dst.h, 0.0f);
	vertices[3].pos = SlVec3(dst.x, dst.y + dst.h, 0.0f);
	vertices[0].uv = SlVec2(uv.x, uv.y);
	vertices[1].uv = SlVec2(uv.x + uv.w, uv.y);
	vertices[2].uv = SlVec2(uv.x + uv.w, uv.y + uv.h);
	vertices[3].uv = SlVec2(uv.x, uv.y + uv.h);
	for (int i = 0; i < 4; ++i)
		vertices[i].color = tint;
	const unsigned short indices[] = { 0, 1, 2, 2, 3, 0 };
	DrawTriangles(texture, vertices, 4, indices, 6, SlBlendMode::Normal);
}

void D3D11Renderer::DrawTriangles(SlTextureId texture, const SlVertex2D* vertices, int vertexCount, const unsigned short* indices, int indexCount, SlBlendMode blendMode, bool premultipliedAlpha)
{
	if (vertices == nullptr || indices == nullptr || vertexCount <= 0 || indexCount <= 0)
		return;

	if (!EnsureVertexBuffer(static_cast<size_t>(vertexCount)) || !EnsureIndexBuffer(static_cast<size_t>(indexCount)))
		return;

	std::vector<SlVertex2D> premultipliedVertices;
	const SlVertex2D* uploadVertices = vertices;
	if (premultipliedAlpha)
	{
		premultipliedVertices.assign(vertices, vertices + vertexCount);
		for (SlVertex2D& vertex : premultipliedVertices)
		{
			vertex.color.r *= vertex.color.a;
			vertex.color.g *= vertex.color.a;
			vertex.color.b *= vertex.color.a;
		}
		uploadVertices = premultipliedVertices.data();
	}

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (FAILED(m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		return;
	std::memcpy(mapped.pData, uploadVertices, sizeof(SlVertex2D) * static_cast<size_t>(vertexCount));
	m_context->Unmap(m_vertexBuffer.Get(), 0);

	if (FAILED(m_context->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		return;
	std::memcpy(mapped.pData, indices, sizeof(unsigned short) * static_cast<size_t>(indexCount));
	m_context->Unmap(m_indexBuffer.Get(), 0);

	UINT stride = sizeof(SlVertex2D);
	UINT offset = 0;
	ID3D11Buffer* vb = m_vertexBuffer.Get();
	m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	auto textureIt = m_textures.find(texture);
	if (textureIt == m_textures.end())
		textureIt = m_textures.find(1);
	if (textureIt == m_textures.end())
		return;

	ApplyBlendMode(blendMode, premultipliedAlpha);
	ID3D11ShaderResourceView* srv = textureIt->second.texture.srv.Get();
	m_context->PSSetShaderResources(0, 1, &srv);
	m_context->DrawIndexed(static_cast<UINT>(indexCount), 0, 0);
}

void D3D11Renderer::Submit(const SlDrawList& drawList, const std::unordered_map<std::uint64_t, SlTextureId>& textureMap)
{
	for (const auto& command : drawList.commands)
	{
		auto textureIt = textureMap.find(command.textureId);
		if (textureIt == textureMap.end())
			continue;
		const SlTextureId texture = textureIt->second;
		DrawTriangles(texture, command.vertices.data(), static_cast<int>(command.vertices.size()),
			command.indices.data(), static_cast<int>(command.indices.size()),
			command.blendMode, command.premultipliedAlpha);
	}
}

SlTextureId D3D11Renderer::LoadTexture(const wchar_t* path, bool premultiplyAlpha)
{
	TextureSlot slot;
	if (!LoadTextureFromFile(m_device, path, slot.texture, premultiplyAlpha))
		return 0;
	const SlTextureId id = ++m_nextTextureId;
	m_textures[id] = slot;
	return id;
}

void D3D11Renderer::ReleaseTexture(SlTextureId texture) noexcept
{
	if (texture == 0 || texture == 1)
		return;
	m_renderTargets.erase(texture);
	m_textures.erase(texture);
}

void* D3D11Renderer::GetTextureSrv(SlTextureId texture) const noexcept
{
	auto it = m_textures.find(texture);
	if (it == m_textures.end())
		return nullptr;
	return it->second.texture.srv.Get();
}

bool D3D11Renderer::GetTextureSize(SlTextureId texture, int& outWidth, int& outHeight) const noexcept
{
	auto it = m_textures.find(texture);
	if (it == m_textures.end())
		return false;
	outWidth = it->second.texture.width;
	outHeight = it->second.texture.height;
	return outWidth > 0 && outHeight > 0;
}

bool D3D11Renderer::ReadTexturePixels(SlTextureId texture, std::vector<unsigned char>& outRgba, int& outWidth, int& outHeight, int& outStride)
{
	outRgba.clear();
	outWidth = 0;
	outHeight = 0;
	outStride = 0;
	if (m_device == nullptr || m_context == nullptr)
		return false;

	auto it = m_textures.find(texture);
	if (it == m_textures.end() || !it->second.texture.texture)
		return false;

	D3D11_TEXTURE2D_DESC desc{};
	it->second.texture.texture->GetDesc(&desc);
	if (desc.Width == 0 || desc.Height == 0 || desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
		return false;

	D3D11_TEXTURE2D_DESC stagingDesc = desc;
	stagingDesc.MipLevels = 1;
	stagingDesc.ArraySize = 1;
	stagingDesc.BindFlags = 0;
	stagingDesc.MiscFlags = 0;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;
	if (FAILED(m_device->CreateTexture2D(&stagingDesc, nullptr, staging.GetAddressOf())))
		return false;

	ID3D11ShaderResourceView* nullSrv = nullptr;
	m_context->PSSetShaderResources(0, 1, &nullSrv);
	m_context->CopyResource(staging.Get(), it->second.texture.texture.Get());

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (FAILED(m_context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
		return false;

	outWidth = static_cast<int>(desc.Width);
	outHeight = static_cast<int>(desc.Height);
	outStride = outWidth * 4;
	outRgba.resize(static_cast<size_t>(outStride) * outHeight);
	for (int y = 0; y < outHeight; ++y)
	{
		const auto* src = static_cast<const unsigned char*>(mapped.pData) + static_cast<size_t>(mapped.RowPitch) * y;
		unsigned char* dst = outRgba.data() + static_cast<size_t>(outStride) * y;
		std::memcpy(dst, src, static_cast<size_t>(outStride));
	}
	m_context->Unmap(staging.Get(), 0);
	return true;
}

bool D3D11Renderer::RasterizeToPixels(SlTextureId texture, const SlVertex2D* vertices, int vertexCount, const unsigned short* indices, int indexCount,
	int width, int height, bool premultipliedAlpha, std::vector<unsigned char>& outRgba, int& outStride)
{
	outRgba.clear();
	outStride = 0;
	if (m_device == nullptr || m_context == nullptr || width <= 0 || height <= 0)
		return false;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> previousTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> previousDepth;
	m_context->OMGetRenderTargets(1, previousTarget.GetAddressOf(), previousDepth.GetAddressOf());

	UINT viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	D3D11_VIEWPORT previousViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
	m_context->RSGetViewports(&viewportCount, previousViewports);

	const SlTextureId renderTarget = CreateRenderTarget(width, height);
	if (renderTarget == 0)
		return false;

	ID3D11RenderTargetView* target = previousTarget.Get();
	ID3D11DepthStencilView* depth = previousDepth.Get();
	bool ok = false;
	if (BeginRenderTarget(renderTarget, SlVec4(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		DrawTriangles(texture, vertices, vertexCount, indices, indexCount, SlBlendMode::Normal, premultipliedAlpha);
		EndFrame();

		m_context->OMSetRenderTargets(1, &target, depth);
		if (viewportCount > 0)
			m_context->RSSetViewports(viewportCount, previousViewports);

		int readWidth = 0;
		int readHeight = 0;
		ok = ReadTexturePixels(renderTarget, outRgba, readWidth, readHeight, outStride) && readWidth == width && readHeight == height;
	}

	m_context->OMSetRenderTargets(1, &target, depth);
	if (viewportCount > 0)
		m_context->RSSetViewports(viewportCount, previousViewports);

	ReleaseTexture(renderTarget);
	return ok;
}

SlTextureId D3D11Renderer::CreateFallbackTexture()
{
	if (m_textures.find(1) != m_textures.end())
		return 1;
	TextureSlot slot;
	if (!CreateSolidTexture(m_device, 255, 255, 255, 255, slot.texture))
		return 0;
	m_textures[1] = slot;
	return 1;
}

SlTextureId D3D11Renderer::CreateRenderTarget(int width, int height)
{
	if (width <= 0 || height <= 0)
		return 0;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = static_cast<UINT>(width);
	desc.Height = static_cast<UINT>(height);
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	if (FAILED(m_device->CreateTexture2D(&desc, nullptr, texture.GetAddressOf())))
		return 0;

	RenderTargetSlot renderTarget;
	if (FAILED(m_device->CreateRenderTargetView(texture.Get(), nullptr, renderTarget.rtv.GetAddressOf())))
		return 0;
	renderTarget.width = width;
	renderTarget.height = height;

	TextureSlot textureSlot;
	if (FAILED(m_device->CreateShaderResourceView(texture.Get(), nullptr, textureSlot.texture.srv.GetAddressOf())))
		return 0;
	textureSlot.texture.texture = texture;
	textureSlot.texture.width = width;
	textureSlot.texture.height = height;

	const SlTextureId id = ++m_nextTextureId;
	m_textures[id] = textureSlot;
	m_renderTargets[id] = renderTarget;
	return id;
}

bool D3D11Renderer::BeginRenderTarget(SlTextureId renderTarget, const SlVec4& clearColor)
{
	auto it = m_renderTargets.find(renderTarget);
	if (it == m_renderTargets.end())
		return false;

	ID3D11ShaderResourceView* nullSrv = nullptr;
	m_context->PSSetShaderResources(0, 1, &nullSrv);
	ID3D11RenderTargetView* target = it->second.rtv.Get();
	m_context->OMSetRenderTargets(1, &target, nullptr);
	const FLOAT color[] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	m_context->ClearRenderTargetView(target, color);

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<FLOAT>(it->second.width);
	viewport.Height = static_cast<FLOAT>(it->second.height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_context->RSSetViewports(1, &viewport);
	BeginFrame(it->second.width, it->second.height);
	return true;
}

bool D3D11Renderer::EnsureVertexBuffer(size_t vertexCount)
{
	if (vertexCount <= m_vertexBufferCapacity && m_vertexBuffer)
		return true;

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = static_cast<UINT>(sizeof(SlVertex2D) * vertexCount);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_device->CreateBuffer(&desc, nullptr, m_vertexBuffer.ReleaseAndGetAddressOf())))
		return false;
	m_vertexBufferCapacity = vertexCount;
	return true;
}

bool D3D11Renderer::EnsureIndexBuffer(size_t indexCount)
{
	if (indexCount <= m_indexBufferCapacity && m_indexBuffer)
		return true;

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = static_cast<UINT>(sizeof(unsigned short) * indexCount);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_device->CreateBuffer(&desc, nullptr, m_indexBuffer.ReleaseAndGetAddressOf())))
		return false;
	m_indexBufferCapacity = indexCount;
	return true;
}

void D3D11Renderer::ApplyBlendMode(SlBlendMode blendMode, bool premultipliedAlpha)
{
	const float factor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState* blend = nullptr;
	switch (blendMode)
	{
	case SlBlendMode::Additive:
		blend = premultipliedAlpha ? m_pmaAddBlend.Get() : m_addBlend.Get();
		break;
	case SlBlendMode::Multiply:
		blend = premultipliedAlpha ? m_pmaMultiplyBlend.Get() : m_multiplyBlend.Get();
		break;
	case SlBlendMode::Screen:
		blend = m_screenBlend.Get();
		break;
	case SlBlendMode::Normal:
	default:
		blend = premultipliedAlpha ? m_pmaAlphaBlend.Get() : m_alphaBlend.Get();
		break;
	}
	m_context->OMSetBlendState(blend, factor, 0xffffffff);
}

}
