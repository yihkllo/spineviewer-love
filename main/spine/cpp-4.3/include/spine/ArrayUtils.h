#ifndef Spine_ContainerUtil_h
#define Spine_ContainerUtil_h

#include <spine/Extension.h>
#include <spine/Array.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

#include <assert.h>

namespace spine {
	class SP_API ArrayUtils : public SpineObject {
	public:

		template<typename T>
		static T *findWithName(Array<T *> &items, const String &name) {
			assert(name.length() > 0);

			for (size_t i = 0; i < items.size(); ++i) {
				T *item = items[i];
				if (item->getName() == name) {
					return item;
				}
			}

			return NULL;
		}

		template<typename T>
		static int findIndexWithName(Array<T *> &items, const String &name) {
			assert(name.length() > 0);

			for (size_t i = 0, len = items.size(); i < len; ++i) {
				T *item = items[i];
				if (item->getName() == name) {
					return static_cast<int>(i);
				}
			}

			return -1;
		}

		template<typename T>
		static T *findWithDataName(Array<T *> &items, const String &name) {
			assert(name.length() > 0);

			for (size_t i = 0; i < items.size(); ++i) {
				T *item = items[i];
				if (item->getData().getName() == name) {
					return item;
				}
			}

			return NULL;
		}

		template<typename T>
		static int findIndexWithDataName(Array<T *> &items, const String &name) {
			assert(name.length() > 0);

			for (size_t i = 0, len = items.size(); i < len; ++i) {
				T *item = items[i];
				if (item->getData().getName() == name) {
					return static_cast<int>(i);
				}
			}

			return -1;
		}

		template<typename T>
		static void deleteElements(Array<T *> &items) {
			for (int i = (int) items.size() - 1; i >= 0; i--) {
				T *item = items[i];

				delete item;

				items.removeAt(i);
			}
		}

	private:

		ArrayUtils();

		ArrayUtils(const ArrayUtils &);

		ArrayUtils &operator=(const ArrayUtils &);
	};
}

#endif
