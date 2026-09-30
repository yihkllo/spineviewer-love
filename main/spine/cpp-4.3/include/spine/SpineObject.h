#ifndef Spine_Object_h
#define Spine_Object_h

#include <new>
#include <stddef.h>

#include <spine/dll.h>

namespace spine {
	class String;

	class SP_API SpineObject {
	public:
		void *operator new(size_t sz);

		void *operator new(size_t sz, const char *file, int line);

		void *operator new(size_t sz, void *ptr);

		void operator delete(void *p, const char *file, int line);

		void operator delete(void *p, void *mem);

		void operator delete(void *p);

		virtual ~SpineObject();
	};
}

#endif
