#ifndef Spine_RTTI_h
#define Spine_RTTI_h

#include <spine/dll.h>

namespace spine {
	class SP_API RTTI {
	public:
		explicit RTTI(const char *className);

		RTTI(const char *className, const RTTI &baseRTTI);

		RTTI(const char *className, const RTTI &baseRTTI, const RTTI *interface1, const RTTI *interface2 = 0, const RTTI *interface3 = 0);

		const char *getClassName() const;

		bool isExactly(const RTTI &rtti) const;

		bool instanceOf(const RTTI &rtti) const;

	private:

		RTTI(const RTTI &obj);

		RTTI &operator=(const RTTI &obj);

		const char *_className;
		const RTTI *_pBaseRTTI;
		const RTTI *_interfaces[3];
		int _interfaceCount;
	};
}

#define RTTI_DECL_NOPARENT                                                                                                                           \
public:                                                                                                                                              \
	static const RTTI rtti;                                                                                                                          \
	virtual const RTTI &getRTTI() const;

#define RTTI_DECL                                                                                                                                    \
public:                                                                                                                                              \
	static const RTTI rtti;                                                                                                                          \
	virtual const RTTI &getRTTI() const override;

#define RTTI_IMPL_NOPARENT(name)                                                                                                                     \
	const RTTI name::rtti(#name);                                                                                                                    \
	const RTTI &name::getRTTI() const {                                                                                                              \
		return rtti;                                                                                                                                 \
	}

#define RTTI_IMPL(name, parent)                                                                                                                      \
	const RTTI name::rtti(#name, parent::rtti);                                                                                                      \
	const RTTI &name::getRTTI() const {                                                                                                              \
		return rtti;                                                                                                                                 \
	}

#define RTTI_IMPL_MULTI(name, parent, ...)                                                                                                           \
	const RTTI name::rtti(#name, parent::rtti, &__VA_ARGS__::rtti);                                                                                  \
	const RTTI &name::getRTTI() const {                                                                                                              \
		return rtti;                                                                                                                                 \
	}

#endif
