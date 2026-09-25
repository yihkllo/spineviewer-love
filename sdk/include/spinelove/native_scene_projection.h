#pragma once
#include "spinelove/sl_gfx_types.h"
#include <QMatrix4x4>
#include <QSize>
#include <QVariantMap>
#include <algorithm>
#include <cmath>

namespace slqt {
inline QMatrix4x4 nativeMatrix4(const QVariantList& values){
    QMatrix4x4 result;
    if(values.size()==16)for(int row=0;row<4;++row)for(int column=0;column<4;++column)result(row,column)=values[column*4+row].toFloat();
    return result;
}
inline QMatrix4x4 nativeViewProjection(const QVariantMap& camera,const QSize& viewport){
    const float aspect=camera.value("aspect",0).toFloat()>0?camera.value("aspect").toFloat():float(viewport.width())/std::max(1,viewport.height());
    const float nearPlane=std::max(.0001f,camera.value("near",.1).toFloat()),farPlane=std::max(nearPlane+.01f,camera.value("far",1000).toFloat());
    QMatrix4x4 projection;
    if(camera.value("perspective",true).toBool())projection.perspective(std::clamp(camera.value("fieldOfView",60).toFloat(),1.f,179.f),aspect,nearPlane,farPlane);
    else {const float size=std::max(.0001f,camera.value("size",5).toFloat());projection.ortho(-size*aspect,size*aspect,-size,size,nearPlane,farPlane);}
    QMatrix4x4 forward;forward.scale(1,1,-1);
    return projection*forward*nativeMatrix4(camera.value("worldMatrix").toList()).inverted();
}
inline void nativeProjectVertex(SlVertex2D& vertex,const QVector3D& local,const QMatrix4x4& matrix,const QSize& viewport){
    const auto clip=matrix*QVector4D(local,1);
    const float w=std::abs(clip.w())<1e-7f?(clip.w()<0?-1e-7f:1e-7f):clip.w();
    vertex.rhw=1.f/w;
    vertex.pos={float((clip.x()/w*.5f+.5f)*viewport.width()),float((.5f-clip.y()/w*.5f)*viewport.height()),-clip.z()/w};
}
}
