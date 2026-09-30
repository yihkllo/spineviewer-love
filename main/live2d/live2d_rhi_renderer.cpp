#include "live2d_rhi_renderer.h"

#include <Model/CubismModel.hpp>

#include <QFile>
#include <QMatrix4x4>
#include <rhi/qrhi.h>
#include <rhi/qshader.h>

#include <algorithm>
#include <cstring>

namespace Csm = Live2D::Cubism::Framework;
namespace CsmRendering = Live2D::Cubism::Framework::Rendering;

namespace live2d
{
	namespace
	{
		constexpr quint32 kUniformSize = 208;
		constexpr int kBlendMask = 0;
		constexpr int kBlendNormal = 1;
		constexpr int kBlendAdd = 2;
		constexpr int kBlendMultiply = 3;

		QShader LoadShader(const char* resource)
		{
			QFile file(QString::fromLatin1(resource));
			return file.open(QIODevice::ReadOnly) ? QShader::fromSerialized(file.readAll()) : QShader{};
		}

		QRhiGraphicsPipeline::TargetBlend Blend(int mode)
		{
			using P = QRhiGraphicsPipeline;
			P::TargetBlend b;
			b.enable = true;
			switch (mode)
			{
			case kBlendMask: b.srcColor = P::Zero; b.dstColor = P::OneMinusSrcColor; b.srcAlpha = P::Zero; b.dstAlpha = P::OneMinusSrcAlpha; break;
			case kBlendAdd: b.srcColor = P::One; b.dstColor = P::One; b.srcAlpha = P::Zero; b.dstAlpha = P::One; break;
			case kBlendMultiply: b.srcColor = P::DstColor; b.dstColor = P::OneMinusSrcAlpha; b.srcAlpha = P::Zero; b.dstAlpha = P::One; break;
			default: b.srcColor = P::One; b.dstColor = P::OneMinusSrcAlpha; b.srcAlpha = P::One; b.dstAlpha = P::OneMinusSrcAlpha; break;
			}
			return b;
		}

		QRhiGraphicsPipeline::FrontFace FrontFace(QRhi* rhi)
		{
			return rhi->clipSpaceCorrMatrix()(1, 1) < 0.0f ? QRhiGraphicsPipeline::CW : QRhiGraphicsPipeline::CCW;
		}

		int BlendOf(Csm::Rendering::CubismRenderer::CubismBlendMode mode)
		{
			switch (mode)
			{
			case CsmRendering::CubismRenderer::CubismBlendMode_Additive: return kBlendAdd;
			case CsmRendering::CubismRenderer::CubismBlendMode_Multiplicative: return kBlendMultiply;
			default: return kBlendNormal;
			}
		}

		QMatrix4x4 FromCubism(const float* array)
		{
			return QMatrix4x4(array).transposed();
		}
	}

	RhiClippingContext::RhiClippingContext(RhiClippingBase* manager, Csm::CubismModel&, const Csm::csmInt32* indices, Csm::csmInt32 count)
		: CubismClippingContext(indices, count)
		, _owner(manager)
	{
		_isUsing = false;
	}

	bool RhiClippingManager::Layout(Csm::CubismModel& model, std::vector<std::vector<MaskDraw>>& buffers)
	{
		for (auto& buffer : buffers) buffer.clear();
		buffers.resize(static_cast<size_t>((std::max)(1, _renderTextureCount)));
		Csm::csmInt32 usingClipCount = 0;
		for (Csm::csmUint32 i = 0; i < _clippingContextListForMask.GetSize(); ++i)
		{
			RhiClippingContext* context = _clippingContextListForMask[i];
			CalcClippedDrawTotalBounds(model, context);
			if (context->_isUsing) ++usingClipCount;
		}
		if (usingClipCount <= 0) return false;
		SetupLayoutBounds(usingClipCount);
		for (Csm::csmUint32 i = 0; i < _clippingContextListForMask.GetSize(); ++i)
		{
			RhiClippingContext* context = _clippingContextListForMask[i];
			Csm::csmRectF* allClippedDrawRect = context->_allClippedDrawRect;
			Csm::csmRectF* layoutBounds = context->_layoutBounds;
			const Csm::csmFloat32 margin = 0.05f;
			_tmpBoundsOnModel.SetRect(allClippedDrawRect);
			_tmpBoundsOnModel.Expand(allClippedDrawRect->Width * margin, allClippedDrawRect->Height * margin);
			const Csm::csmFloat32 scaleX = layoutBounds->Width / _tmpBoundsOnModel.Width;
			const Csm::csmFloat32 scaleY = layoutBounds->Height / _tmpBoundsOnModel.Height;
			createMatrixForMask(true, layoutBounds, scaleX, scaleY);
			context->_matrixForMask.SetMatrix(_tmpMatrixForMask.GetArray());
			context->_matrixForDraw.SetMatrix(_tmpMatrixForDraw.GetArray());
			const size_t buffer = static_cast<size_t>((std::max)(0, context->_bufferIndex));
			if (buffer >= buffers.size()) continue;
			for (Csm::csmInt32 k = 0; k < context->_clippingIdCount; ++k)
			{
				const Csm::csmInt32 drawable = context->_clippingIdList[k];
				if (!model.GetDrawableDynamicFlagVertexPositionsDidChange(drawable)) continue;
				buffers[buffer].push_back({ drawable, context });
			}
		}
		return true;
	}

