#pragma once
#include "spinelove/scene_snapshot.h"
#include <QObject>
#include <QSizeF>
#include "spinelove/sdk_api.h"

namespace slqt {
class SL_SDK_API SceneSource : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual void setViewport(QSizeF logical,qreal dpr)=0;
    virtual QSize preferredViewport()const{return {};}
    virtual std::shared_ptr<const SceneSnapshot> snapshot()const=0;
signals:
    void frameChanged();
};
}
