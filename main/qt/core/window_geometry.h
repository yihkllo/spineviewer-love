#pragma once
#include <QSize>
#include <algorithm>
#include <cmath>

namespace slqt {
inline QSize legacyClientSize(QSize requested,QSize frame,QSize work,bool borderless){
    const float fit=std::min(1.f,std::min(
        float(std::max(1,work.width()-frame.width()))/requested.width(),
        float(std::max(1,work.height()-frame.height()))/requested.height()));
    if(fit<1)requested=QSize(std::max(1,int(std::floor(requested.width()*fit))),
        std::max(1,int(std::floor(requested.height()*fit))));
    return borderless?requested+frame:requested;
}
}
