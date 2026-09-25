

#pragma once

#include "Type/csmString.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismIdManager;

struct CubismId
{
    friend class CubismIdManager;

    const csmString& GetString() const;

    CubismId& operator=(const CubismId& c);

    csmBool operator==(const CubismId& c) const;
    csmBool operator!=(const CubismId& c) const;

private:
    CubismId();

    CubismId(const csmChar* id);

    ~CubismId();

    CubismId(const CubismId& c);

    csmString _id;
};

typedef const CubismId* CubismIdHandle;

}}}
