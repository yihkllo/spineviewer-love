

#include "CubismCdiJson.hpp"

namespace Live2D {  namespace Cubism {  namespace Framework {

namespace {
const csmChar* Version = "Version";
const csmChar* Parameters = "Parameters";
const csmChar* ParameterGroups = "ParameterGroups";
const csmChar* Parts = "Parts";
const csmChar* Id = "Id";
const csmChar* GroupId = "GroupId";
const csmChar* Name = "Name";
}

CubismCdiJson::CubismCdiJson(const csmByte* buffer, csmSizeInt size)
{
    CreateCubismJson(buffer, size);
}

CubismCdiJson::~CubismCdiJson()
{
    DeleteCubismJson();
}

csmBool CubismCdiJson::IsExistParameters() const
{
    Utils::Value& node = (_json->GetRoot()[Parameters]);
    return !node.IsNull() && !node.IsError();
}

csmBool CubismCdiJson::IsExistParameterGroups() const
{
    Utils::Value& node = (_json->GetRoot()[ParameterGroups]);
    return !node.IsNull() && !node.IsError();
}

csmBool CubismCdiJson::IsExistParts() const
{
    Utils::Value& node = (_json->GetRoot()[Parts]);
    return !node.IsNull() && !node.IsError();
}

csmInt32 CubismCdiJson::GetParametersCount()
{
    if (!IsExistParameters()) return 0;
    return _json->GetRoot()[Parameters].GetSize();
}

const csmChar* CubismCdiJson::GetParametersId(csmInt32 index)
{
    return _json->GetRoot()[Parameters][index][Id].GetRawString();
}

const csmChar* CubismCdiJson::GetParametersGroupId(csmInt32 index)
{
    return _json->GetRoot()[Parameters][index][GroupId].GetRawString();
}

const csmChar* CubismCdiJson::GetParametersName(csmInt32 index)
{
    return _json->GetRoot()[Parameters][index][Name].GetRawString();
}

csmInt32 CubismCdiJson::GetParameterGroupsCount()
{
    if (!IsExistParameterGroups()) return 0;
    return _json->GetRoot()[ParameterGroups].GetSize();
}

const csmChar* CubismCdiJson::GetParameterGroupsId(csmInt32 index)
{
    return _json->GetRoot()[ParameterGroups][index][Id].GetRawString();
}

const csmChar* CubismCdiJson::GetParameterGroupsGroupId(csmInt32 index)
{
    return _json->GetRoot()[ParameterGroups][index][GroupId].GetRawString();
}

const csmChar* CubismCdiJson::GetParameterGroupsName(csmInt32 index)
{
    return _json->GetRoot()[ParameterGroups][index][Name].GetRawString();
}

csmInt32 CubismCdiJson::GetPartsCount()
{
    if (!IsExistParts()) return 0;
    return _json->GetRoot()[Parts].GetSize();
}

const csmChar* CubismCdiJson::GetPartsId(csmInt32 index)
{
    return _json->GetRoot()[Parts][index][Id].GetRawString();
}

const csmChar* CubismCdiJson::GetPartsName(csmInt32 index)
{
    return _json->GetRoot()[Parts][index][Name].GetRawString();
}

}}}
