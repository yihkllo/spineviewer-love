
#include <spine/Sequence.h>
#include <spine/Slot.h>
#include <spine/Attachment.h>
#include <spine/RegionAttachment.h>
#include <spine/MeshAttachment.h>

using namespace spine;

Sequence::Sequence(int count) : _id(Sequence::getNextID()),
								_regions(),
								_start(0),
								_digits(0),
								_setupIndex(0) {
	_regions.setSize(count, NULL);
}

Sequence::~Sequence() {
}

Sequence *Sequence::copy() {
	Sequence *copy = new (__FILE__, __LINE__) Sequence((int) _regions.size());
	for (size_t i = 0; i < _regions.size(); i++) {
		copy->_regions[i] = _regions[i];
	}
	copy->_start = _start;
	copy->_digits = _digits;
	copy->_setupIndex = _setupIndex;
	return copy;
}

void Sequence::apply(Slot *slot, Attachment *attachment) {
	int index = slot->getSequenceIndex();
	if (index == -1) index = _setupIndex;
	if (index >= (int) _regions.size()) index = (int) _regions.size() - 1;
	TextureRegion *region = _regions[index];

	if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
		RegionAttachment *regionAttachment = static_cast<RegionAttachment *>(attachment);
		if (regionAttachment->getRegion() != region) {
			regionAttachment->setRegion(region);
			regionAttachment->updateRegion();
		}
	}

	if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
		MeshAttachment *meshAttachment = static_cast<MeshAttachment *>(attachment);
		if (meshAttachment->getRegion() != region) {
			meshAttachment->setRegion(region);
			meshAttachment->updateRegion();
		}
	}
}

String Sequence::getPath(const String &basePath, int index) {
	String result(basePath);
	String frame;
	frame.append(_start + index);
	for (int i = _digits - (int) frame.length(); i > 0; i--)
		result.append("0");
	result.append(frame);
	return result;
}

int Sequence::getNextID() {
	static int _nextID = 0;
	return _nextID;
}
