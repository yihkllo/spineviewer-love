

#pragma once

#include "CubismNativeInclude_D3D11.hpp"

#include "../CubismRenderer.hpp"
#include "CubismType_D3D11.hpp"
#include "CubismFramework.hpp"
#include "Type/csmVector.hpp"

namespace Live2D { namespace Cubism { namespace Framework {
    enum ShaderNames
    {
        ShaderNames_SetupMask,

        ShaderNames_Normal,
        ShaderNames_NormalMasked,
        ShaderNames_NormalMaskedInverted,
        ShaderNames_NormalPremultipliedAlpha,
        ShaderNames_NormalMaskedPremultipliedAlpha,
        ShaderNames_NormalMaskedInvertedPremultipliedAlpha,

        ShaderNames_Add,
        ShaderNames_AddMasked,
        ShaderNames_AddMaskedInverted,
        ShaderNames_AddPremultipliedAlpha,
        ShaderNames_AddMaskedPremultipliedAlpha,
        ShaderNames_AddMaskedInvertedPremultipliedAlpha,

        ShaderNames_Mult,
        ShaderNames_MultMasked,
        ShaderNames_MultMaskedInverted,
        ShaderNames_MultPremultipliedAlpha,
        ShaderNames_MultMaskedPremultipliedAlpha,
        ShaderNames_MultMaskedInvertedPremultipliedAlpha,

        ShaderNames_Max,
    };
}}}

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismRenderer_D3D11;
class CubismClippingContext;

class CubismShaderSet
{
public:
    CubismShaderSet()
        : _vertexShader(NULL)
        , _pixelShader(NULL)
    {
    }

    ID3D11VertexShader*     _vertexShader;
    ID3D11PixelShader*      _pixelShader;
};

class CubismShader_D3D11
{
    friend class CubismRenderer_D3D11;

public:

    CubismShader_D3D11();

    virtual ~CubismShader_D3D11();

    void ReleaseShaderProgram();

    ID3D11VertexShader* GetVertexShader(csmUint32 assign);

    ID3D11PixelShader* GetPixelShader(csmUint32 assign);

    void SetupShader(ID3D11Device* device, ID3D11DeviceContext* renderContext);

private:

    void GenerateShaders(ID3D11Device* device);

    Csm::csmBool LoadShaderProgram(ID3D11Device* device, bool isPs, csmInt32 assign, const csmChar* entryPoint);

    csmVector<CubismShaderSet*> _shaderSets;

    csmVector<ID3D11VertexShader*> _shaderSetsVS;
    csmVector<ID3D11PixelShader*> _shaderSetsPS;

    ID3D11InputLayout*    _vertexFormat;
};

}}}}
