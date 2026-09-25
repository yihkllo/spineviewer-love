
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
		LinkedMesh(MeshAttachment *mesh, const int skinIndex, size_t slotIndex, const String &parent,
				   bool inheritTimeline);

        LinkedMesh(MeshAttachment *mesh, const String &skin, size_t slotIndex, const String &parent,
                   bool inheritTimeline);

	private:
		MeshAttachment *_mesh;
		int _skinIndex;
        String _skin;
		size_t _slotIndex;
		String _parent;
		bool _inheritTimeline;
	};
}

#endif
