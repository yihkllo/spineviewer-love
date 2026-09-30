#include <spine/Sequence.h>
#include <spine/MeshAttachment.h>
#include <spine/RegionAttachment.h>
#include <spine/SlotPose.h>

using namespace spine;

int Sequence::_nextID = 0;

Sequence::Sequence(int count, bool pathSuffix)
	: _id(nextID()), _regions(count), _pathSuffix(pathSuffix), _uvs(), _offsets(), _start(0), _digits(0), _setupIndex(0) {
	_regions.setSize(count, NULL);
}

Sequence::Sequence(const Sequence &other)
	: _id(nextID()), _regions(other._regions), _pathSuffix(other._pathSuffix), _uvs(other._uvs), _offsets(other._offsets), _start(other._start),
	  _digits(other._digits), _setupIndex(other._setupIndex) {
}

Sequence::~Sequence() {
}

void Sequence::update(RegionAttachment &attachment) {
	int regionCount = (int) _regions.size();
	Array<float> empty;
	_uvs.setSize(regionCount, empty);
	_offsets.setSize(regionCount, empty);
	for (int i = 0; i < regionCount; i++) {
		_uvs[i].setSize(8, 0);
		_offsets[i].setSize(8, 0);
		RegionAttachment::computeUVs(_regions[i], attachment.getX(), attachment.getY(), attachment.getScaleX(), attachment.getScaleY(),
									 attachment.getRotation(), attachment.getWidth(), attachment.getHeight(), _offsets[i], _uvs[i]);
	}
}

void Sequence::update(MeshAttachment &attachment) {
	int regionCount = (int) _regions.size();
	Array<float> empty;
	_uvs.setSize(regionCount, empty);
	_offsets.clear();
	for (int i = 0; i < regionCount; i++) {
		_uvs[i].setSize(attachment.getRegionUVs().size(), 0);
		MeshAttachment::computeUVs(_regions[i], attachment.getRegionUVs(), _uvs[i]);
	}
}

int Sequence::resolveIndex(SlotPose &pose) {
	int index = pose.getSequenceIndex();
	if (index == -1) index = _setupIndex;
	if (index >= (int) _regions.size()) index = (int) _regions.size() - 1;
	return index;
}

TextureRegion *Sequence::getRegion(int index) {
	return _regions[index];
}

Array<float> &Sequence::getUVs(int index) {
	return _uvs[index];
}

Array<float> &Sequence::getOffsets(int index) {
	return _offsets[index];
}

String &Sequence::getPath(const String &basePath, int index) {
	if (!_pathSuffix) {
		_tmpPath = basePath;
		return _tmpPath;
	}
	_tmpPath = basePath;
	String frame;
	frame.append(_start + index);
	for (int i = _digits - (int) frame.length(); i > 0; i--) _tmpPath.append("0");
	_tmpPath.append(frame);
	return _tmpPath;
}

int Sequence::nextID() {
	return _nextID++;
}
