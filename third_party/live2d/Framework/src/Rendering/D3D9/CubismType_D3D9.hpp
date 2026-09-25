

#pragma once

namespace Live2D { namespace Cubism { namespace Framework {


struct CubismVertexD3D9
{
    float x, y;
    float u, v;
};
enum CubismD3D9VertexElem
{
    CubismD3D9VertexElem_Position = 0,
    CubismD3D9VertexElem_UV,
    CubismD3D9VertexElem_Max
};

}}}
