

#pragma once

#include "CubismFramework.hpp"
#include "csmString.hpp"
#include "Utils/CubismDebug.hpp"

#ifndef NULL
#   define  NULL 0
#endif

namespace Live2D { namespace Cubism { namespace Framework {


template<class _KeyT, class _ValT>
class csmPair
{
public:

    csmPair() : First()
              , Second() {}

    csmPair(const _KeyT& key) : First(key)
                              , Second() {}

    csmPair(const _KeyT& key, const _ValT& value): First(key)
                                                 , Second(value) {}

    virtual ~csmPair() {}

    _KeyT First;
    _ValT Second;
};

template<class _KeyT, class _ValT>
class csmMap
{
public:

    csmMap();

    csmMap(csmInt32 size);

    virtual ~csmMap();

    void AppendKey(_KeyT& key)
    {
        PrepareCapacity(_size + 1, false);

        void* addr = &_keyValues[_size];
        CSM_PLACEMENT_NEW(addr) csmPair<_KeyT, _ValT>(key);

        _size += 1;
    }

    _ValT& operator[](_KeyT key)
    {
        csmInt32 found = -1;
        for (csmInt32 i = 0; i < _size; i++)
        {
            if (_keyValues[i].First == key)
            {
                found = i;
                break;
            }
        }
        if (found >= 0)
        {
            return _keyValues[found].Second;
        }
        else
        {
            AppendKey(key);
            return _keyValues[_size - 1].Second;
        }
    }

    const _ValT& operator[](_KeyT key) const
    {
        csmInt32 found = -1;
        for (csmInt32 i = 0; i < _size; i++)
        {
            if (_keyValues[i].First == key)
            {
                found = i;
                break;
            }
        }
        if (found >= 0)
        {
            return _keyValues[found].Second;
        }
        else
        {
            if (!_dummyValuePtr) _dummyValuePtr = CSM_NEW _ValT();
            return *_dummyValuePtr;
        }
    }

    csmBool IsExist(_KeyT key)
    {
        for (csmInt32 i = 0; i < _size; i++)
        {
            if (_keyValues[i].First == key)
            {
                return true;
            }
        }
        return false;
    }

    void Clear();

    csmInt32 GetSize() const { return _size; }

    void PrepareCapacity(csmInt32 newSize, csmBool fitToSize);

    class iterator
    {
        friend class csmMap;

    public:
        iterator() : _index(0)
                   , _map(NULL) {}

        iterator(csmMap<_KeyT, _ValT>* v) : _index(0)
                                          , _map(v) {}

        iterator(csmMap<_KeyT, _ValT>* v, int idx) : _index(idx)
                                                   , _map(v) {}

        iterator& operator=(const iterator& ite)
        {
            this->_index = ite._index;
            this->_map = ite._map;
            return *this;
        }

        iterator& operator++()
        {
            this->_index++;
            return *this;
        }

        iterator& operator--()
        {
            this->_index--;
            return *this;
        }

        iterator operator++(csmInt32)
        {
            iterator iteold(this->_map, this->_index++);
            return iteold;
        }

        iterator operator--(csmInt32)
        {
            iterator iteold(this->_map, this->_index--);
            return iteold;
        }

        csmPair<_KeyT, _ValT>& operator*() const
        {
            return this->_map->_keyValues[this->_index];
        }

        csmBool operator!=(const iterator& ite) const
        {
            return (this->_index != ite._index) || (this->_map != ite._map);
        }

    private:
        csmInt32 _index;
        csmMap<_KeyT, _ValT>* _map;
    };

    class const_iterator
    {
        friend class csmMap;

    public:
        const_iterator() : _index(0)
                         , _map(NULL) {}

        const_iterator(const csmMap<_KeyT, _ValT>* v) : _index(0)
                                                      , _map(v) {}

        const_iterator(const csmMap<_KeyT, _ValT>* v, csmInt32 idx) : _index(idx)
                                                               , _map(v) {}

        const_iterator& operator=(const const_iterator& ite)
        {
            this->_index = ite._index;
            this->_map = ite._map;
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
            const_iterator iteold(this->_map, this->_index++);
            return iteold;
        }

        const_iterator operator--(csmInt32)
        {
            const_iterator iteold(this->_map, this->_index--);
            return iteold;
        }

        csmPair<_KeyT, _ValT>* operator->() const
        {
            return &this->_map->_keyValues[this->_index];
        }

