#include <spine/MeshAttachment.h>
#include <spine/Atlas.h>

#include <assert.h>

using namespace spine;

RTTI_IMPL(MeshAttachment, VertexAttachment)

MeshAttachment::MeshAttachment(const String &name, Sequence *sequence)
	: VertexAttachment(name), _sequence(sequence), _regionUVs(), _triangles(), _hullLength(0), _path(), _color(1, 1, 1, 1), _sourceMesh(NULL),
	  _edges(), _width(0), _height(0) {
	assert(sequence);
}

MeshAttachment::~MeshAttachment() {
	delete _sequence;
}

void MeshAttachment::computeWorldVertices(Skeleton &skeleton, Slot &slot, size_t start, size_t count, float *worldVertices, size_t offset,
										  size_t stride) {
	VertexAttachment::computeWorldVertices(skeleton, slot, start, count, worldVertices, offset, stride);
}

Array<float> &MeshAttachment::getRegionUVs() {
	return _regionUVs;
}

void MeshAttachment::setRegionUVs(Array<float> &inValue) {
	_regionUVs.clearAndAddAll(inValue);
}

Array<unsigned short> &MeshAttachment::getTriangles() {
	return _triangles;
}

void MeshAttachment::setTriangles(Array<unsigned short> &inValue) {
	_triangles.clearAndAddAll(inValue);
}

int MeshAttachment::getHullLength() {
	return _hullLength;
}

void MeshAttachment::setHullLength(int inValue) {
	_hullLength = inValue;
}

Sequence &MeshAttachment::getSequence() {
	return *_sequence;
}

void MeshAttachment::updateSequence() {
	_sequence->update(*this);
}

const String &MeshAttachment::getPath() {
	return _path;
}

void MeshAttachment::setPath(const String &inValue) {
	_path = inValue;
}

Color &MeshAttachment::getColor() {
	return _color;
}

MeshAttachment *MeshAttachment::getSourceMesh() {
	return _sourceMesh;
}

void MeshAttachment::setSourceMesh(MeshAttachment *inValue) {
	_sourceMesh = inValue;
	if (inValue != NULL) {
		_bones.clearAndAddAll(inValue->_bones);
		_vertices.clearAndAddAll(inValue->_vertices);
		_regionUVs.clearAndAddAll(inValue->_regionUVs);
		_triangles.clearAndAddAll(inValue->_triangles);
		_hullLength = inValue->_hullLength;
		_worldVerticesLength = inValue->_worldVerticesLength;
		_edges.clearAndAddAll(inValue->_edges);
		_width = inValue->_width;
		_height = inValue->_height;
	}
}

Array<unsigned short> &MeshAttachment::getEdges() {
	return _edges;
}

void MeshAttachment::setEdges(Array<unsigned short> &inValue) {
	_edges.clearAndAddAll(inValue);
}

float MeshAttachment::getWidth() {
	return _width;
}

void MeshAttachment::setWidth(float inValue) {
	_width = inValue;
}

float MeshAttachment::getHeight() {
	return _height;
}

void MeshAttachment::setHeight(float inValue) {
	_height = inValue;
}

Attachment &MeshAttachment::copy() {
	if (_sourceMesh) return newLinkedMesh();

	MeshAttachment *copy = new (__FILE__, __LINE__) MeshAttachment(getName(), new (__FILE__, __LINE__) Sequence(*_sequence));
	copy->_path = _path;
	copy->_color.set(_color);
	copyTo(*copy);
	copy->_regionUVs.clearAndAddAll(_regionUVs);
	copy->_triangles.clearAndAddAll(_triangles);
	copy->_hullLength = _hullLength;
	copy->_edges.clearAndAddAll(_edges);
	copy->_width = _width;
	copy->_height = _height;
	return *copy;
}

MeshAttachment &MeshAttachment::newLinkedMesh() {
	MeshAttachment *copy = new (__FILE__, __LINE__) MeshAttachment(getName(), new (__FILE__, __LINE__) Sequence(*_sequence));
	copy->setTimelineAttachment(getTimelineAttachment());
	copy->_path = _path;
	copy->_color.set(_color);
	copy->setSourceMesh(_sourceMesh != NULL ? _sourceMesh : this);
	copy->updateSequence();
	return *copy;
}

void MeshAttachment::computeUVs(TextureRegion *region, Array<float> &regionUVs, Array<float> &uvs) {
	int n = (int) uvs.size();
	float u, v, width, height;
	if (region != NULL && region->getRTTI().instanceOf(AtlasRegion::rtti)) {
		AtlasRegion *r = static_cast<AtlasRegion *>(region);
		u = r->_u;
		v = r->_v;
		float textureWidth = r->getPage()->width;
		float textureHeight = r->getPage()->height;
		switch (r->_degrees) {
			case 90: {
				u -= (r->_originalHeight - r->_offsetY - r->_packedWidth) / textureWidth;
				v -= (r->_originalWidth - r->_offsetX - r->_packedHeight) / textureHeight;
				width = r->_originalHeight / textureWidth;
				height = r->_originalWidth / textureHeight;
				for (int i = 0; i < n; i += 2) {
					uvs[i] = u + regionUVs[i + 1] * width;
					uvs[i + 1] = v + (1 - regionUVs[i]) * height;
				}
				return;
			}
			case 180: {
				u -= (r->_originalWidth - r->_offsetX - r->_packedWidth) / textureWidth;
				v -= r->_offsetY / textureHeight;
				width = r->_originalWidth / textureWidth;
				height = r->_originalHeight / textureHeight;
				for (int i = 0; i < n; i += 2) {
					uvs[i] = u + (1 - regionUVs[i]) * width;
					uvs[i + 1] = v + (1 - regionUVs[i + 1]) * height;
				}
				return;
			}
			case 270: {
				u -= r->_offsetY / textureWidth;
				v -= r->_offsetX / textureHeight;
				width = r->_originalHeight / textureWidth;
				height = r->_originalWidth / textureHeight;
				for (int i = 0; i < n; i += 2) {
					uvs[i] = u + (1 - regionUVs[i + 1]) * width;
					uvs[i + 1] = v + regionUVs[i] * height;
				}
				return;
			}
			default: {
				u -= r->_offsetX / textureWidth;
				v -= (r->_originalHeight - r->_offsetY - r->_packedHeight) / textureHeight;
				width = r->_originalWidth / textureWidth;
				height = r->_originalHeight / textureHeight;
			}
		}
	} else if (region == NULL) {
		u = v = 0;
		width = height = 1;
	} else {
		u = region->_u;
		v = region->_v;
		width = region->_u2 - u;
		height = region->_v2 - v;
	}
	for (int i = 0; i < n; i += 2) {
		uvs[i] = u + regionUVs[i] * width;
		uvs[i + 1] = v + regionUVs[i + 1] * height;
	}
}
