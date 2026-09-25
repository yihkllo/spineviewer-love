
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
		LinkedMesh(MeshAttachment *mesh, const String &skin, size_t slotIndex, const String &parent,
				   bool inheritDeform);

	private:
		MeshAttachment *_mesh;
		String _skin;
		size_t _slotIndex;
		String _parent;
		bool _inheritDeform;
	};
}

#endif
