

#pragma once

#include "Type/CubismBasicType.hpp"
#include "Type/csmString.hpp"
#include "Type/csmVector.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

struct CubismId;

class CubismIdManager
{
    friend struct CubismId;

public:
    CubismIdManager();

    ~CubismIdManager();

    void RegisterIds(const csmChar** ids, csmInt32 count);

    void RegisterIds(const csmVector<csmString>& ids);

    const CubismId* RegisterId(const csmChar* id);

    const CubismId* RegisterId(const csmString& id);

    const CubismId* GetId(const csmString& id);

    const CubismId* GetId(const csmChar* id);

    csmBool IsExist(const csmString& id) const;

    csmBool IsExist(const csmChar* id) const;

private:
    CubismIdManager(const CubismIdManager&);
    CubismIdManager& operator=(const CubismIdManager&);

    CubismId* FindId(const csmChar* id) const;

    csmVector<CubismId*> _ids;
};

}}}
