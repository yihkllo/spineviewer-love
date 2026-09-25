#pragma once
#include "core/viewer_controller.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QQuickWindow>
#include <algorithm>

inline int viewerLoadBenchmark(const QString& manifest,const QString& destination){
    QFile source(manifest);if(!source.open(QIODevice::ReadOnly))return 2;
    const auto rows=QJsonDocument::fromJson(source.readAll()).array();
    QCoreApplication::instance()->setProperty("qaProfileLoads",true);
    slqt::ViewerController controller;QQuickWindow window;window.resize(1440,810);
    controller.setWindow(&window);controller.setViewport({1100,810},2);
    QJsonArray report;
    for(const auto& row:rows){
        const auto item=row.toObject();const auto path=item["path"].toString();
        QMap<QString,QList<double>> times;QVariantMap first;
        for(int sample=0;sample<10;++sample){
            controller.openPaths({path},true);
            if(!controller.state()["loaded"].toBool()||!controller.state()["lastError"].toString().isEmpty())return 3;
            const auto stages=controller.property("_qaLoadTiming").toMap();
            if(sample==0)first=stages;else for(auto it=stages.begin();it!=stages.end();++it)times[it.key()].append(it.value().toDouble());
        }
        QJsonObject entry{{"id",item["id"]},{"version",item["version"]},
            {"firstOpen",QJsonObject::fromVariantMap(first)}};
        for(auto it=times.begin();it!=times.end();++it){auto values=it.value();std::sort(values.begin(),values.end());entry[it.key()]=values[values.size()/2];}
        report.append(entry);
    }
    QFile file(destination);if(!file.open(QIODevice::WriteOnly))return 4;
    file.write(QJsonDocument(report).toJson());return 0;
}
