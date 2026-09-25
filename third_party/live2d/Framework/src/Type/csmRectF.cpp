

#include "csmRectF.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

csmRectF::csmRectF()
{ }

csmRectF::csmRectF(csmFloat32 x, csmFloat32 y, csmFloat32 w, csmFloat32 h)
    : X(x)
    , Y(y)
    , Width(w)
    , Height(h)
{ }

csmRectF::~csmRectF()
{ }

void csmRectF::SetRect(csmRectF* r)
{
    X = r->X;
    Y = r->Y;
    Width = r->Width;
    Height = r->Height;
}

void csmRectF::Expand(csmFloat32 w, csmFloat32 h)
{
    X -= w;
    Y -= h;
    Width += w * 2.0f;
    Height += h * 2.0f;
}

}}}
