#pragma once
#include "spinelove/scene_snapshot.h"
#include <QPointF>
#include <algorithm>
#include <cmath>

namespace slqt {
inline double textureAlphaAt(const QImage& image,double u,double v)
{
    if(image.isNull()||!std::isfinite(u)||!std::isfinite(v))return 0;
    const double x=std::clamp(u*image.width()-0.5,0.0,double(image.width()-1));
    const double y=std::clamp(v*image.height()-0.5,0.0,double(image.height()-1));
    const int x0=int(x),y0=int(y),x1=std::min(x0+1,image.width()-1),y1=std::min(y0+1,image.height()-1);
    const double fx=x-x0,fy=y-y0;
    const double top=image.pixelColor(x0,y0).alphaF()*(1-fx)+image.pixelColor(x1,y0).alphaF()*fx;
    const double bottom=image.pixelColor(x0,y1).alphaF()*(1-fx)+image.pixelColor(x1,y1).alphaF()*fx;
    return top*(1-fy)+bottom*fy;
}

template<class Draw>
double drawAlphaAt(const SceneSnapshot& frame,const Draw& draw,QPointF point)
{
    const auto texture=frame.textures.constFind(SlTextureId(draw.textureId));
    if(texture==frame.textures.cend())return 0;
    double coverage=0;
    for(size_t i=0;i+2<draw.indices.size();i+=3){
        const auto ia=draw.indices[i],ib=draw.indices[i+1],ic=draw.indices[i+2];
        if(ia>=draw.vertices.size()||ib>=draw.vertices.size()||ic>=draw.vertices.size())continue;
        const auto &a=draw.vertices[ia],&b=draw.vertices[ib],&c=draw.vertices[ic];
        const double x=point.x(),y=point.y();
        if(x<std::min({a.pos.x,b.pos.x,c.pos.x})||x>std::max({a.pos.x,b.pos.x,c.pos.x})||
           y<std::min({a.pos.y,b.pos.y,c.pos.y})||y>std::max({a.pos.y,b.pos.y,c.pos.y}))continue;
        const double denominator=(b.pos.y-c.pos.y)*(a.pos.x-c.pos.x)+(c.pos.x-b.pos.x)*(a.pos.y-c.pos.y);
        if(!std::isfinite(denominator)||std::abs(denominator)<1e-8)continue;
        const double wa=((b.pos.y-c.pos.y)*(x-c.pos.x)+(c.pos.x-b.pos.x)*(y-c.pos.y))/denominator;
        const double wb=((c.pos.y-a.pos.y)*(x-c.pos.x)+(a.pos.x-c.pos.x)*(y-c.pos.y))/denominator;
        const double wc=1-wa-wb;
        if(wa<-1e-5||wb<-1e-5||wc<-1e-5)continue;
        const double opacity=std::clamp(wa*a.color.a+wb*b.color.a+wc*c.color.a,0.0,1.0);
        const double alpha=opacity*textureAlphaAt(*texture,wa*a.uv.x+wb*b.uv.x+wc*c.uv.x,wa*a.uv.y+wb*b.uv.y+wc*c.uv.y);
        coverage+=alpha*(1-coverage);
        if(coverage>=0.999)return coverage;
    }
    return coverage;
}

inline bool sceneHitTest(const SceneSnapshot& frame,QPointF point)
{
    if(point.x()<0||point.y()<0||point.x()>=frame.size.width()||point.y()>=frame.size.height())return false;
    double coverage=0;
    for(auto it=frame.draws.crbegin();it!=frame.draws.crend();++it){
        double alpha=drawAlphaAt(frame,*it,point);
        if(alpha<=0)continue;
        if(!it->masks.empty()){
            double maskAlpha=0;
            for(const auto& mask:it->masks){const double value=drawAlphaAt(frame,mask,point);maskAlpha+=value*(1-maskAlpha);}
            alpha*=it->invertedMask?1-maskAlpha:maskAlpha;
        }
        coverage+=alpha*(1-coverage);
        if(coverage>=0.03)return true;
    }
    return false;
}
}
