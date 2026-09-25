

#pragma once

#include "CubismNativeInclude_D3D9.hpp"

#include "../CubismRenderer.hpp"
#include "CubismType_D3D9.hpp"
#include "CubismFramework.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismRenderer_D3D9;
class CubismClippingContext_D3D9;

class CubismShaderSet
{
public:
    CubismShaderSet()
        : _shaderEffect(NULL)
    {
    }

    ID3DXEffect*    _shaderEffect;
};

class CubismShader_D3D9
{
    friend class CubismRenderer_D3D9;

public:

    CubismShader_D3D9();

    virtual ~CubismShader_D3D9();

    void ReleaseShaderProgram();

    ID3DXEffect* GetShaderEffect() const;

    void SetupShader(LPDIRECT3DDEVICE9 pD3dDevice);

private:

    void GenerateShaders(LPDIRECT3DDEVICE9 pD3dDevice);

    Csm::csmBool LoadShaderProgram(LPDIRECT3DDEVICE9 pD3dDevice);


    ID3DXEffect*                    _shaderEffect;
    IDirect3DVertexDeclaration9*    _vertexFormat;
};

}}}}
