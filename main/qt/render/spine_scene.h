#pragma once
#include <QQuickRhiItem>
#include <memory>
#include "spinelove/scene_snapshot.h"
#include "spinelove/scene_source.h"
#include <QPointer>
#include "../core/viewer_controller.h"

namespace slqt {
class SpineScene : public QQuickRhiItem {
    Q_OBJECT
    Q_PROPERTY(slqt::ViewerController* controller READ controller WRITE setController NOTIFY controllerChanged)
    Q_PROPERTY(QObject* frameSource READ frameSource WRITE setFrameSource NOTIFY frameSourceChanged)
    Q_PROPERTY(qreal renderScale READ renderScale WRITE setRenderScale NOTIFY renderScaleChanged)
public:
    explicit SpineScene(QQuickItem* parent = nullptr);
    ViewerController* controller() const { return m_controller; }
    void setController(ViewerController* controller);
    QObject* frameSource()const{return m_frameSource;}
    void setFrameSource(QObject* source);
    qreal renderScale()const{return m_renderScale;}
    void setRenderScale(qreal scale);
    quint64 sourceGeneration()const{return m_sourceGeneration;}
    bool contains(const QPointF& point) const override;
    std::shared_ptr<const SceneSnapshot> snapshot() const;
signals:
    void controllerChanged();
    void frameSourceChanged();
    void renderScaleChanged();
protected:
    QQuickRhiItemRenderer* createRenderer() override;
    void geometryChange(const QRectF& now, const QRectF& before) override;
    void itemChange(ItemChange change, const ItemChangeData& data) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void hoverMoveEvent(QHoverEvent*) override;
    void hoverLeaveEvent(QHoverEvent*) override;
private:
    void updateViewport();
    ViewerController* m_controller = nullptr;
    QPointer<SceneSource> m_frameSource;
    quint64 m_sourceGeneration=0;
    qreal m_renderScale=1;
};
}
