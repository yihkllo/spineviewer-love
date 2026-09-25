
#include <spine/Timeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

namespace spine {
	RTTI_IMPL_NOPARENT(Timeline)

	Timeline::Timeline(size_t frameCount, size_t frameEntries)
		: _propertyIds(), _frames(), _frameEntries(frameEntries) {
		_frames.setSize(frameCount * frameEntries, 0);
	}

	Timeline::~Timeline() {
	}

	Vector<PropertyId> &Timeline::getPropertyIds() {
		return _propertyIds;
	}

	void Timeline::setPropertyIds(PropertyId propertyIds[], size_t propertyIdsCount) {
		_propertyIds.clear();
		_propertyIds.ensureCapacity(propertyIdsCount);
		for (size_t i = 0; i < propertyIdsCount; i++) {
			_propertyIds.add(propertyIds[i]);
		}
	}

	size_t Timeline::getFrameCount() {
		return _frames.size() / _frameEntries;
	}

	Vector<float> &Timeline::getFrames() {
		return _frames;
	}

	size_t Timeline::getFrameEntries() {
		return _frameEntries;
	}

	float Timeline::getDuration() {
		return _frames[_frames.size() - getFrameEntries()];
	}

}
