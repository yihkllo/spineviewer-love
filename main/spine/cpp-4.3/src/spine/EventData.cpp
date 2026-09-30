#include <spine/EventData.h>

#include <assert.h>

using namespace spine;

EventData::EventData(const String &name) : _name(name), _audioPath(), _setupPose(-1, *this) {
	assert(_name.length() > 0);
}

const String &EventData::getName() const {
	return _name;
}

Event &EventData::getSetupPose() {
	return _setupPose;
}

const Event &EventData::getSetupPose() const {
	return _setupPose;
}

const String &EventData::getAudioPath() const {
	return _audioPath;
}

void EventData::setAudioPath(const String &inValue) {
	_audioPath = inValue;
}
