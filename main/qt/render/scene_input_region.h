#pragma once
#include "spinelove/scene_snapshot.h"
#include <QRectF>
#include <QRegion>
#include <algorithm>
#include <cmath>
#include <limits>

namespace slqt {
template<class Draw>
QRectF visibleDrawBounds(const Draw& draw)
{
    double left=std::numeric_limits<double>::infinity(),top=left;
    double right=-left,bottom=-left;
    bool visible=false;
    for(const auto index:draw.indices){
        if(index>=draw.vertices.size())continue;
        const auto& vertex=draw.vertices[index];
        if(!std::isfinite(vertex.pos.x)||!std::isfinite(vertex.pos.y))continue;
        visible=visible||vertex.color.a>0.01f;
        left=std::min(left,double(vertex.pos.x));top=std::min(top,double(vertex.pos.y));
        right=std::max(right,double(vertex.pos.x));bottom=std::max(bottom,double(vertex.pos.y));
    }
    return visible&&right>left&&bottom>top?QRectF(QPointF(left,top),QPointF(right,bottom)):QRectF{};
}

inline QRegion paddedInputRegion(QRectF physical,qreal dpr,QSize viewport)
{
    if(physical.isEmpty()||dpr<=0)return {};
    physical=physical.adjusted(-8,-8,8,8).intersected(QRectF(QPointF{},QSizeF(viewport)));
    if(physical.isEmpty())return {};
    const int step=std::max(1,int(std::ceil(4/dpr)));
    const int left=int(std::floor(physical.left()/dpr/step))*step;
    const int top=int(std::floor(physical.top()/dpr/step))*step;
    const int right=int(std::ceil(physical.right()/dpr/step))*step;
    const int bottom=int(std::ceil(physical.bottom()/dpr/step))*step;
    return QRegion(QRect(left,top,std::max(1,right-left),std::max(1,bottom-top)));
}

inline QRegion sceneInputRegion(const SceneSnapshot& frame,qreal dpr)
{
    QRegion region;
    for(const auto& draw:frame.draws){
        if(!frame.textures.contains(SlTextureId(draw.textureId)))continue;
        QRectF bounds=visibleDrawBounds(draw);
        if(!draw.masks.empty()&&!draw.invertedMask){
            QRectF maskBounds;
            for(const auto& mask:draw.masks)maskBounds=maskBounds.united(visibleDrawBounds(mask));
            bounds=bounds.intersected(maskBounds);
        }
        region|=paddedInputRegion(bounds,dpr,frame.size);
    }
    return region;
}

inline QRectF sceneVisibleBounds(const SceneSnapshot& frame)
{
    QRectF bounds;
    for(const auto& draw:frame.draws){
        if(!frame.textures.contains(SlTextureId(draw.textureId)))continue;
        QRectF part=visibleDrawBounds(draw);
        if(!draw.masks.empty()&&!draw.invertedMask){
            QRectF masks;
            for(const auto& mask:draw.masks)masks=masks.united(visibleDrawBounds(mask));
            part=part.intersected(masks);
        }
        bounds=bounds.united(part);
    }
    return bounds;
}
}
