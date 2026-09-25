#pragma once
#include "viewer_controller.h"
#include <QQuickWindow>
#include <QScreen>
#include <QMutex>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <algorithm>
#include <cstring>

namespace slqt {
inline void installFrameProbe(QCoreApplication& app,QQuickWindow* window,ViewerController& controller,
    const QString& output,int durationMs=6500){
    struct Sample {double ms,time;quint64 pose;};
    struct Samples {QMutex mutex;QElapsedTimer clock;std::vector<Sample> frames;double time=0;quint64 pose=0;};
    auto samples=std::make_shared<Samples>();samples->clock.start();
    QObject::connect(window,&QQuickWindow::afterSynchronizing,window,[samples,&controller]{
        const auto snapshot=controller.snapshot();if(!snapshot)return;
        quint64 hash=1469598103934665603ull;
        auto add=[&](quint32 value){hash=(hash^value)*1099511628211ull;};
        for(const auto& draw:snapshot->draws){
            add(quint32(draw.vertices.size()));add(quint32(draw.indices.size()));
            for(const auto& vertex:draw.vertices)for(float value:{vertex.pos.x,vertex.pos.y,vertex.uv.x,vertex.uv.y,vertex.color.a}){
                quint32 bits;std::memcpy(&bits,&value,sizeof(bits));add(bits);
            }
        }
        QMutexLocker lock(&samples->mutex);samples->time=snapshot->animationTime;samples->pose=hash;
    },Qt::DirectConnection);
    QObject::connect(window,&QQuickWindow::frameSwapped,window,[samples]{
        QMutexLocker lock(&samples->mutex);
        samples->frames.push_back({samples->clock.nsecsElapsed()/1e6,samples->time,samples->pose});
    },Qt::DirectConnection);
    QTimer::singleShot(durationMs,&app,[samples,window,output,&app]{
        std::vector<Sample> frames;
        {QMutexLocker lock(&samples->mutex);for(const auto& frame:samples->frames)if(frame.ms>=1500)frames.push_back(frame);}
        QJsonObject report{{"screenHz",window->screen()->refreshRate()},{"presentedFrames",int(frames.size())},
            {"width",qRound(window->width()*window->devicePixelRatio())},{"height",qRound(window->height()*window->devicePixelRatio())}};
        if(frames.size()>1){
            const double seconds=(frames.back().ms-frames.front().ms)/1000.;
            int animationUpdates=0,poseChanges=0;std::vector<double> intervals;
            for(size_t i=1;i<frames.size();++i){
                animationUpdates+=frames[i].time!=frames[i-1].time;poseChanges+=frames[i].pose!=frames[i-1].pose;
                intervals.push_back(frames[i].ms-frames[i-1].ms);
            }
            std::sort(intervals.begin(),intervals.end());
            report["seconds"]=seconds;report["presentedFps"]=(frames.size()-1)/seconds;
            report["animationUpdatesPerSecond"]=animationUpdates/seconds;report["poseChangesPerSecond"]=poseChanges/seconds;
            report["animationSecondsPerWallSecond"]=(frames.back().time-frames.front().time)/seconds;
            report["medianFrameMs"]=intervals[intervals.size()/2];report["p95FrameMs"]=intervals[intervals.size()*95/100];
            report["p99FrameMs"]=intervals[intervals.size()*99/100];report["maxFrameMs"]=intervals.back();
        }
        QJsonArray times;for(const auto& frame:frames)times.append(frame.ms);report["frameTimesMs"]=times;
        QFile file(output);const bool ok=file.open(QIODevice::WriteOnly)&&file.write(QJsonDocument(report).toJson())>0;
        app.exit(ok&&frames.size()>1?0:5);
    });
}
}
