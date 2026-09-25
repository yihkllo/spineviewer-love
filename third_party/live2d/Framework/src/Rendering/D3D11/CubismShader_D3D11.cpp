

#include "CubismShader_D3D11.hpp"

#include "CubismRenderer_D3D11.hpp"
#include <d3dcompiler.h>

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

static const csmChar* CubismShaderEffectSrc =
    "cbuffer ConstantBuffer {"\
        "float4x4 projectMatrix;"\
        "float4x4 clipMatrix;"\
        "float4 baseColor;"\
        "float4 multiplyColor;"\
        "float4 screenColor;"\
        "float4 channelFlag;"\
    "}"\
    "Texture2D mainTexture : register(t0);"\
    "SamplerState mainSampler : register(s0);"\
    "Texture2D maskTexture : register(t1);"\
    \
    "struct VS_IN {"\
        "float2 pos : POSITION;"\
        "float2 uv : TEXCOORD0;"\
    "};"\
    "struct VS_OUT {"\
        "float4 Position : SV_POSITION;"\
        "float2 uv : TEXCOORD0;"\
        "float4 clipPosition : TEXCOORD1;"\
    "};"\
    \
"/* Setup mask shader */"\
    "VS_OUT VertSetupMask(VS_IN In) {"\
        "VS_OUT Out = (VS_OUT)0;"\
        "Out.Position = mul(float4(In.pos, 0.0f, 1.0f), projectMatrix);"\
        "Out.clipPosition = mul(float4(In.pos, 0.0f, 1.0f), projectMatrix);"\
        "Out.uv.x = In.uv.x;"\
        "Out.uv.y = 1.0 - +In.uv.y;"\
        "return Out;"\
    "}"\
    "float4 PixelSetupMask(VS_OUT In) : SV_Target{"\
        "float isInside ="\
        "step(baseColor.x, In.clipPosition.x / In.clipPosition.w)"\
        "* step(baseColor.y, In.clipPosition.y / In.clipPosition.w)"\
        "* step(In.clipPosition.x / In.clipPosition.w, baseColor.z)"\
        "* step(In.clipPosition.y / In.clipPosition.w, baseColor.w);"\
        "return channelFlag * mainTexture.Sample(mainSampler, In.uv).a * isInside;"\
    "}"\
    \
"/* Vertex Shader */"\
    "/* normal */"\
    "VS_OUT VertNormal(VS_IN In) {"\
        "VS_OUT Out = (VS_OUT)0;"\
        "Out.Position = mul(float4(In.pos, 0.0f, 1.0f), projectMatrix);"\
        "Out.uv.x = In.uv.x;"\
        "Out.uv.y = 1.0 - +In.uv.y;"\
        "return Out;"\
    "}"\
    "/* masked */"\
    "VS_OUT VertMasked(VS_IN In) {"\
        "VS_OUT Out = (VS_OUT)0;"\
        "Out.Position = mul(float4(In.pos, 0.0f, 1.0f), projectMatrix);"\
        "Out.clipPosition = mul(float4(In.pos, 0.0f, 1.0f), clipMatrix);"\
        "Out.uv.x = In.uv.x;"\
        "Out.uv.y = 1.0 - In.uv.y;"\
        "return Out;"\
    "}"\
    \
