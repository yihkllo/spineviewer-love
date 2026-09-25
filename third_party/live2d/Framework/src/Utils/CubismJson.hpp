

#pragma once
#include <stdio.h>
#include "CubismFramework.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmMap.hpp"
#include "Type/csmString.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Utils {
class Value;
class Error;
class NullValue;

#define CSM_JSON_ERROR_TYPE_MISMATCH            "Error:type mismatch"
#define CSM_JSON_ERROR_INDEX_OUT_OF_BOUNDS      "Error:index out of bounds"


class Value
{
    friend class Array;
    friend class Live2D::Cubism::Framework::CubismFramework;

public:
    static Value* ErrorValue;
    static Value* NullValue;

    Value() {}

    virtual ~Value() {}

    virtual const csmString& GetString(const csmString& defaultValue = "", const csmString& indent = "") = 0;

    virtual const csmChar* GetRawString(const csmString& defaultValue = "", const csmString& indent = "")
    {
        return this->GetString(defaultValue, indent).GetRawString();
    }

    virtual csmInt32 ToInt(csmInt32 defaultValue = 0) { return defaultValue; }

    virtual csmFloat32 ToFloat(csmFloat32 defaultValue = 0.0f) { return defaultValue; }

    virtual csmBool ToBoolean(csmBool defaultValue = false) { return defaultValue; }

    virtual csmInt32 GetSize() { return 0; }

    virtual csmVector<Value*>* GetVector(csmVector<Value*>* defaultValue = NULL) {
        if (!defaultValue)
        {
            defaultValue = new csmVector<Value*>;
        }
        return defaultValue;
    }

    virtual csmMap<csmString, Value*>* GetMap(csmMap<csmString, Value*>* defaultValue = NULL) { return defaultValue; }

    virtual Value& operator[](csmInt32 index)
    {
        return *(ErrorValue->SetErrorNotForClientCall(CSM_JSON_ERROR_TYPE_MISMATCH));
    }

    virtual Value& operator[](const csmString& string)
    {
        return *(NullValue->SetErrorNotForClientCall(CSM_JSON_ERROR_TYPE_MISMATCH));
    }

    virtual Value& operator[](const csmChar* s)
    {
        return *(NullValue->SetErrorNotForClientCall(CSM_JSON_ERROR_TYPE_MISMATCH));
    }

    virtual csmVector<csmString>& GetKeys()
    {
        return *s_dummyKeys;
    }

    virtual csmBool IsError() { return false; }

    virtual csmBool IsNull() { return false; }

    virtual csmBool IsBool() { return false; }

    virtual csmBool IsFloat() { return false; }

    virtual csmBool IsString() { return false; }

    virtual csmBool IsArray() { return false; }

    virtual csmBool IsMap() { return false; }

    virtual csmBool Equals(const csmString& value) { return false; }

    virtual csmBool Equals(const csmChar* value) { return false; }

    virtual csmBool Equals(csmInt32 value) { return false; }

    virtual csmBool Equals(csmFloat32 value) { return false; }

    virtual csmBool Equals(csmBool value) { return false; }

    virtual csmBool IsStatic() { return false; }

    virtual Value* SetErrorNotForClientCall(const csmChar* errorStr) {
        this->_stringBuffer = errorStr;
        return NullValue;
    }

protected:
    csmString _stringBuffer;

private:
    static csmVector<csmString>* s_dummyKeys;

    static void StaticInitializeNotForClientCall();

    static void StaticReleaseNotForClientCall();


};

class CubismJson
{
public:
    static CubismJson* Create(const csmByte* buffer, csmSizeInt size);

    static void Delete(CubismJson* instance);

    Value& GetRoot() const;

    const csmChar* GetParseError() const { return _error; }

    csmBool CheckEndOfFile() const { return (*_root)[1].Equals("EOF"); }

protected:
    csmBool ParseBytes(const csmByte* buffer, csmInt32 size);

    csmString ParseString(const csmChar* string, csmInt32 length, csmInt32 begin, csmInt32* outEndPos);


    Value* ParseObject(const csmChar* buffer, csmInt32 length, csmInt32 begin, csmInt32* outEndPos);

    Value* ParseArray(const csmChar* buffer, csmInt32 length, csmInt32 begin, csmInt32* outEndPos);

