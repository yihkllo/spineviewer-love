

#pragma once

#include "CubismJsonHolder.hpp"
#include "Utils/CubismJson.hpp"

namespace Live2D {  namespace Cubism {  namespace Framework {

class CubismCdiJson : public CubismJsonHolder
{
public:
    CubismCdiJson(const csmByte* buffer, csmSizeInt size);

    virtual ~CubismCdiJson();

    csmInt32 GetParametersCount();

    const csmChar* GetParametersId(csmInt32 index);

    const csmChar* GetParametersGroupId(csmInt32 index);

    const csmChar* GetParametersName(csmInt32 index);

    csmInt32 GetParameterGroupsCount();

    const csmChar* GetParameterGroupsId(csmInt32 index);

    const csmChar* GetParameterGroupsGroupId(csmInt32 index);

    const csmChar* GetParameterGroupsName(csmInt32 index);

    csmInt32 GetPartsCount();

    const csmChar* GetPartsId(csmInt32 index);

    const csmChar* GetPartsName(csmInt32 index);


private:
    csmBool IsExistParameters() const;

    csmBool IsExistParameterGroups() const;

    csmBool IsExistParts() const;
};

}}}
