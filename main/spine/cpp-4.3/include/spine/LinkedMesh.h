#ifndef Spine_LinkedMesh_h
#define Spine_LinkedMesh_h

#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
	class MeshAttachment;

	class SP_API LinkedMesh : public SpineObject {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	public:
		LinkedMesh(MeshAttachment &mesh, const int skinIndex, size_t slotIndex, size_t sourceIndex, const String &source, bool inheritTimelines);

		LinkedMesh(MeshAttachment &mesh, const String &skin, size_t slotIndex, size_t sourceIndex, const String &source, bool inheritTimelines);

	private:
		MeshAttachment *_mesh;
		int _skinIndex;
		String _skin;
		size_t _slotIndex;
		size_t _sourceIndex;
		String _source;
		bool _inheritTimelines;
	};
}

#endif
