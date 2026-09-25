
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/Event.h>

#include <spine/EventData.h>

spine::Event::Event(float time, const spine::EventData &data) :
		_data(data),
		_time(time),
		_intValue(0),
		_floatValue(0),
		_stringValue(),
		_volume(1),
		_balance(0) {
}

const spine::EventData &spine::Event::getData() {
	return _data;
}

float spine::Event::getTime() {
	return _time;
}

int spine::Event::getIntValue() {
	return _intValue;
}

void spine::Event::setIntValue(int inValue) {
	_intValue = inValue;
}

float spine::Event::getFloatValue() {
	return _floatValue;
}

void spine::Event::setFloatValue(float inValue) {
	_floatValue = inValue;
}

const spine::String &spine::Event::getStringValue() {
	return _stringValue;
}

void spine::Event::setStringValue(const spine::String &inValue) {
	_stringValue = inValue;
}


float spine::Event::getVolume() {
	return _volume;
}

void spine::Event::setVolume(float inValue) {
	_volume = inValue;
}

float spine::Event::getBalance() {
	return _balance;
}

void spine::Event::setBalance(float inValue) {
	_balance = inValue;
}
