
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/LinkedMesh.h>

#include <spine/MeshAttachment.h>

using namespace spine;

LinkedMesh::LinkedMesh(MeshAttachment *mesh, const String &skin, size_t slotIndex, const String &parent, bool inheritDeform) :
		_mesh(mesh),
		_skin(skin),
		_slotIndex(slotIndex),
		_parent(parent),
		_inheritDeform(inheritDeform) {
}
