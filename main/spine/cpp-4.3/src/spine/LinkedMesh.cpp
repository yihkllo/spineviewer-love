#include <spine/LinkedMesh.h>

#include <spine/MeshAttachment.h>

using namespace spine;

LinkedMesh::LinkedMesh(MeshAttachment &mesh, const int skinIndex, size_t slotIndex, size_t sourceIndex, const String &source, bool inheritTimelines)
	: _mesh(&mesh), _skinIndex(skinIndex), _skin(""), _slotIndex(slotIndex), _sourceIndex(sourceIndex), _source(source),
	  _inheritTimelines(inheritTimelines) {
}

LinkedMesh::LinkedMesh(MeshAttachment &mesh, const String &skin, size_t slotIndex, size_t sourceIndex, const String &source, bool inheritTimelines)
	: _mesh(&mesh), _skinIndex(-1), _skin(skin), _slotIndex(slotIndex), _sourceIndex(sourceIndex), _source(source),
	  _inheritTimelines(inheritTimelines) {
}