        csmPair<_KeyT, _ValT>& operator*() const
        {
            return this->_map->_keyValues[this->_index];
        }

        csmBool operator!=(const const_iterator& ite) const
        {
            return (this->_index != ite._index) || (this->_map != ite._map);
        }

    private:
        csmInt32 _index;
        const csmMap<_KeyT, _ValT>* _map;
    };

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
        int index = ite._index;
        if (index < 0 || _size <= index) return ite;

        if (index < _size - 1)
            memmove(&(_keyValues[index]), &(_keyValues[index + 1]), sizeof(csmPair<_KeyT, _ValT>) * (_size - index - 1));
        --_size;

        iterator ite2(this, index);
        return ite2;
    }

    const const_iterator Erase(const const_iterator& ite)
    {
        csmInt32 index = ite._index;
        if (index < 0 || _size <= index) return ite;

        if (index < _size - 1)
            memmove(&(_keyValues[index]), &(_keyValues[index + 1]), sizeof(csmPair<_KeyT, _ValT>) * (_size - index - 1));
        --_size;

        const_iterator ite2(this, index);
        return ite2;
    }

    void DumpAsInt()
    {
        for (csmInt32 i = 0; i < _size; i++) CubismLogDebug("%d ,", _keyValues[i]);
        CubismLogDebug("\n");
    }

private:
    static const csmInt32 DefaultSize = 10;

    csmPair<_KeyT, _ValT>* _keyValues;
    _ValT* _dummyValuePtr;
    csmInt32 _size;
    csmInt32 _capacity;
};



template<class _KeyT, class _ValT>
csmMap<_KeyT, _ValT>::csmMap()
    : _keyValues(NULL)
    , _dummyValuePtr(NULL)
    , _size(0)
    , _capacity(0)
{ }

template<class _KeyT, class _ValT>
csmMap<_KeyT, _ValT>::csmMap(csmInt32 size)
    : _dummyValuePtr(NULL)
{
    if (size < 1)
    {
        _keyValues = NULL;
        _capacity = 0;
        _size = 0;
    }
    else
    {
        _keyValues = static_cast<csmPair<_KeyT, _ValT> *>(CSM_MALLOC(size * sizeof(csmPair<_KeyT, _ValT>)));

        CSM_ASSERT(_keyValues != NULL);

        memset(_keyValues, 0, size * sizeof(csmPair<_KeyT, _ValT>));

        _capacity = size;
        _size = size;
    }
}

template<class _KeyT, class _ValT>
csmMap<_KeyT, _ValT>::~csmMap()
{
    Clear();
}

template<class _KeyT, class _ValT>
void csmMap<_KeyT, _ValT>::PrepareCapacity(csmInt32 newSize, csmBool fitToSize)
{
    if (newSize > _capacity)
    {
        if (_capacity == 0)
        {
            if (!fitToSize && newSize < DefaultSize) newSize = DefaultSize;

            _keyValues = static_cast<csmPair<_KeyT, _ValT> *>(CSM_MALLOC(sizeof(csmPair<_KeyT, _ValT>) * newSize));

            CSM_ASSERT(_keyValues != NULL);

            _capacity = newSize;
        }
        else
        {
            if (!fitToSize && newSize < _capacity * 2) newSize = _capacity * 2;

            csmInt32 tmp_capacity = newSize;
            csmPair<_KeyT, _ValT>* tmp = static_cast<csmPair<_KeyT, _ValT> *>(CSM_MALLOC(sizeof(csmPair<_KeyT, _ValT>) * tmp_capacity));

            CSM_ASSERT(tmp != NULL);

            memcpy(static_cast<void*>(tmp), static_cast<void*>(_keyValues), sizeof(csmPair<_KeyT, _ValT>) * _capacity);
            CSM_FREE(_keyValues);

            _keyValues = tmp;
            _capacity = newSize;
        }
    }
}

template<class _KeyT, class _ValT>
void csmMap<_KeyT, _ValT>::Clear()
{
    if (_dummyValuePtr) CSM_DELETE(_dummyValuePtr);
    for (csmInt32 i = 0; i < _size; i++)
    {
        _keyValues[i].~csmPair<_KeyT, _ValT>();
    }

    CSM_FREE(_keyValues);

    _keyValues = NULL;

    _size = 0;
    _capacity = 0;
}
}}}

