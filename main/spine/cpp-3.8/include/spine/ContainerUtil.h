
#ifndef Spine_ContainerUtil_h
#define Spine_ContainerUtil_h

#include <spine/Extension.h>
#include <spine/Vector.h>
#include <spine/HashMap.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

#include <assert.h>

namespace spine {
	class SP_API ContainerUtil : public SpineObject {
	public:
		template<typename T>
		static T* findWithName(Vector<T*>& items, const String& name) {
			assert(name.length() > 0);

			for (size_t i = 0; i < items.size(); ++i) {
				T* item = items[i];
				if (item->getName() == name) {
					return item;
				}
			}

			return NULL;
		}

		template<typename T>
		static int findIndexWithName(Vector<T*>& items, const String& name) {
			assert(name.length() > 0);

			for (size_t i = 0, len = items.size(); i < len; ++i) {
				T* item = items[i];
				if (item->getName() == name) {
					return static_cast<int>(i);
				}
			}

			return -1;
		}

		template<typename T>
		static T* findWithDataName(Vector<T*>& items, const String& name) {
			assert(name.length() > 0);

			for (size_t i = 0; i < items.size(); ++i) {
				T* item = items[i];
				if (item->getData().getName() == name) {
					return item;
				}
			}

			return NULL;
		}

		template<typename T>
		static int findIndexWithDataName(Vector<T*>& items, const String& name) {
			assert(name.length() > 0);

			for (size_t i = 0, len = items.size(); i < len; ++i) {
				T* item = items[i];
				if (item->getData().getName() == name) {
					return static_cast<int>(i);
				}
			}

			return -1;
		}

		template<typename T>
		static void cleanUpVectorOfPointers(Vector<T*>& items) {
			for (int i = (int)items.size() - 1; i >= 0; i--) {
				T* item = items[i];

				delete item;

				items.removeAt(i);
			}
		}

	private:
		ContainerUtil();
		ContainerUtil(const ContainerUtil&);
		ContainerUtil& operator=(const ContainerUtil&);
	};
}

#endif
