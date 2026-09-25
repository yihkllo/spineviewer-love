

#pragma once

#include "csmString.hpp"
#include "CubismFramework.hpp"
#include "Utils/CubismDebug.hpp"

#ifndef NULL
#   define  NULL    0
#endif

namespace Live2D { namespace Cubism { namespace Framework {


template<class T>
class csmVector
{
public:
    csmVector();

    csmVector(csmInt32 initialCapacity, csmBool zeroClear = false);

    virtual ~csmVector();

    T* GetPtr()
    {
        return _ptr;
    }

    T& operator[](csmInt32 index)
    {
        return _ptr[index];
    }

    const T& operator[](csmInt32 index) const
    {
        return _ptr[index];
    }

    T& At(int index)
    {
        return _ptr[index];
    }

    void PushBack(const T& value, csmBool callPlacementNew = true);

    void Clear();

    csmUint32 GetSize() const { return _size; }

    void Resize(csmInt32 size, T value = T())
    {
        UpdateSize(size, value, true);
    }

    void UpdateSize(csmInt32 size, T value = T(), csmBool callPlacementNew = true);

    void PrepareCapacity(csmInt32 newSize);

    class iterator;

    void Insert(iterator position, iterator begin, iterator end, bool callPlacementNew = true);

    void Assign(csmInt32 newSize, T value = T(), csmBool callPlacementNew = true);

    csmBool Remove(csmInt32 index)
    {
        if (index < 0 || _size <= index) return false;
        _ptr[index].~T();

        if (index < _size - 1) memmove(&(_ptr[index]), &(_ptr[index + 1]), sizeof(T) * (_size - index - 1));
        --_size;
        return true;
    }

    class iterator
    {
        friend class csmVector;

    public:
        iterator() : _index(0)
                   , _vector(NULL) {}

        iterator(csmVector<T>* v) : _index(0)
                                  , _vector(v) {}

        iterator(csmVector<T>* v, csmInt32 idx) : _index(idx)
                                                , _vector(v) {}

        iterator& operator=(const iterator& ite)
        {
            this->_index = ite._index;
            this->_vector = ite._vector;
            return *this;
        }

        iterator& operator++()
        {
            ++this->_index;
            return *this;
        }

        iterator& operator--()
        {
            --this->_index;
            return *this;
        }

        iterator operator++(csmInt32)
        {
            iterator iteold(this->_vector, this->_index++);
            return iteold;
        }

        iterator operator--(csmInt32)
        {
            iterator iteold(this->_vector, this->_index--);
            return iteold;
        }

        T& operator*() const
        {
            return this->_vector->_ptr[this->_index];
        }

        csmBool operator!=(const iterator& ite) const
        {
            return (this->_index != ite._index) || (this->_vector != ite._vector);
        }

    private:
        csmInt32 _index;
        csmVector<T>* _vector;

    };

    class const_iterator
    {
        friend class csmVector;

    public:
        const_iterator() : _index(0)
                         , _vector(NULL) {}

        const_iterator(const csmVector<T>* v) : _index(0)
                                              , _vector(v) {}

        const_iterator(const csmVector<T>* v, csmInt32 idx) : _index(idx)
                                                            , _vector(v) {}
        const_iterator& operator=(const const_iterator& ite)
        {
            this->_index = ite._index;
            this->_vector = ite._vector;
            return *this;
        }

        const_iterator& operator++()
        {
            ++this->_index;
            return *this;
        }

        const_iterator& operator--()
        {
            --this->_index;
            return *this;
        }

        const_iterator operator++(csmInt32)
        {
            const_iterator iteold(this->_vector, this->_index++);
            return iteold;
        }

        const_iterator operator--(csmInt32)
        {
            const_iterator iteold(this->_vector, this->_index--);
            return iteold;
        }

        T& operator*() const
        {
            return this->_vector->_ptr[this->_index];
        }

        csmBool operator!=(const const_iterator& ite) const
        {
            return (this->_index != ite._index) || (this->_vector != ite._vector);
        }

    private:
        csmInt32 _index;
        const csmVector<T>* _vector;
    };

    const iterator Begin()
    {
        iterator ite(this, 0);
        return ite;
    }

    const iterator End()
    {
        iterator ite(this, _size);

        return ite;
    }

    const const_iterator Begin() const
    {
        const_iterator ite(this, 0);
        return ite;
    }

    const const_iterator End() const
    {
        const_iterator ite(this, _size);
        return ite;
    }

    const iterator Erase(const iterator& ite)
    {
        csmInt32 index = ite._index;
        if (index < 0 || _size <= index) return ite;

        if (index < _size - 1) memmove(&(_ptr[index]), &(_ptr[index + 1]), sizeof(T) * (_size - index - 1));
        --_size;

        iterator ite2(this, index);
        return ite2;
    }

    const const_iterator Erase(const const_iterator& ite)
    {
        csmInt32 index = ite._index;
        if (index < 0 || _size <= index) return ite;

        if (index < _size - 1) memmove(&(_ptr[index]), &(_ptr[index + 1]), sizeof(T) * (_size - index - 1));

        --_size;

        const_iterator ite2(this, index);
        return ite2;
    }

    void DumpAsInt()
    {
        for (csmInt32 i = 0; i < _size; i++) CubismLogDebug("%d ,", _ptr[i]);
        CubismLogDebug("\n");
    }

    csmVector(const csmVector& c)
    {
        Copy(c);
    }

    csmVector& operator=(const csmVector& c)
    {
        if (this != &c)
        {
            Clear();
            Copy(c);
        }

        return *this;
    }

private:
    static const csmInt32 s_defaultSize = 10;