    Value* ParseValue(const csmChar* buffer, csmInt32 length, csmInt32 begin, csmInt32* outEndPos);

private:
    CubismJson();

    CubismJson(const csmByte* buffer, csmInt32 length);

    virtual ~CubismJson();

    const csmChar*  _error;
    csmInt32        _lineCount;
    Value*          _root;
};


class Float : public Value
{
public:
    Float(csmFloat32 v) : Value()
    {
        this->_value = v;
    }

    virtual ~Float() {}

    virtual csmBool IsFloat() { return true; }

    virtual const csmString& GetString(const csmString& defaultValue = "", const csmString& indent = "")
    {
#if defined(CSM_TARGET_WIN_GL) || defined(_MSC_VER)
        csmChar strbuf[32] = {'\0'};
        _snprintf_s(strbuf, 32, 32, "%f", this->_value);
        _stringBuffer = csmString(strbuf);
        return _stringBuffer;
#else
        csmChar strbuf[32] = { '\0' };
        snprintf(strbuf, 32, "%f", this->_value);
        _stringBuffer = csmString(strbuf);
        return _stringBuffer;
#endif
    }

    virtual csmInt32 ToInt(csmInt32 defaultValue = 0) { return static_cast<csmInt32>(this->_value); }

    virtual csmFloat32 ToFloat(csmFloat32 defaultValue = 0) { return this->_value; }

    virtual csmBool Equals(csmFloat32 v) { return v == this->_value; }

    virtual csmBool Equals(const csmString& v) { return false; }

    virtual csmBool Equals(const csmChar* v) { return false; }

    virtual csmBool Equals(csmInt32 v) { return false; }

    virtual csmBool Equals(csmBool v) { return false; }

private:
    csmFloat32 _value;
};


class Boolean : public Value
{
    friend class Value;

public:
    static Boolean* TrueValue;
    static Boolean* FalseValue;

    virtual ~Boolean() {}

    virtual csmBool IsBool() { return true; }

    virtual csmBool ToBoolean(csmBool defaultValue = false) { return _boolValue; }

    virtual const csmString& GetString(const csmString& defaultValue = "", const csmString& indent = "")
    {
        _stringBuffer = csmString(_boolValue ? "true" : "false");
        return _stringBuffer;
    }

    virtual csmBool Equals(csmBool v) { return v == _boolValue; }

    virtual csmBool Equals(const csmString& v) { return false; }

    virtual csmBool Equals(const csmChar* v) { return false; }

    virtual csmBool Equals(csmInt32 v) { return false; }

    virtual csmBool Equals(csmFloat32 v) { return false; }

    virtual csmBool IsStatic() { return true; }

private:
    Boolean(csmBool v) : Value() { this->_boolValue = v; }
    csmBool _boolValue;
};


class String : public Value
{
public:
    String(const csmString& s) : Value() { this->_stringBuffer = s; }

    String(const csmChar* s) : Value() { this->_stringBuffer = s; }

    virtual ~String() {}

    virtual csmBool IsString() { return true; }

    virtual const csmString& GetString(const csmString& defaultValue = "", const csmString& indent = "")
    {
        return _stringBuffer;
    }

    virtual csmBool Equals(const csmString& v) { return (_stringBuffer == v); }

    virtual csmBool Equals(const csmChar* v) { return (_stringBuffer == v); }

    virtual csmBool Equals(csmInt32 v) { return false; }

    virtual csmBool Equals(csmFloat32 v) { return false; }

    virtual csmBool Equals(csmBool v) { return false; }
};


class Error : public String
{
    friend class Value;
    friend class Array;
    friend class CubismJson;

public:
    virtual csmBool IsStatic() { return _isStatic; }

    virtual Value* SetErrorNotForClientCall(const csmChar* s)
    {
        this->_stringBuffer = s;
        return this;
    }

protected:
    Error(const csmString& s, csmBool isStatic) : String(s)
                                                , _isStatic(isStatic) {}

    virtual ~Error() {}

    virtual csmBool IsError() { return true; }

    csmBool _isStatic;
};


class NullValue : public Value
{
    friend class Value;
    friend class CubismJson;

public:
    virtual ~NullValue() {}

    virtual csmBool IsNull() { return true; }

    virtual const csmString& GetString(const csmString& defaultValue = "", const csmString& indent = "")
    {
        return _stringBuffer;
    }

