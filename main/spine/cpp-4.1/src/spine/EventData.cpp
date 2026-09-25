
#include <spine/EventData.h>

#include <assert.h>

spine::EventData::EventData(const spine::String &name) : _name(name),
														 _intValue(0),
														 _floatValue(0),
														 _stringValue(),
														 _audioPath(),
														 _volume(1),
														 _balance(0) {
	assert(_name.length() > 0);
}

const spine::String &spine::EventData::getName() const {
	return _name;
}

int spine::EventData::getIntValue() const {
	return _intValue;
}

void spine::EventData::setIntValue(int inValue) {
	_intValue = inValue;
}

float spine::EventData::getFloatValue() const {
	return _floatValue;
}

void spine::EventData::setFloatValue(float inValue) {
	_floatValue = inValue;
}

const spine::String &spine::EventData::getStringValue() const {
	return _stringValue;
}

void spine::EventData::setStringValue(const spine::String &inValue) {
	this->_stringValue = inValue;
}

const spine::String &spine::EventData::getAudioPath() const {
	return _audioPath;
}

void spine::EventData::setAudioPath(const spine::String &inValue) {
	_audioPath = inValue;
}


float spine::EventData::getVolume() const {
	return _volume;
}

void spine::EventData::setVolume(float inValue) {
	_volume = inValue;
}

float spine::EventData::getBalance() const {
	return _balance;
}

void spine::EventData::setBalance(float inValue) {
	_balance = inValue;
}