    void Copy(const csmVector& c)
    {
        _size = c._size;
        _capacity = c._capacity;

        if (c._capacity == 0)
        {
            _ptr = NULL;
            return;
        }

        _ptr = (T*)CSM_MALLOC(_capacity * sizeof(T));

        for (csmInt32 i = 0; i < _size; ++i)
        {
            CSM_PLACEMENT_NEW(&_ptr[i]) T(c._ptr[i]);
        }
    }

    T* _ptr;
    csmInt32 _size;
    csmInt32 _capacity;
};


template<class T>
csmVector<T>::csmVector()
    : _ptr(NULL)
    , _size(0)
    , _capacity(0)
{ }

template<class T>
csmVector<T>::csmVector(csmInt32 initialCapacity, csmBool zeroClear)
{
    if (initialCapacity < 1)
    {
        _ptr = NULL;
        _capacity = 0;
        _size = 0;
    }
    else
    {
        _ptr = static_cast<T *>(CSM_MALLOC(sizeof(T) * initialCapacity));

        CSM_ASSERT(_ptr != NULL);

        if (zeroClear)
        {
            memset(_ptr, 0, sizeof(T) * initialCapacity);
        }

        _capacity = initialCapacity;
        _size = 0;
    }
}

template<class T>
csmVector<T>::~csmVector()
{
    Clear();
}

template<class T>
void csmVector<T>::PushBack(const T& value, csmBool callPlacementNew)
{
    if (_size >= _capacity)
    {
        PrepareCapacity(_capacity == 0 ? s_defaultSize : _capacity * 2);
    }

    if (callPlacementNew)
    {
        CSM_PLACEMENT_NEW(&_ptr[_size++]) T(value);
    }
    else
    {
        _ptr[_size++] = value;
    }
}

template<class T>
void csmVector<T>::PrepareCapacity(csmInt32 newSize)
{
    if (newSize > _capacity)
    {
        if (_capacity == 0)
        {
            _ptr = static_cast<T *>(CSM_MALLOC(sizeof(T) * newSize));

            CSM_ASSERT(_ptr != NULL);

            _capacity = newSize;
        }
        else
        {
            csmInt32 tmp_capacity = newSize;
            T* tmp = static_cast<T *>(CSM_MALLOC(sizeof(T) * tmp_capacity));
            csmInt32 tmp_size = _size;

            CSM_ASSERT(tmp != NULL);

            for (csmInt32 i = 0; i < _size; i++)
            {
                CSM_PLACEMENT_NEW(&tmp[i]) T(_ptr[i]);
            }
            Clear();

            _ptr = tmp;
            _capacity = newSize;
            _size = tmp_size;
        }
    }
}

template<class T>
void csmVector<T>::Clear()
{
    if (_ptr != NULL)
    {
        for (csmInt32 i = 0; i < _size; i++)
        {
            _ptr[i].~T();
        }

        CSM_FREE(_ptr);
    }

    _ptr = NULL;
    _size = 0;
    _capacity = 0;
}

template<class T>
void csmVector<T>::UpdateSize(csmInt32 newSize, T value, csmBool callPlacementNew)
{
    csmInt32 cur_size = this->_size;
    if (cur_size < newSize)
    {
        PrepareCapacity(newSize);

        if (callPlacementNew)
        {
            for (csmInt32 i = _size; i < newSize; i++)
            {
                CSM_PLACEMENT_NEW(&_ptr[i]) T(value);
            }
        }
        else
        {
            for (csmInt32 i = _size; i < newSize; i++)
            {
                _ptr[i] = value;
            }
        }
    }
    else
    {
        for (csmInt32 i = newSize; i < _size; i++)
        {
            _ptr[i].~T();
        }
    }
    this->_size = newSize;
}

template<class T>
void csmVector<T>::Assign(csmInt32 newSize, T value, csmBool callPlacementNew)
{
    csmInt32 cur_size = this->_size;

    for (csmInt32 i = 0; i < _size; i++)
    {
        _ptr[i].~T();
    }

    if (cur_size < newSize)
    {
        PrepareCapacity(newSize);
    }

    if (callPlacementNew)
    {
        for (csmInt32 i = 0; i < newSize; i++)
        {
            CSM_PLACEMENT_NEW(&_ptr[i]) T(value);
        }
    }
    else
    {
        for (csmInt32 i = 0; i < newSize; i++)
        {
            _ptr[i] = value;
        }
    }

    this->_size = newSize;
}

template<class T>
void csmVector<T>::Insert(iterator position, iterator begin, iterator end, csmBool callPlacementNew)
{
    csmInt32 dst_si = position._index;
    csmInt32 src_si = begin._index;
    csmInt32 src_ei = end._index;

    csmInt32 addcount = src_ei - src_si;

    PrepareCapacity(_size + addcount);

    if (_size - dst_si > 0)
    {
        memmove(&(_ptr[dst_si + addcount]), &(_ptr[dst_si]), sizeof(T) * (_size - dst_si));
    }

    if (callPlacementNew)
    {
        for (csmInt32 i = src_si; i < src_ei; i++, dst_si++)
        {
            CSM_PLACEMENT_NEW(&_ptr[i]) T(begin._vector->_ptr[i]);
        }
    }
    else
    {
        for (csmInt32 i = src_si; i < src_ei; i++, dst_si++)
        {
            _ptr[dst_si] = begin._vector->_ptr[i];
        }
    }

    this->_size = _size + addcount;
}
}}}

