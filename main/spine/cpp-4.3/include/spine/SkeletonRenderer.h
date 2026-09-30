#ifndef Spine_SkeletonRenderer_h
#define Spine_SkeletonRenderer_h

#include <spine/BlockAllocator.h>
#include <spine/BlendMode.h>
#include <spine/SkeletonClipping.h>

namespace spine {
	class Skeleton;

	struct SP_API RenderCommand {
		float *positions;
		float *uvs;
		uint32_t *colors;
		uint32_t *darkColors;
		int32_t numVertices;
		uint16_t *indices;
		int32_t numIndices;
		BlendMode blendMode;
		void *texture;
		RenderCommand *next;
	};

	class SP_API SkeletonRenderer : public SpineObject {
	public:
		explicit SkeletonRenderer();

		~SkeletonRenderer();

		RenderCommand *render(Skeleton &skeleton);

	private:
		BlockAllocator _allocator;
		Array<float> _worldVertices;
		Array<unsigned short> _quadIndices;
		SkeletonClipping _clipping;
		Array<RenderCommand *> _renderCommands;
	};
}

#endif
