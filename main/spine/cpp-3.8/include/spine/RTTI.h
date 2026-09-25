
#ifndef Spine_RTTI_h
#define Spine_RTTI_h

#include <spine/SpineObject.h>

namespace spine {
class SP_API RTTI : public SpineObject {
public:
	explicit RTTI(const char *className);

	RTTI(const char *className, const RTTI &baseRTTI);

	const char *getClassName() const;

	bool isExactly(const RTTI &rtti) const;

	bool instanceOf(const RTTI &rtti) const;

private:
	RTTI(const RTTI &obj);

	RTTI &operator=(const RTTI &obj);

	const char* _className;
	const RTTI *_pBaseRTTI;
};
}

#define RTTI_DECL \
public: \
static const spine::RTTI rtti; \
virtual const spine::RTTI& getRTTI() const;

#define RTTI_IMPL_NOPARENT(name) \
const spine::RTTI name::rtti(#name); \
const spine::RTTI& name::getRTTI() const { return rtti; }

#define RTTI_IMPL(name, parent) \
const spine::RTTI name::rtti(#name, parent::rtti); \
const spine::RTTI& name::getRTTI() const { return rtti; }

#endif
