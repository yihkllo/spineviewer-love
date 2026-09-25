

#pragma once

#include "CubismJsonHolder.hpp"
#include "Utils/CubismJson.hpp"
#include "Model/CubismModel.hpp"
#include "Id/CubismIdManager.hpp"

namespace Live2D {  namespace Cubism {  namespace Framework {

class CubismModelUserDataJson : public CubismJsonHolder
{
public:
    CubismModelUserDataJson(const csmByte* buffer, csmSizeInt size);

    virtual ~CubismModelUserDataJson();

    csmInt32 GetUserDataCount() const;

    csmInt32 GetTotalUserDataSize() const;

    csmString GetUserDataTargetType(csmInt32 i) const;

    CubismIdHandle GetUserDataId(csmInt32 i) const;

    const csmChar* GetUserDataValue(csmInt32 i) const;
};

}}}
