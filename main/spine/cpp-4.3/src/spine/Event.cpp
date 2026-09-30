#include <spine/Event.h>

#include <spine/EventData.h>

using namespace spine;

Event::Event(float time, const EventData &data) : _data(data), _time(time), _intValue(0), _floatValue(0), _stringValue(), _volume(0), _balance(0) {
}

const EventData &Event::getData() {
	return _data;
}

float Event::getTime() {
	return _time;
}

int Event::getInt() {
	return _intValue;
}

void Event::setInt(int inValue) {
	_intValue = inValue;
}

float Event::getFloat() {
	return _floatValue;
}

void Event::setFloat(float inValue) {
	_floatValue = inValue;
}

const String &Event::getString() {
	return _stringValue;
}

void Event::setString(const String &inValue) {
	_stringValue = inValue;
}

float Event::getVolume() {
	return _volume;
}

void Event::setVolume(float inValue) {
	_volume = inValue;
}

float Event::getBalance() {
	return _balance;
}

void Event::setBalance(float inValue) {
	_balance = inValue;
}
