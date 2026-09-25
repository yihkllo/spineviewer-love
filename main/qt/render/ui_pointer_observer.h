#pragma once
#include <QQuickItem>
#include <QPointer>
#include <QQuickWindow>

namespace slqt {
class UiPointerObserver : public QQuickItem {
    Q_OBJECT
public:
    explicit UiPointerObserver(QQuickItem* parent=nullptr);
    ~UiPointerObserver() override;
signals:
    void pressed(const QPointF& position);
protected:
    bool eventFilter(QObject* watched,QEvent* event) override;
private:
    QPointer<QQuickWindow> m_observedWindow;
};
}
