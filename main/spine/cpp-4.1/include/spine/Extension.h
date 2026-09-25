
#ifndef Spine_Extension_h
#define Spine_Extension_h


#include <stdlib.h>
#include <spine/dll.h>

#define SP_UNUSED(x) (void)(x)

namespace spine {
	class String;

	class SP_API SpineExtension {
	public:
		template<typename T>
		static T *alloc(size_t num, const char *file, int line) {
			return (T *) getInstance()->_alloc(sizeof(T) * num, file, line);
		}

		template<typename T>
		static T *calloc(size_t num, const char *file, int line) {
			return (T *) getInstance()->_calloc(sizeof(T) * num, file, line);
		}

		template<typename T>
		static T *realloc(T *ptr, size_t num, const char *file, int line) {
			return (T *) getInstance()->_realloc(ptr, sizeof(T) * num, file, line);
		}

		template<typename T>
		static void free(T *ptr, const char *file, int line) {
			getInstance()->_free((void *) ptr, file, line);
		}

		template<typename T>
		static void beforeFree(T *ptr) {
			getInstance()->_beforeFree((void *) ptr);
		}

		static char *readFile(const String &path, int *length) {
			return getInstance()->_readFile(path, length);
		}

		static void setInstance(SpineExtension *inSpineExtension);

		static SpineExtension *getInstance();

		virtual ~SpineExtension();

		virtual void *_alloc(size_t size, const char *file, int line) = 0;

		virtual void *_calloc(size_t size, const char *file, int line) = 0;

		virtual void *_realloc(void *ptr, size_t size, const char *file, int line) = 0;

		virtual void _free(void *mem, const char *file, int line) = 0;

		virtual char *_readFile(const String &path, int *length) = 0;

		virtual void _beforeFree(void *ptr) { SP_UNUSED(ptr); }

	protected:
		SpineExtension();

	private:
		static SpineExtension *_instance;
	};

	class SP_API DefaultSpineExtension : public SpineExtension {
	public:
		DefaultSpineExtension();

		virtual ~DefaultSpineExtension();

	protected:
		virtual void *_alloc(size_t size, const char *file, int line) override;

		virtual void *_calloc(size_t size, const char *file, int line) override;

		virtual void *_realloc(void *ptr, size_t size, const char *file, int line) override;

		virtual void _free(void *mem, const char *file, int line) override;

		virtual char *_readFile(const String &path, int *length) override;
	};

	extern SpineExtension *getDefaultExtension();
}

#endif
