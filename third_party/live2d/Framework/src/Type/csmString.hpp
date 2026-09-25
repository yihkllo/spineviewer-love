

#pragma once

#include "CubismFramework.hpp"
#include <string.h>

namespace Live2D { namespace Cubism { namespace Framework {

class csmString
{
public:
    csmString();

    csmString(const csmChar* c);

    csmString(const csmChar* c, csmInt32 length);

    csmString(const csmString& s);

    csmString(const csmChar* c, csmInt32 length, csmBool usePtr);

    virtual ~csmString();

    csmString& operator=(const csmString& s);

    csmString& operator=(const csmChar* c);

    csmBool operator==(const csmString& s) const;

    csmBool operator==(const csmChar* c) const;

    csmBool operator<(const csmString& s) const;

    csmBool operator<(const csmChar* c) const;

    csmBool operator>(const csmString& s) const;

    csmBool operator>(const csmChar* c) const;

    csmString operator+(const csmString& s) const;

    csmString operator+(const csmChar* c) const;

    csmString& operator+=(const csmString& s);

    csmString& operator+=(const csmChar* c);

    csmString& Append(const csmChar* c, csmInt32 length);

    csmString& Append(csmInt32 length, const csmChar v);

    csmInt32 GetLength() const { return _length; }

    const csmChar* GetRawString() const;

    void Clear();

    csmInt32 GetHashcode();


protected:

    void Copy(const csmChar* c, csmInt32 length);

    void Initialize(const csmChar* c, csmInt32 length, csmBool usePtr);

    csmInt32 CalcHashcode(const csmChar* c, csmInt32 length);

private:
    static const csmInt32 SmallLength = 64;
    static const csmInt32 DefaultSize = 10;
    static csmInt32 s_totalInstanceNo;
    csmChar* _ptr;
    csmInt32 _length;
    csmInt32 _hashcode;
    csmInt32 _instanceNo;

    csmChar _small[SmallLength];

    csmBool IsEmpty() const;

    void SetEmpty();

    csmChar* WritePointer();
};
}}}