	struct RhiShared::Impl
	{
		QShader vertex;
		QShader fragment;
		std::unique_ptr<QRhiSampler> sampler;
		std::unique_ptr<QRhiSampler> maskSampler;
		std::unique_ptr<QRhiTexture> white;
		std::unique_ptr<QRhiBuffer> layoutUniform;
		std::unique_ptr<QRhiShaderResourceBindings> layout;
		std::unique_ptr<QRhiTexture> probe;
		std::unique_ptr<QRhiTextureRenderTarget> probeTarget;
		std::unique_ptr<QRhiRenderPassDescriptor> pass;
		std::unordered_map<int, std::unique_ptr<QRhiGraphicsPipeline>> pipelines;
		quint32 uniformSize = 256;
		bool valid = false;
	};

	RhiShared::RhiShared(QRhi* rhi) : m_rhi(rhi), m(std::make_unique<Impl>())
	{
		if (m_rhi == nullptr) return;
		m->vertex = LoadShader(":/live2d/live2d.vert.qsb");
		m->fragment = LoadShader(":/live2d/live2d.frag.qsb");
		if (!m->vertex.isValid() || !m->fragment.isValid()) return;
		m->sampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::Repeat, QRhiSampler::Repeat));
		m->maskSampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None, QRhiSampler::Repeat, QRhiSampler::Repeat));
		m->white.reset(m_rhi->newTexture(QRhiTexture::RGBA8, QSize(1, 1)));
		m->uniformSize = static_cast<quint32>(m_rhi->ubufAligned(kUniformSize));
		m->layoutUniform.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m->uniformSize));
		m->probe.reset(m_rhi->newTexture(QRhiTexture::RGBA8, QSize(1, 1), 1, QRhiTexture::RenderTarget));
		if (!m->sampler->create() || !m->maskSampler->create() || !m->white->create() || !m->layoutUniform->create() || !m->probe->create()) return;
		m->layout.reset(m_rhi->newShaderResourceBindings());
		m->layout->setBindings({
			QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, m->layoutUniform.get(), m->uniformSize),
			QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, m->white.get(), m->maskSampler.get()),
			QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage, m->white.get(), m->maskSampler.get()) });
		if (!m->layout->create()) return;
		m->probeTarget.reset(m_rhi->newTextureRenderTarget(QRhiTextureRenderTargetDescription(m->probe.get())));
		m->pass.reset(m->probeTarget->newCompatibleRenderPassDescriptor());
		m->probeTarget->setRenderPassDescriptor(m->pass.get());
		if (!m->probeTarget->create()) return;
		for (int blend = 0; blend < 4; ++blend)
			for (const bool cull : { false, true })
				if (Pipeline(blend, cull) == nullptr) return;
		m->valid = true;
	}

	RhiShared::~RhiShared() = default;
	bool RhiShared::Valid() const noexcept { return m->valid; }
	QRhiSampler* RhiShared::Sampler() const noexcept { return m->sampler.get(); }
	QRhiSampler* RhiShared::MaskSampler() const noexcept { return m->maskSampler.get(); }
	QRhiTexture* RhiShared::White() const noexcept { return m->white.get(); }
	QRhiRenderPassDescriptor* RhiShared::Pass() const noexcept { return m->pass.get(); }
	quint32 RhiShared::UniformSize() const noexcept { return m->uniformSize; }

	QRhiGraphicsPipeline* RhiShared::Pipeline(int blend, bool cull)
	{
		const int key = blend * 2 + (cull ? 1 : 0);
		if (const auto found = m->pipelines.find(key); found != m->pipelines.end()) return found->second.get();
		auto pipeline = std::unique_ptr<QRhiGraphicsPipeline>(m_rhi->newGraphicsPipeline());
		pipeline->setShaderStages({ { QRhiShaderStage::Vertex, m->vertex }, { QRhiShaderStage::Fragment, m->fragment } });
		QRhiVertexInputLayout input;
		input.setBindings({ QRhiVertexInputBinding(4 * sizeof(float)) });
		input.setAttributes({ { 0, 0, QRhiVertexInputAttribute::Float2, 0 }, { 0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float) } });
		pipeline->setVertexInputLayout(input);
		pipeline->setShaderResourceBindings(m->layout.get());
		pipeline->setRenderPassDescriptor(m->pass.get());
		pipeline->setCullMode(cull ? QRhiGraphicsPipeline::Back : QRhiGraphicsPipeline::None);
		pipeline->setFrontFace(FrontFace(m_rhi));
		pipeline->setTargetBlends({ Blend(blend) });
		if (!pipeline->create()) return nullptr;
		return m->pipelines.emplace(key, std::move(pipeline)).first->second.get();
	}

	RhiRenderer::RhiRenderer() = default;

	RhiRenderer::~RhiRenderer()
	{
		ReleaseGpu();
	}

	void RhiRenderer::Initialize(Csm::CubismModel* model)
	{
		Initialize(model, 1);
	}

	void RhiRenderer::Initialize(Csm::CubismModel* model, Csm::csmInt32 maskBufferCount)
	{
		maskBufferCount = (std::max)(1, maskBufferCount);
		m_clipping.reset();
		if (model->IsUsingMasking())
		{
			m_clipping = std::make_unique<RhiClippingManager>();
			m_clipping->SetClippingMaskBufferSize(m_maskWidth, m_maskHeight);
			m_clipping->Initialize(*model, maskBufferCount);
		}
		CubismRenderer::Initialize(model, maskBufferCount);
		const int count = model->GetDrawableCount();
		m_sorted.assign(static_cast<size_t>(count), 0);
		m_vertexOffset.assign(static_cast<size_t>(count), -1);
		m_firstIndex.assign(static_cast<size_t>(count), 0);
	}

	void RhiRenderer::SetClippingMaskBufferSize(float width, float height)
	{
		m_maskWidth = width;
		m_maskHeight = height;
		if (!m_clipping || GetModel() == nullptr) return;
		const Csm::csmInt32 count = m_clipping->GetRenderTextureCount();
		m_clipping = std::make_unique<RhiClippingManager>();
		m_clipping->SetClippingMaskBufferSize(width, height);
		m_clipping->Initialize(*GetModel(), count);
		m_maskTextures.clear();
		m_maskTargets.clear();
		m_maskPasses.clear();
		ClearBindings();
	}

	void RhiRenderer::SetTexture(int index, QImage image)
	{
		if (index < 0) return;
		if (static_cast<size_t>(index) >= m_images.size()) m_images.resize(static_cast<size_t>(index) + 1);
		m_images[static_cast<size_t>(index)] = image.format() == QImage::Format_RGBA8888 ? std::move(image) : image.convertToFormat(QImage::Format_RGBA8888);
		if (static_cast<size_t>(index) < m_textures.size()) m_textures[static_cast<size_t>(index)].reset();
		ClearBindings();
	}

	void RhiRenderer::ClearBindings() noexcept
	{
		for (auto& chunk : m_chunks) chunk->bindings.clear();
	}

	void RhiRenderer::ReleaseGpu() noexcept
	{
		m_chunks.clear();
		m_chunk = 0;
		m_current = nullptr;
		m_maskTargets.clear();
		m_maskPasses.clear();
		m_maskTextures.clear();
		m_textures.clear();
		m_indices.reset();
		m_rhi = nullptr;
	}

	bool RhiRenderer::EnsureResources(RhiShared& shared, QRhiCommandBuffer* cb)
	{
		QRhi* rhi = shared.Rhi();
		if (m_rhi != rhi)
		{
			ReleaseGpu();
			m_rhi = rhi;
		}
		m_uniformStride = static_cast<quint32>(rhi->ubufAligned(kUniformSize));
		QRhiResourceUpdateBatch* batch = nullptr;
		const auto updates = [&]() { if (!batch) batch = rhi->nextResourceUpdateBatch(); return batch; };
		if (!shared.WhiteUploaded)
		{
			QImage white(1, 1, QImage::Format_RGBA8888);
			white.fill(Qt::white);
			updates()->uploadTexture(shared.White(), white);
			shared.WhiteUploaded = true;
		}
		if (m_textures.size() < m_images.size()) m_textures.resize(m_images.size());
		for (size_t i = 0; i < m_images.size(); ++i)
		{
			if (m_textures[i] || m_images[i].isNull()) continue;
			auto texture = std::unique_ptr<QRhiTexture>(rhi->newTexture(QRhiTexture::RGBA8, m_images[i].size(), 1, QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips));
			if (!texture->create()) return false;
			updates()->uploadTexture(texture.get(), m_images[i]);
			updates()->generateMips(texture.get());
			m_textures[i] = std::move(texture);
			m_images[i] = QImage();
		}
		const Csm::CubismModel& model = *GetModel();
		if (!m_indices)
		{
			std::vector<Csm::csmUint16> indices;
			for (int i = 0; i < model.GetDrawableCount(); ++i)
			{
				m_firstIndex[static_cast<size_t>(i)] = static_cast<quint32>(indices.size());
				const Csm::csmUint16* source = model.GetDrawableVertexIndices(i);
				const int count = model.GetDrawableVertexIndexCount(i);
				if (source != nullptr && count > 0) indices.insert(indices.end(), source, source + count);
			}
			if (indices.empty()) indices.push_back(0);
			if (indices.size() % 2) indices.push_back(0);
			m_indices.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer, static_cast<quint32>(indices.size() * sizeof(Csm::csmUint16))));
			if (!m_indices->create()) { m_indices.reset(); return false; }
			updates()->uploadStaticBuffer(m_indices.get(), indices.data());
		}
		const size_t maskCount = m_clipping ? static_cast<size_t>(m_clipping->GetRenderTextureCount()) : 0;
		const QSize maskSize(static_cast<int>(m_maskWidth), static_cast<int>(m_maskHeight));
		while (m_maskTextures.size() < maskCount)
		{
			auto texture = std::unique_ptr<QRhiTexture>(rhi->newTexture(QRhiTexture::RGBA8, maskSize, 1, QRhiTexture::RenderTarget));
			if (!texture->create()) return false;
			auto target = std::unique_ptr<QRhiTextureRenderTarget>(rhi->newTextureRenderTarget(QRhiTextureRenderTargetDescription(texture.get())));
			auto pass = std::unique_ptr<QRhiRenderPassDescriptor>(target->newCompatibleRenderPassDescriptor());
			target->setRenderPassDescriptor(pass.get());
			if (!target->create()) return false;
			m_maskTextures.push_back(std::move(texture));
			m_maskPasses.push_back(std::move(pass));
			m_maskTargets.push_back(std::move(target));
		}
		if (batch) cb->resourceUpdate(batch);
		return true;
	}

	QRhiShaderResourceBindings* RhiRenderer::Bindings(RhiShared& shared, QRhiTexture* texture, QRhiTexture* mask)
	{
		if (mask == nullptr) mask = shared.White();
		const quint64 key = (static_cast<quint64>(reinterpret_cast<quintptr>(texture)) * 1000003ull) ^ static_cast<quint64>(reinterpret_cast<quintptr>(mask));
		auto& cache = m_current->bindings;
		if (const auto found = cache.find(key); found != cache.end()) return found->second.get();
		auto bindings = std::unique_ptr<QRhiShaderResourceBindings>(m_rhi->newShaderResourceBindings());
		bindings->setBindings({
			QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, m_current->uniforms.get(), shared.UniformSize()),
			QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, texture, shared.Sampler()),
			QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage, mask, shared.MaskSampler()) });
		if (!bindings->create()) return nullptr;
		return cache.emplace(key, std::move(bindings)).first->second.get();
	}

	void RhiRenderer::WriteUniform(quint32 slot, const float* projection, const float* maskMatrix, const CubismTextureColor& base,
		const CubismTextureColor& multiply, const CubismTextureColor& screen, const CubismTextureColor& channel, float mode)
	{
		const size_t offset = static_cast<size_t>(slot) * m_uniformStride;
		if (m_uniformData.size() < offset + m_uniformStride) m_uniformData.resize(offset + m_uniformStride);
		float values[kUniformSize / sizeof(float)] = {};
		std::memcpy(values, projection, 16 * sizeof(float));
		std::memcpy(values + 16, maskMatrix, 16 * sizeof(float));
		const CubismTextureColor* colors[] = { &base, &multiply, &screen, &channel };
		for (int i = 0; i < 4; ++i)
		{
			values[32 + i * 4] = colors[i]->R;
			values[33 + i * 4] = colors[i]->G;
			values[34 + i * 4] = colors[i]->B;
			values[35 + i * 4] = colors[i]->A;
		}
		values[48] = mode;
		values[49] = IsPremultipliedAlpha() ? 1.0f : 0.0f;
		values[50] = m_rhi->isYUpInFramebuffer() ? 1.0f : 0.0f;
		std::memcpy(m_uniformData.data() + offset, values, sizeof(values));
	}

	bool RhiRenderer::Drawable(int index, bool mask) const
	{
		const Csm::CubismModel& model = *GetModel();
		if (m_disabled.count(index)) return false;
		if (model.GetDrawableVertexIndexCount(index) == 0) return false;
		if (model.GetDrawableOpacity(index) <= 0.0f && !mask) return false;
		const int texture = model.GetDrawableTextureIndex(index);
		return texture >= 0 && static_cast<size_t>(texture) < m_textures.size() && m_textures[static_cast<size_t>(texture)] != nullptr;
	}

	void RhiRenderer::DoDrawModel()
	{
		Csm::CubismModel& model = *GetModel();
		const int drawableCount = model.GetDrawableCount();
		std::fill(m_vertexOffset.begin(), m_vertexOffset.end(), -1);
		m_vertexData.clear();
		m_mainDraws.clear();
		m_maskDraws.clear();
		quint32 uniformCount = 0;
		const auto vertexOf = [&](int index) {
			int& offset = m_vertexOffset[static_cast<size_t>(index)];
			if (offset >= 0) return;
			offset = static_cast<int>(m_vertexData.size() / 4);
			const int count = model.GetDrawableVertexCount(index);
			const Csm::csmFloat32* positions = model.GetDrawableVertices(index);
			const auto* uvs = reinterpret_cast<const Csm::csmFloat32*>(model.GetDrawableVertexUvs(index));
			for (int v = 0; v < count; ++v)
			{
				m_vertexData.push_back(positions[v * 2]);
				m_vertexData.push_back(positions[v * 2 + 1]);
				m_vertexData.push_back(uvs[v * 2]);
				m_vertexData.push_back(uvs[v * 2 + 1]);
			}
		};
		const QMatrix4x4 correction = m_rhi->clipSpaceCorrMatrix();
		if (m_clipping && m_clipping->Layout(model, m_maskBuffers))
		{
			m_maskDraws.resize(m_maskBuffers.size());
			for (size_t buffer = 0; buffer < m_maskBuffers.size(); ++buffer)
			{
				for (const auto& item : m_maskBuffers[buffer])
				{
					if (!Drawable(item.drawable, true)) continue;
					vertexOf(item.drawable);
					RhiClippingContext* context = item.context;
					const Csm::csmRectF* rect = context->_layoutBounds;
					const CubismTextureColor base(rect->X * 2.0f - 1.0f, rect->Y * 2.0f - 1.0f, rect->GetRight() * 2.0f - 1.0f, rect->GetBottom() * 2.0f - 1.0f);
					const QMatrix4x4 projection = correction * FromCubism(context->_matrixForMask.GetArray());
					const CubismTextureColor* channel = context->GetClippingManager()->GetChannelFlagAsColor(context->_layoutChannelIndex);
					WriteUniform(uniformCount, projection.constData(), context->_matrixForMask.GetArray(), base,
						model.GetMultiplyColor(item.drawable), model.GetScreenColor(item.drawable), *channel, 0.0f);
					m_maskDraws[buffer].push_back({ item.drawable, kBlendMask, model.GetDrawableCulling(item.drawable) != 0, uniformCount++, nullptr });
				}
			}
		}
		const Csm::csmInt32* renderOrder = model.GetDrawableRenderOrders();
		for (int i = 0; i < drawableCount; ++i) m_sorted[static_cast<size_t>(renderOrder[i])] = i;
		Csm::CubismMatrix44 mvpMatrix = GetMvpMatrix();
		const QMatrix4x4 mvp = correction * FromCubism(mvpMatrix.GetArray());
		const float identity[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		const CubismTextureColor none(0.0f, 0.0f, 0.0f, 0.0f);
		for (int i = 0; i < drawableCount; ++i)
		{
			const int index = m_sorted[static_cast<size_t>(i)];
			if (!model.GetDrawableDynamicFlagIsVisible(index)) continue;
			RhiClippingContext* context = m_clipping ? (*m_clipping->GetClippingContextListForDraw())[index] : nullptr;
			if (!Drawable(index, false)) continue;
			vertexOf(index);
			const CubismTextureColor base = GetModelColorWithOpacity(model.GetDrawableOpacity(index));
			QRhiTexture* mask = nullptr;
			float mode = 1.0f;
			const float* maskMatrix = identity;
			const CubismTextureColor* channel = &none;
			if (context != nullptr)
			{
				const size_t buffer = static_cast<size_t>((std::max)(0, context->_bufferIndex));
				mask = buffer < m_maskTextures.size() ? m_maskTextures[buffer].get() : nullptr;
				mode = model.GetDrawableInvertedMask(index) ? 3.0f : 2.0f;
				maskMatrix = context->_matrixForMask.GetArray();
				channel = context->GetClippingManager()->GetChannelFlagAsColor(context->_layoutChannelIndex);
			}
			WriteUniform(uniformCount, mvp.constData(), maskMatrix, base, model.GetMultiplyColor(index), model.GetScreenColor(index), *channel, mode);
			m_mainDraws.push_back({ index, BlendOf(model.GetDrawableBlendMode(index)), model.GetDrawableCulling(index) != 0, uniformCount++, mask });
		}
		m_uniformData.resize(static_cast<size_t>(uniformCount) * m_uniformStride);
	}

	void RhiRenderer::RecordDraws(QRhiCommandBuffer* cb, RhiShared& shared, const std::vector<Draw>& draws, const QRhiViewport& viewport)
	{
		const Csm::CubismModel& model = *GetModel();
		QRhiGraphicsPipeline* current = nullptr;
		for (const Draw& draw : draws)
		{
			QRhiGraphicsPipeline* pipeline = shared.Pipeline(draw.blend, draw.cull);
			const int textureIndex = model.GetDrawableTextureIndex(draw.drawable);
			QRhiShaderResourceBindings* bindings = Bindings(shared, m_textures[static_cast<size_t>(textureIndex)].get(), draw.mask);
			if (pipeline == nullptr || bindings == nullptr) continue;
			if (pipeline != current)
			{
				cb->setGraphicsPipeline(pipeline);
				cb->setViewport(viewport);
				current = pipeline;
			}
			const QRhiCommandBuffer::DynamicOffset offset(0, m_uniformBase + draw.uniform * m_uniformStride);
			cb->setShaderResources(bindings, 1, &offset);
			const QRhiCommandBuffer::VertexInput input(m_current->vertices.get(), m_vertexBase + static_cast<quint32>(m_vertexOffset[static_cast<size_t>(draw.drawable)]) * 4 * sizeof(float));
			cb->setVertexInput(0, 1, &input, m_indices.get(), 0, QRhiCommandBuffer::IndexUInt16);
			cb->drawIndexed(static_cast<quint32>(model.GetDrawableVertexIndexCount(draw.drawable)), 1, m_firstIndex[static_cast<size_t>(draw.drawable)]);
		}
	}

	RhiRenderer::Chunk* RhiRenderer::Reserve(quint32 vertexBytes, quint32 uniformBytes, bool append)
	{
		if (!append)
		{
			if (m_chunks.size() > 1) m_chunks.resize(1);
			m_chunk = 0;
			if (!m_chunks.empty()) m_chunks[0]->vertexUsed = m_chunks[0]->uniformUsed = 0;
		}
		if (m_chunks.empty()) m_chunks.push_back(std::make_unique<Chunk>());
		Chunk* chunk = m_chunks[m_chunk].get();
		const auto fits = [&](const Chunk& c) {
			return c.vertices && c.uniforms && c.vertexUsed + vertexBytes <= c.vertices->size() && c.uniformUsed + uniformBytes <= c.uniforms->size();
		};
		if (fits(*chunk)) return chunk;
		if (chunk->vertexUsed != 0 || chunk->uniformUsed != 0)
		{
			if (++m_chunk >= m_chunks.size()) m_chunks.push_back(std::make_unique<Chunk>());
			chunk = m_chunks[m_chunk].get();
			chunk->vertexUsed = chunk->uniformUsed = 0;
			if (fits(*chunk)) return chunk;
		}
		const quint32 renders = append ? 8u : 1u;
		const auto capacity = [renders](quint32 bytes) { return ((bytes * renders + bytes / 2 + 4095) / 4096) * 4096; };
		if (!chunk->vertices || chunk->vertices->size() < vertexBytes)
		{
			chunk->vertices.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, capacity(vertexBytes)));
			if (!chunk->vertices->create()) { chunk->vertices.reset(); return nullptr; }
		}
		if (!chunk->uniforms || chunk->uniforms->size() < uniformBytes)
		{
			chunk->bindings.clear();
			chunk->uniforms.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, capacity(uniformBytes)));
			if (!chunk->uniforms->create()) { chunk->uniforms.reset(); return nullptr; }
		}
		return chunk;
	}

	bool RhiRenderer::Record(RhiShared& shared, QRhiCommandBuffer* cb, QRhiRenderTarget* target, bool append)
	{
		if (GetModel() == nullptr || cb == nullptr || target == nullptr || !shared.Valid()) return false;
		if (!EnsureResources(shared, cb)) return false;
		DrawModel();
		QRhi* rhi = shared.Rhi();
		const quint32 vertexBytes = static_cast<quint32>(((std::max<size_t>)(16, m_vertexData.size() * sizeof(float)) + 255) / 256 * 256);
		const quint32 uniformBytes = static_cast<quint32>((std::max<size_t>)(m_uniformStride, m_uniformData.size()));
		m_current = Reserve(vertexBytes, uniformBytes, append);
		if (m_current == nullptr) return false;
		m_vertexBase = m_current->vertexUsed;
		m_uniformBase = m_current->uniformUsed;
		m_current->vertexUsed += vertexBytes;
		m_current->uniformUsed += uniformBytes;
		QRhiResourceUpdateBatch* batch = rhi->nextResourceUpdateBatch();
		if (!m_vertexData.empty()) batch->updateDynamicBuffer(m_current->vertices.get(), m_vertexBase, static_cast<quint32>(m_vertexData.size() * sizeof(float)), m_vertexData.data());
		if (!m_uniformData.empty()) batch->updateDynamicBuffer(m_current->uniforms.get(), m_uniformBase, static_cast<quint32>(m_uniformData.size()), m_uniformData.data());
		cb->resourceUpdate(batch);
		const QSize maskSize(static_cast<int>(m_maskWidth), static_cast<int>(m_maskHeight));
		for (size_t buffer = 0; buffer < m_maskDraws.size() && buffer < m_maskTargets.size(); ++buffer)
		{
			if (m_maskDraws[buffer].empty()) continue;
			cb->beginPass(m_maskTargets[buffer].get(), QColor::fromRgbF(1, 1, 1, 1), { 1.0f, 0 });
			RecordDraws(cb, shared, m_maskDraws[buffer], { 0, 0, float(maskSize.width()), float(maskSize.height()) });
			cb->endPass();
		}
		const QSize size = target->pixelSize();
		cb->beginPass(target, QColor::fromRgbF(0, 0, 0, 0), { 1.0f, 0 });
		RecordDraws(cb, shared, m_mainDraws, { 0, 0, float(size.width()), float(size.height()) });
		cb->endPass();
		return true;
	}
}

#if !defined(SL_LIVE2D_D3D11)
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {
CubismRenderer* CubismRenderer::Create()
{
	return CSM_NEW live2d::RhiRenderer();
}

void CubismRenderer::StaticRelease()
{
}
}}}}
#endif
