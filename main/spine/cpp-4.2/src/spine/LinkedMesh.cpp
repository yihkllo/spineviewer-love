
#include <spine/LinkedMesh.h>

#include <spine/MeshAttachment.h>

using namespace spine;

LinkedMesh::LinkedMesh(MeshAttachment *mesh, const int skinIndex, size_t slotIndex, const String &parent,
					   bool inheritTimeline) : _mesh(mesh),
											   _skinIndex(skinIndex),
											   _skin(""),
											   _slotIndex(slotIndex),
											   _parent(parent),
											   _inheritTimeline(inheritTimeline) {
}

LinkedMesh::LinkedMesh(MeshAttachment *mesh, const String &skin, size_t slotIndex, const String &parent,
					   bool inheritTimeline) : _mesh(mesh),
											   _skinIndex(-1),
											   _skin(skin),
											   _slotIndex(slotIndex),
											   _parent(parent),
											   _inheritTimeline(inheritTimeline) {
}
