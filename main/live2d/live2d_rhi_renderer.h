#ifndef SPINELOVE_LIVE2D_RHI_RENDERER_H_
#define SPINELOVE_LIVE2D_RHI_RENDERER_H_

#include <Rendering/CubismRenderer.hpp>
#include <Rendering/CubismClippingManager.hpp>

#include <QImage>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class QRhi;
class QRhiBuffer;
class QRhiViewport;
class QRhiCommandBuffer;
class QRhiGraphicsPipeline;
class QRhiRenderPassDescriptor;
class QRhiRenderTarget;
class QRhiSampler;
class QRhiShaderResourceBindings;
class QRhiTexture;
class QRhiTextureRenderTarget;

namespace live2d
{
	class RhiClippingContext;
	struct RhiMaskSurface {};
	using RhiClippingBase = Live2D::Cubism::Framework::Rendering::CubismClippingManager<RhiClippingContext, RhiMaskSurface>;

	class RhiClippingContext : public Live2D::Cubism::Framework::Rendering::CubismClippingContext
	{
	public:
		RhiClippingContext(RhiClippingBase* manager, Live2D::Cubism::Framework::CubismModel& model, const Live2D::Cubism::Framework::csmInt32* indices, Live2D::Cubism::Framework::csmInt32 count);
		RhiClippingBase* GetClippingManager() { return _owner; }
		RhiClippingBase* _owner;
	};

	class RhiClippingManager : public RhiClippingBase
	{
	public:
		struct MaskDraw { int drawable; RhiClippingContext* context; };
		bool Layout(Live2D::Cubism::Framework::CubismModel& model, std::vector<std::vector<MaskDraw>>& buffers);
	};

	class RhiShared
	{
	public:
		explicit RhiShared(QRhi* rhi);
		~RhiShared();
		bool Valid() const noexcept;
		QRhi* Rhi() const noexcept { return m_rhi; }
		QRhiSampler* Sampler() const noexcept;
		QRhiSampler* MaskSampler() const noexcept;
		QRhiTexture* White() const noexcept;
		QRhiRenderPassDescriptor* Pass() const noexcept;
		quint32 UniformSize() const noexcept;
		QRhiGraphicsPipeline* Pipeline(int blend, bool cull);
		bool WhiteUploaded = false;

	private:
		struct Impl;
		QRhi* m_rhi = nullptr;
		std::unique_ptr<Impl> m;
	};

	class RhiRenderer final : public Live2D::Cubism::Framework::Rendering::CubismRenderer
	{
	public:
		RhiRenderer();
		~RhiRenderer() override;

		void Initialize(Live2D::Cubism::Framework::CubismModel* model) override;
		void Initialize(Live2D::Cubism::Framework::CubismModel* model, Live2D::Cubism::Framework::csmInt32 maskBufferCount) override;
		void SetClippingMaskBufferSize(float width, float height);
		void SetDrawableDisabled(int index) { m_disabled.insert(index); }
		void SetTexture(int index, QImage image);
		bool Record(RhiShared& shared, QRhiCommandBuffer* cb, QRhiRenderTarget* target, bool append = false);
		void ReleaseGpu() noexcept;

	protected:
		void DoDrawModel() override;
		void SaveProfile() override {}
		void RestoreProfile() override {}

	private:
		struct Draw
		{
			int drawable = 0;
			int blend = 0;
			bool cull = false;
			quint32 uniform = 0;
			QRhiTexture* mask = nullptr;
		};

		struct Chunk
		{
			std::unique_ptr<QRhiBuffer> vertices;
			std::unique_ptr<QRhiBuffer> uniforms;
			std::unordered_map<quint64, std::unique_ptr<QRhiShaderResourceBindings>> bindings;
			quint32 vertexUsed = 0;
			quint32 uniformUsed = 0;
		};

		bool EnsureResources(RhiShared& shared, QRhiCommandBuffer* cb);
		Chunk* Reserve(quint32 vertexBytes, quint32 uniformBytes, bool append);
		void ClearBindings() noexcept;
		QRhiShaderResourceBindings* Bindings(RhiShared& shared, QRhiTexture* texture, QRhiTexture* mask);
		void WriteUniform(quint32 slot, const float* projection, const float* maskMatrix, const Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismTextureColor& base,
			const Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismTextureColor& multiply, const Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismTextureColor& screen,
			const Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismTextureColor& channel, float mode);
		bool Drawable(int index, bool mask) const;
		void RecordDraws(QRhiCommandBuffer* cb, RhiShared& shared, const std::vector<Draw>& draws, const QRhiViewport& viewport);

		std::unique_ptr<RhiClippingManager> m_clipping;
		std::unordered_set<int> m_disabled;
		std::vector<QImage> m_images;
		std::vector<std::unique_ptr<QRhiTexture>> m_textures;
		std::vector<std::unique_ptr<QRhiTexture>> m_maskTextures;
		std::vector<std::unique_ptr<QRhiRenderPassDescriptor>> m_maskPasses;
		std::vector<std::unique_ptr<QRhiTextureRenderTarget>> m_maskTargets;
		std::unique_ptr<QRhiBuffer> m_indices;
		std::vector<std::unique_ptr<Chunk>> m_chunks;
		size_t m_chunk = 0;
		Chunk* m_current = nullptr;
		quint32 m_vertexBase = 0;
		quint32 m_uniformBase = 0;
		std::vector<quint32> m_firstIndex;
		std::vector<int> m_vertexOffset;
		std::vector<int> m_sorted;
		std::vector<float> m_vertexData;
		std::vector<unsigned char> m_uniformData;
		std::vector<std::vector<RhiClippingManager::MaskDraw>> m_maskBuffers;
		std::vector<std::vector<Draw>> m_maskDraws;
		std::vector<Draw> m_mainDraws;
		QRhi* m_rhi = nullptr;
		quint32 m_uniformStride = 256;
		float m_maskWidth = 2048.0f;
		float m_maskHeight = 2048.0f;
	};
}

#endif
