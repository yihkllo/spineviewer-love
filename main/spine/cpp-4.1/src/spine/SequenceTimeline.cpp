
#include <spine/SequenceTimeline.h>
#include <spine/Bone.h>
#include <spine/RegionAttachment.h>
#include <spine/MeshAttachment.h>
#include <spine/Event.h>
#include <spine/Skeleton.h>
#include <spine/Attachment.h>
#include <spine/PathConstraintData.h>
#include <spine/Slot.h>
#include <spine/Animation.h>

using namespace spine;

RTTI_IMPL(SequenceTimeline, Timeline)

SequenceTimeline::SequenceTimeline(size_t frameCount, int slotIndex, Attachment *attachment) : Timeline(frameCount, ENTRIES), _slotIndex(slotIndex), _attachment(attachment) {
	int sequenceId = 0;
	if (attachment->getRTTI().instanceOf(RegionAttachment::rtti)) sequenceId = ((RegionAttachment *) attachment)->getSequence()->getId();
	if (attachment->getRTTI().instanceOf(MeshAttachment::rtti)) sequenceId = ((MeshAttachment *) attachment)->getSequence()->getId();
	PropertyId ids[] = {((PropertyId) Property_Sequence << 32) | ((slotIndex << 16 | sequenceId) & 0xffffffff)};
	setPropertyIds(ids, 1);
}

SequenceTimeline::~SequenceTimeline() {
}

void SequenceTimeline::setFrame(int frame, float time, SequenceMode mode, int index, float delay) {
	Vector<float> &frames = this->_frames;
	frame *= ENTRIES;
	frames[frame] = time;
	frames[frame + MODE] = mode | (index << 4);
	frames[frame + DELAY] = delay;
}

void SequenceTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents,
							 float alpha, MixBlend blend, MixDirection direction) {
	SP_UNUSED(alpha);
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);

	Slot *slot = skeleton.getSlots()[_slotIndex];
	if (!slot->getBone().isActive()) return;
	Attachment *slotAttachment = slot->getAttachment();
	if (slotAttachment != _attachment) {
		if (slotAttachment == NULL || !slotAttachment->getRTTI().instanceOf(VertexAttachment::rtti) || ((VertexAttachment *) slotAttachment)->getTimelineAttachment() != _attachment) return;
	}

	Vector<float> &frames = this->_frames;
	if (time < frames[0]) {
		if (blend == MixBlend_Setup || blend == MixBlend_First) slot->setSequenceIndex(-1);
		return;
	}

	int i = Animation::search(frames, time, ENTRIES);
	float before = frames[i];
	int modeAndIndex = (int) frames[i + MODE];
	float delay = frames[i + DELAY];

	Sequence *sequence = NULL;
	if (_attachment->getRTTI().instanceOf(RegionAttachment::rtti)) sequence = ((RegionAttachment *) _attachment)->getSequence();
	if (_attachment->getRTTI().instanceOf(MeshAttachment::rtti)) sequence = ((MeshAttachment *) _attachment)->getSequence();
	if (!sequence) return;
	int index = modeAndIndex >> 4, count = (int) sequence->getRegions().size();
	int mode = modeAndIndex & 0xf;
	if (mode != SequenceMode::hold) {
		index += (int) (((time - before) / delay + 0.00001));
		switch (mode) {
			case SequenceMode::once:
				index = MathUtil::min(count - 1, index);
				break;
			case SequenceMode::loop:
				index %= count;
				break;
			case SequenceMode::pingpong: {
				int n = (count << 1) - 2;
				index = n == 0 ? 0 : index % n;
				if (index >= count) index = n - index;
				break;
			}
			case SequenceMode::onceReverse:
				index = MathUtil::max(count - 1 - index, 0);
				break;
			case SequenceMode::loopReverse:
				index = count - 1 - (index % count);
				break;
			case SequenceMode::pingpongReverse: {
				int n = (count << 1) - 2;
				index = n == 0 ? 0 : (index + count - 1) % n;
				if (index >= count) index = n - index;
			}
		}
	}
	slot->setSequenceIndex(index);
}
