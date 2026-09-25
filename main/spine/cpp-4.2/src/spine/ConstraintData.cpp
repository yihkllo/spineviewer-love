
#include <spine/ConstraintData.h>

using namespace spine;

RTTI_IMPL_NOPARENT(ConstraintData)

ConstraintData::ConstraintData(const String &name) : _name(name), _order(0), _skinRequired(false) {
}

ConstraintData::~ConstraintData() {
}

const String &ConstraintData::getName() {
	return _name;
}

size_t ConstraintData::getOrder() {
	return _order;
}

void ConstraintData::setOrder(size_t inValue) {
	_order = inValue;
}

bool ConstraintData::isSkinRequired() {
	return _skinRequired;
}

void ConstraintData::setSkinRequired(bool inValue) {
	_skinRequired = inValue;
}
