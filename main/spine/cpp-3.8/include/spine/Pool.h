
#ifndef Spine_Pool_h
#define Spine_Pool_h

#include <spine/Extension.h>
#include <spine/Vector.h>
#include <spine/ContainerUtil.h>
#include <spine/SpineObject.h>

namespace spine {
template<typename T>
class SP_API Pool : public SpineObject {
public:
	Pool() {
	}

	~Pool() {
		ContainerUtil::cleanUpVectorOfPointers(_objects);
	}

	T *obtain() {
		if (_objects.size() > 0) {
			T **object = &_objects[_objects.size() - 1];
			T *ret = *object;
			_objects.removeAt(_objects.size() - 1);

			return ret;
		} else {
			T *ret = new(__FILE__, __LINE__) T();

			return ret;
		}
	}

	void free(T *object) {
		if (!_objects.contains(object)) {
			_objects.add(object);
		}
	}

private:
	Vector<T *> _objects;
};
}

#endif