    virtual csmBool IsStatic() { return true; }

private:
    NullValue() : Value() { _stringBuffer = "NullValue"; }
};


class Array : public Value
{
public:
    Array() : Value()
            , _array() {}

    virtual ~Array();

    virtual csmBool IsArray() { return true; }

    virtual Value& operator[](csmInt32 index)
    {
        if (index < 0 || (static_cast<csmInt32>(_array.GetSize()) <= index))
            return *(ErrorValue->SetErrorNotForClientCall(CSM_JSON_ERROR_INDEX_OUT_OF_BOUNDS));
        Value* v = _array[index];

        if (v == NULL) return *Value::NullValue;
        return *v;
    }

    virtual Value& operator[](const csmString& string)
    {
        return *(ErrorValue->SetErrorNotForClientCall(CSM_JSON_ERROR_TYPE_MISMATCH));
    }

    virtual Value& operator[](const csmChar* s)
    {
        return *(ErrorValue->SetErrorNotForClientCall(CSM_JSON_ERROR_TYPE_MISMATCH));
    }

    virtual const csmString& GetString(const csmString& defaultValue = "", const csmString& indent = "")
    {
        _stringBuffer = indent + "[\n";
        csmVector<Value*>::iterator ite = _array.Begin();
        for (; ite != _array.End(); ite++)
        {
            Value* v = (*ite);
            _stringBuffer += indent + "	" + v->GetString(indent + "	") + "\n";
        }
        _stringBuffer += indent + "]\n";

        return _stringBuffer;
    }

    void Add(Value* v) { _array.PushBack(v, false); }

    virtual csmVector<Value*>* GetVector(csmVector<Value*>* defaultValue = NULL) { return &_array; }

    virtual csmInt32 GetSize() { return static_cast<csmInt32>(_array.GetSize()); }

private:
    csmVector<Value*> _array;
};


class Map : public Value
{
public:
    Map() : Value()
          , _keys(NULL) {}

    virtual ~Map();

    virtual csmBool IsMap() { return true; }

    virtual Value& operator[](const csmString& s)
    {
        Value* ret = _map[s];
        if (ret == NULL)
        {
            return *Value::NullValue;
        }
        return *ret;
    }

    virtual Value& operator[](const csmChar* s)
    {
        for (csmMap<csmString, Value*>::const_iterator iter = _map.Begin(); iter != _map.End(); ++iter)
        {
            if( strcmp(iter->First.GetRawString(), s) == 0 )
            {
                if(iter->Second==NULL)
                {
                    return *Value::NullValue;
                }
                return *iter->Second;
            }
        }

        return *Value::NullValue;
    }

    virtual Value& operator[](csmInt32 index)
    {
        return *(ErrorValue->SetErrorNotForClientCall(CSM_JSON_ERROR_TYPE_MISMATCH));
    }

    virtual const csmString& GetString(const csmString& defaultValue = "", const csmString& indent = "")
    {
        _stringBuffer = indent + "{\n";
        csmMap<csmString, Value*>::const_iterator ite = _map.Begin();
        while (ite != _map.End())
        {
            const csmString& key = (*ite).First;
            Value* v = (*ite).Second;

            _stringBuffer += indent + "	" + key + " : " + v->GetString(indent + "	") + "\n";
            ++ite;
        }
        _stringBuffer += indent + "}\n";
        return _stringBuffer;
    }

    virtual csmMap<csmString, Value*>* GetMap(csmMap<csmString, Value*>* defaultValue = NULL)
    {
        return &_map;
    }

    void Put(csmString& key, Value* v)
    {
        _map[key] = v;
    }

    virtual csmVector<csmString>& GetKeys()
    {
        if (!_keys)
        {
            _keys = CSM_NEW csmVector<csmString>();
            csmMap<csmString, Value*>::const_iterator ite = _map.Begin();
            while (ite != _map.End())
            {
                const csmString& key = (*ite).First;
                _keys->PushBack(key, true);
                ++ite;
            }
        }
        return *_keys;
    }

    virtual csmInt32 GetSize() { return static_cast<csmInt32>(_keys->GetSize()); }

private:
    csmMap<csmString, Value*> _map;
    csmVector<csmString>* _keys;
};
}}}}

