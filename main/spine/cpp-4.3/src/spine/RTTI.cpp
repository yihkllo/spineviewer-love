#include <spine/RTTI.h>
#include <spine/SpineString.h>

using namespace spine;

RTTI::RTTI(const char *className) : _className(className), _pBaseRTTI(NULL), _interfaceCount(0) {
	_interfaces[0] = NULL;
	_interfaces[1] = NULL;
	_interfaces[2] = NULL;
}

RTTI::RTTI(const char *className, const RTTI &baseRTTI) : _className(className), _pBaseRTTI(&baseRTTI), _interfaceCount(0) {
	_interfaces[0] = NULL;
	_interfaces[1] = NULL;
	_interfaces[2] = NULL;
}

RTTI::RTTI(const char *className, const RTTI &baseRTTI, const RTTI *interface1, const RTTI *interface2, const RTTI *interface3)
	: _className(className), _pBaseRTTI(&baseRTTI), _interfaceCount(0) {
	_interfaces[0] = interface1;
	_interfaces[1] = interface2;
	_interfaces[2] = interface3;

	if (interface1) _interfaceCount++;
	if (interface2) _interfaceCount++;
	if (interface3) _interfaceCount++;
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

		for (int i = 0; i < pCompare->_interfaceCount; i++) {
			if (pCompare->_interfaces[i] && !strcmp(pCompare->_interfaces[i]->_className, rtti._className)) {
				return true;
			}
		}

		pCompare = pCompare->_pBaseRTTI;
	}

	return false;
}
