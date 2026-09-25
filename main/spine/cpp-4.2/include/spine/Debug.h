
#ifndef SPINE_LOG_H
#define SPINE_LOG_H

#include <spine/Extension.h>
#include <spine/Vector.h>

#include <map>

namespace spine {

	class SP_API DebugExtension : public SpineExtension {
		struct Allocation {
			void *address;
			size_t size;
			const char *fileName;
			int line;

			Allocation() : address(NULL), size(0), fileName(NULL), line(0) {
			}

			Allocation(void *a, size_t s, const char *f, int l) : address(a), size(s), fileName(f), line(l) {
			}
		};

	public:
		DebugExtension(SpineExtension *extension) : _extension(extension), _allocations(0), _reallocations(0),
													_frees(0) {
		}

		void reportLeaks() {
			for (std::map<void *, Allocation>::iterator it = _allocated.begin(); it != _allocated.end(); it++) {
				printf("\"%s:%i (%zu bytes at %p)\n", it->second.fileName, it->second.line, it->second.size,
					   it->second.address);
			}
			printf("allocations: %zu, reallocations: %zu, frees: %zu\n", _allocations, _reallocations, _frees);
			if (_allocated.empty()) printf("No leaks detected\n");
		}

		void clearAllocations() {
			_allocated.clear();
			_usedMemory = 0;
		}

		virtual void *_alloc(size_t size, const char *file, int line) {
			void *result = _extension->_alloc(size, file, line);
			_allocated[result] = Allocation(result, size, file, line);
			_allocations++;
			_usedMemory += size;
			return result;
		}

		virtual void *_calloc(size_t size, const char *file, int line) {
			void *result = _extension->_calloc(size, file, line);
			_allocated[result] = Allocation(result, size, file, line);
			_allocations++;
			_usedMemory += size;
			return result;
		}

		virtual void *_realloc(void *ptr, size_t size, const char *file, int line) {
			if (_allocated.count(ptr)) _usedMemory -= _allocated[ptr].size;
			_allocated.erase(ptr);
			void *result = _extension->_realloc(ptr, size, file, line);
			_reallocations++;
			_allocated[result] = Allocation(result, size, file, line);
			_usedMemory += size;
			return result;
		}

		virtual void _free(void *mem, const char *file, int line) {
			if (_allocated.count(mem)) {
				_extension->_free(mem, file, line);
				_frees++;
				_usedMemory -= _allocated[mem].size;
				_allocated.erase(mem);
				return;
			}

			printf("%s:%i (address %p): Double free or not allocated through SpineExtension\n", file, line, mem);
			_extension->_free(mem, file, line);
		}

		virtual char *_readFile(const String &path, int *length) {
            auto data = _extension->_readFile(path, length);

            if (_allocated.count(data) == 0) {
                _allocated[data] = Allocation(data, sizeof(char) * (*length), nullptr, 0);
                _allocations++;
                _usedMemory += sizeof(char) * (*length);
            }

            return data;
		}

		size_t getUsedMemory() {
			return _usedMemory;
		}

	private:
		SpineExtension *_extension;
		std::map<void *, Allocation> _allocated;
		size_t _allocations;
		size_t _reallocations;
		size_t _frees;
		size_t _usedMemory;
	};
}


#endif
