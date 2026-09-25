
#include <spine/RTTI.h>
#include <spine/SpineString.h>

using namespace spine;

RTTI::RTTI(const char *className) : _className(className), _pBaseRTTI(NULL) {
}

RTTI::RTTI(const char *className, const RTTI &baseRTTI) : _className(className), _pBaseRTTI(&baseRTTI) {
}

const char *RTTI::getClassName() const {
	return _className;
}

bool RTTI::isExactly(const RTTI &rtti) const {
	return !strcmp(this->_className, rtti._className);
}

bool RTTI::instanceOf(const RTTI &rtti) const {
	const RTTI *pCompare = this;
	while (pCompare) {
		if (!strcmp(pCompare->_className, rtti._className)) return true;
		pCompare = pCompare->_pBaseRTTI;
	}
	return false;
}
