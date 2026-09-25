
#ifndef Spine_Vector_h
#define Spine_Vector_h

#include <spine/Extension.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <assert.h>

namespace spine {
	template<typename T>
	class SP_API Vector : public SpineObject {
	public:
		using size_type = size_t;
		using value_type = T;

		Vector() : _size(0), _capacity(0), _buffer(NULL) {
		}

		Vector(const Vector &inVector) : _size(inVector._size), _capacity(inVector._capacity), _buffer(NULL) {
			if (_capacity > 0) {
				_buffer = allocate(_capacity);
				for (size_t i = 0; i < _size; ++i) {
					construct(_buffer + i, inVector._buffer[i]);
				}
			}
		}

		~Vector() {
			clear();
			deallocate(_buffer);
		}

		inline void clear() {
			for (size_t i = 0; i < _size; ++i) {
				destroy(_buffer + (_size - 1 - i));
			}

			_size = 0;
		}

		inline size_t getCapacity() const {
			return _capacity;
		}

		inline size_t size() const {
			return _size;
		}

		inline void setSize(size_t newSize, const T &defaultValue) {
			assert(newSize >= 0);
			size_t oldSize = _size;
			_size = newSize;
			if (_capacity < newSize) {
				if (_capacity == 0) {
					_capacity = _size;
				} else {
					_capacity = (int) (_size * 1.75f);
				}
				if (_capacity < 8) _capacity = 8;
				_buffer = spine::SpineExtension::realloc<T>(_buffer, _capacity, __FILE__, __LINE__);
			}
			if (oldSize < _size) {
				for (size_t i = oldSize; i < _size; i++) {
					construct(_buffer + i, defaultValue);
				}
			} else {
				for (size_t i = _size; i < oldSize; i++) {
					destroy(_buffer + i);
				}
			}
		}

		inline void ensureCapacity(size_t newCapacity = 0) {
			if (_capacity >= newCapacity) return;
			_capacity = newCapacity;
			_buffer = SpineExtension::realloc<T>(_buffer, newCapacity, __FILE__, __LINE__);
		}

		inline void add(const T &inValue) {
			if (_size == _capacity) {
				T valueCopy = inValue;
				_capacity = (int) (_size * 1.75f);
				if (_capacity < 8) _capacity = 8;
				_buffer = spine::SpineExtension::realloc<T>(_buffer, _capacity, __FILE__, __LINE__);
				construct(_buffer + _size++, valueCopy);
			} else {
				construct(_buffer + _size++, inValue);
			}
		}

		inline void addAll(const Vector<T> &inValue) {
			ensureCapacity(this->size() + inValue.size());
			for (size_t i = 0; i < inValue.size(); i++) {
				add(inValue[i]);
			}
		}

		inline void clearAndAddAll(const Vector<T> &inValue) {
			this->clear();
			this->addAll(inValue);
		}

		inline void removeAt(size_t inIndex) {
			assert(inIndex < _size);

			--_size;

			if (inIndex != _size) {
				for (size_t i = inIndex; i < _size; ++i) {
					T tmp(_buffer[i]);
					_buffer[i] = _buffer[i + 1];
					_buffer[i + 1] = tmp;
				}
			}

			destroy(_buffer + _size);
		}

		inline bool contains(const T &inValue) {
			for (size_t i = 0; i < _size; ++i) {
				if (_buffer[i] == inValue) {
					return true;
				}
			}

			return false;
		}

		inline int indexOf(const T &inValue) {
			for (size_t i = 0; i < _size; ++i) {
				if (_buffer[i] == inValue) {
					return (int) i;
				}
			}

			return -1;
		}

		inline T &operator[](size_t inIndex) {
			assert(inIndex < _size);

			return _buffer[inIndex];
		}

		inline const T &operator[](size_t inIndex) const {
			assert(inIndex < _size);

			return _buffer[inIndex];
		}

		inline friend bool operator==(Vector<T> &lhs, Vector<T> &rhs) {
			if (lhs.size() != rhs.size()) {
				return false;
			}

			for (size_t i = 0, n = lhs.size(); i < n; ++i) {
				if (lhs[i] != rhs[i]) {
					return false;
				}
			}

			return true;
		}

		inline friend bool operator!=(Vector<T> &lhs, Vector<T> &rhs) {
			return !(lhs == rhs);
		}

		Vector &operator=(const Vector &inVector) {
			if (this != &inVector) {
				clearAndAddAll(inVector);
			}
			return *this;
		}

		inline T *buffer() {
			return _buffer;
		}

	private:
		size_t _size;
		size_t _capacity;
		T *_buffer;

		inline T *allocate(size_t n) {
			assert(n > 0);

			T *ptr = SpineExtension::calloc<T>(n, __FILE__, __LINE__);

			assert(ptr);

			return ptr;
		}

		inline void deallocate(T *buffer) {
			if (_buffer) {
				SpineExtension::free(buffer, __FILE__, __LINE__);
			}
		}

		inline void construct(T *buffer, const T &val) {
			new(buffer) T(val);
		}

		inline void destroy(T *buffer) {
			buffer->~T();
		}

	};
}

#endif
