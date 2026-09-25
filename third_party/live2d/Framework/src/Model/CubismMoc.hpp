

#pragma once

#include "CubismFramework.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismModel;

class CubismMoc
{
    friend class CubismModel;
public:
    static CubismMoc* Create(const csmByte* mocBytes, csmSizeInt size, csmBool shouldCheckMocConsistency = false);

    static void Delete(CubismMoc* moc);

    CubismModel* CreateModel();

    void DeleteModel(CubismModel* model);

    static Core::csmMocVersion GetLatestMocVersion();

    Core::csmMocVersion GetMocVersion();

    static csmBool HasMocConsistency(void* address, const csmUint32 size);

    static csmBool HasMocConsistencyFromUnrevivedMoc(const csmByte* mocBytes, csmSizeInt size);

private:
    CubismMoc(Core::csmMoc* moc);

    virtual ~CubismMoc();

    Core::csmMoc*     _moc;
    csmInt32          _modelCount;
    csmUint32         _mocVersion;
};

}}}