"/* Pixel Shader */"\
    "/* normal */"\
    "float4 PixelNormal(VS_OUT In) : SV_Target{"\
        "float4 texColor = mainTexture.Sample(mainSampler, In.uv);"\
        "texColor.rgb = texColor.rgb * multiplyColor.rgb;"\
        "texColor.rgb = (texColor.rgb + screenColor.rgb) - (texColor.rgb * screenColor.rgb);"\
        "float4 color = texColor * baseColor;"\
        "color.xyz *= color.w;"\
        "return color;"\
    "}"\
    \
    "/* normal premult alpha */"\
    "float4 PixelNormalPremult(VS_OUT In) : SV_Target{"\
        "float4 texColor = mainTexture.Sample(mainSampler, In.uv);"\
        "texColor.rgb = texColor.rgb * multiplyColor.rgb;"\
        "texColor.rgb = (texColor.rgb + screenColor.rgb * texColor.a) - (texColor.rgb * screenColor.rgb);"\
        "float4 color = texColor * baseColor;"\
        "return color;"\
    "}"\
    \
    "/* masked */\n"\
    "float4 PixelMasked(VS_OUT In) : SV_Target{\n"\
        "float4 texColor = mainTexture.Sample(mainSampler, In.uv);"\
        "texColor.rgb = texColor.rgb * multiplyColor.rgb;"\
        "texColor.rgb = (texColor.rgb + screenColor.rgb) - (texColor.rgb * screenColor.rgb);"\
        "float4 color = texColor * baseColor;"\
        "color.xyz *= color.w;\n"\
        "float4 clipMask = (1.0 - maskTexture.Sample(mainSampler, In.clipPosition.xy / In.clipPosition.w)) * channelFlag;\n"\
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;\n"\
        "color = color * maskVal;\n"\
        "return color;\n"\
    "}"\
    "/* masked inverted*/\n"\
    "float4 PixelMaskedInverted(VS_OUT In) : SV_Target{\n"\
        "float4 texColor = mainTexture.Sample(mainSampler, In.uv);"\
        "texColor.rgb = texColor.rgb * multiplyColor.rgb;"\
        "texColor.rgb = (texColor.rgb + screenColor.rgb) - (texColor.rgb * screenColor.rgb);"\
        "float4 color = texColor * baseColor;"\
        "color.xyz *= color.w;\n"\
        "float4 clipMask = (1.0 - maskTexture.Sample(mainSampler, In.clipPosition.xy / In.clipPosition.w)) * channelFlag;\n"\
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;\n"\
        "color = color * (1.0 - maskVal);\n"\
        "return color;\n"\
    "}"\
    "/* masked premult alpha */\n"\
    "float4 PixelMaskedPremult(VS_OUT In) : SV_Target{\n"\
        "float4 texColor = mainTexture.Sample(mainSampler, In.uv);"\
        "texColor.rgb = texColor.rgb * multiplyColor.rgb;"\
        "texColor.rgb = (texColor.rgb + screenColor.rgb * texColor.a) - (texColor.rgb * screenColor.rgb);"\
        "float4 color = texColor * baseColor;\n"\
        "float4 clipMask = (1.0 - maskTexture.Sample(mainSampler, In.clipPosition.xy / In.clipPosition.w)) * channelFlag;\n"\
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;\n"\
        "color = color * maskVal;\n"\
        "return color;\n"\
    "}"\
    "/* masked inverted premult alpha */\n"\
    "float4 PixelMaskedInvertedPremult(VS_OUT In) : SV_Target{\n"\
        "float4 texColor = mainTexture.Sample(mainSampler, In.uv);"\
        "texColor.rgb = texColor.rgb * multiplyColor.rgb;"\
        "texColor.rgb = (texColor.rgb + screenColor.rgb * texColor.a) - (texColor.rgb * screenColor.rgb);"\
        "float4 color = texColor * baseColor;\n"\
        "float4 clipMask = (1.0 - maskTexture.Sample(mainSampler, In.clipPosition.xy / In.clipPosition.w)) * channelFlag;\n"\
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;\n"\
        "color = color * (1.0 - maskVal);\n"\
        "return color;\n"\
    "}\n";


void CubismShader_D3D11::ReleaseShaderProgram()
{
    if (_vertexFormat)
    {
        _vertexFormat->Release();
        _vertexFormat = NULL;
    }

    for (csmInt32 i = 0; i < ShaderNames_Max; i++)
    {
        if(_shaderSetsVS[i])
        {
            _shaderSetsVS[i]->Release();
            _shaderSetsVS[i] = NULL;
        }

        if (_shaderSetsPS[i])
        {
            _shaderSetsPS[i]->Release();
            _shaderSetsPS[i] = NULL;
        }
    }
}

CubismShader_D3D11::CubismShader_D3D11()
    : _vertexFormat(NULL)
{
    for (csmInt32 i = 0; i < ShaderNames_Max; i++)
    {
        _shaderSetsVS.PushBack(NULL);
        _shaderSetsPS.PushBack(NULL);
    }
}

CubismShader_D3D11::~CubismShader_D3D11()
{
    ReleaseShaderProgram();

    _shaderSetsVS.Clear();
    _shaderSetsPS.Clear();
}

void CubismShader_D3D11::GenerateShaders(ID3D11Device* device)
{
    if(_vertexFormat!=NULL)
    {
        return;
    }

    ReleaseShaderProgram();

    csmBool isSuccess = false;
    do
    {
        if(!LoadShaderProgram(device, false, ShaderNames_SetupMask, static_cast<const csmChar*>("VertSetupMask")))
        {
            break;
        }
        if (!LoadShaderProgram(device, true, ShaderNames_SetupMask, static_cast<const csmChar*>("PixelSetupMask")))
        {
            break;
        }

        if (!LoadShaderProgram(device, false, ShaderNames_Normal, static_cast<const csmChar*>("VertNormal")))
        {
            break;
        }
        if (!LoadShaderProgram(device, false, ShaderNames_NormalMasked, static_cast<const csmChar*>("VertMasked")))
        {
            break;
        }

        if (!LoadShaderProgram(device, true, ShaderNames_Normal, static_cast<const csmChar*>("PixelNormal")))
        {
            break;
        }
        if (!LoadShaderProgram(device, true, ShaderNames_NormalMasked, static_cast<const csmChar*>("PixelMasked")))
        {
            break;
        }
        if (!LoadShaderProgram(device, true, ShaderNames_NormalMaskedInverted, static_cast<csmChar*>("PixelMaskedInverted")))
        {
            break;
        }
        if (!LoadShaderProgram(device, true, ShaderNames_NormalPremultipliedAlpha, static_cast<csmChar*>("PixelNormalPremult")))
        {
            break;
        }
        if (!LoadShaderProgram(device, true, ShaderNames_NormalMaskedPremultipliedAlpha, static_cast<const csmChar*>("PixelMaskedPremult")))
        {
            break;
        }
        if (!LoadShaderProgram(device, true, ShaderNames_NormalMaskedInvertedPremultipliedAlpha, static_cast<csmChar*>("PixelMaskedInvertedPremult")))
        {
            break;
        }

        isSuccess = true;
    } while (0);

    if(!isSuccess)
    {
        CubismLogError("Fail Compile shader");
        CSM_ASSERT(0);
        return;
    }



    UINT compileFlag = 0;
#ifdef CSM_DEBUG
    compileFlag |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* layoutError = NULL;
    ID3DBlob* layoutBlobr = NULL;
    HRESULT hr = D3DCompile(
        CubismShaderEffectSrc,
        strlen(CubismShaderEffectSrc),
        NULL,
        NULL,
        NULL,
        "VertNormal",
        "vs_4_0",
        compileFlag,
        0,
        &layoutBlobr,
        &layoutError);
    if (FAILED(hr))
    {
        CubismLogError("Fail create input layout");
        CSM_ASSERT(0);
    }
    else
    {
        D3D11_INPUT_ELEMENT_DESC elems[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = device->CreateInputLayout(elems, ARRAYSIZE(elems), layoutBlobr->GetBufferPointer(), layoutBlobr->GetBufferSize(), &_vertexFormat);

        if (FAILED(hr))
        {
            CubismLogWarning("CreateVertexDeclaration failed");
        }
    }

    if (layoutError)
    {
        layoutError->Release();
    }
    if (layoutBlobr)
    {
        layoutBlobr->Release();
    }
}

Csm::csmBool CubismShader_D3D11::LoadShaderProgram(ID3D11Device* device, bool isPs, csmInt32 assign, const csmChar* entryPoint)
{
    csmBool bRet = false;
    if (!device) return false;

    ID3DBlob* errorBlob = NULL;
    ID3DBlob* vertexBlob = NULL;

    ID3D11VertexShader*     vertexShader = NULL;
    ID3D11PixelShader*      pixelShader = NULL;

    HRESULT hr = S_OK;
    do
    {
        UINT compileFlag = 0;
#ifdef CSM_DEBUG
        compileFlag |= D3DCOMPILE_DEBUG;
#endif

        hr = D3DCompile(
            CubismShaderEffectSrc,
            strlen(CubismShaderEffectSrc),
            NULL,
            NULL,
            NULL,
            entryPoint,
            isPs ? "ps_4_0" :"vs_4_0",
            compileFlag,
            0,
            &vertexBlob,
            &errorBlob);
        if (FAILED(hr))
        {
            CubismLogWarning("Fail Compile Shader : %s", entryPoint==NULL ? "" : entryPoint);
            break;
        }
        hr = isPs ? device->CreatePixelShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), NULL, &pixelShader) :
            device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), NULL, &vertexShader);
        if (FAILED(hr))
        {
            CubismLogWarning("Fail Create Shader");
            break;
        }

        if(isPs)
        {
            _shaderSetsPS[assign] = pixelShader;
        }
        else
        {
            _shaderSetsVS[assign] = vertexShader;
        }
        bRet = true;
    } while (0);

    if(errorBlob)
    {
        errorBlob->Release();
    }
    if (vertexBlob)
    {
        vertexBlob->Release();
    }

    return bRet;
}

ID3D11VertexShader* CubismShader_D3D11::GetVertexShader(csmUint32 assign)
{
    if(assign<ShaderNames_Max)
    {
        return _shaderSetsVS[assign];
    }

    return NULL;
}

ID3D11PixelShader* CubismShader_D3D11::GetPixelShader(csmUint32 assign)
{
    if (assign<ShaderNames_Max)
    {
        return _shaderSetsPS[assign];
    }

    return NULL;
}

void CubismShader_D3D11::SetupShader(ID3D11Device* device, ID3D11DeviceContext* renderContext)
{
    GenerateShaders(device);

    if (!renderContext || !_vertexFormat) return;

    renderContext->IASetInputLayout(_vertexFormat);
}

}}}}


