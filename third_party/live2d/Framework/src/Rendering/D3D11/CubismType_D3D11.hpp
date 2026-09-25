

#pragma once

namespace Live2D { namespace Cubism { namespace Framework {


struct CubismVertexD3D11
{
    float x, y;
    float u, v;
};

struct CubismConstantBufferD3D11
{
    DirectX::XMFLOAT4X4 projectMatrix;
    DirectX::XMFLOAT4X4 clipMatrix;
    DirectX::XMFLOAT4 baseColor;
    DirectX::XMFLOAT4 multiplyColor;
    DirectX::XMFLOAT4 screenColor;
    DirectX::XMFLOAT4 channelFlag;
};

}}}
