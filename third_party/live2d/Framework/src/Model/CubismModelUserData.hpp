

#pragma once

#include "CubismModel.hpp"

namespace Live2D {  namespace Cubism {  namespace Framework {

typedef CubismIdHandle  ModelUserDataType;

class CubismModelUserData
{
public:
    struct CubismModelUserDataNode
    {
        ModelUserDataType   TargetType;
        CubismIdHandle      TargetId;
        csmString           Value;
    };

    static CubismModelUserData* Create(const csmByte* buffer, csmSizeInt size);

    static void Delete(CubismModelUserData* modelUserData);

    virtual ~CubismModelUserData();

    const csmVector<const CubismModelUserDataNode*>& GetArtMeshUserDatas() const;

private:

    void ParseUserData(const csmByte* buffer, csmSizeInt size);

    csmVector<const CubismModelUserDataNode*>    _userDataNodes;
    csmVector<const CubismModelUserDataNode*>    _artMeshUserDataNodes;
};
}}}
