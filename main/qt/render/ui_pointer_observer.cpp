#include "ui_pointer_observer.h"
#include <QMouseEvent>
#include <QTouchEvent>

namespace slqt {
UiPointerObserver::UiPointerObserver(QQuickItem* parent):QQuickItem(parent){
    connect(this,&QQuickItem::windowChanged,this,[this](QQuickWindow* window){
        if(m_observedWindow)m_observedWindow->removeEventFilter(this);
        m_observedWindow=window;
        if(window)window->installEventFilter(this);
    });
}
UiPointerObserver::~UiPointerObserver(){if(m_observedWindow)m_observedWindow->removeEventFilter(this);}
bool UiPointerObserver::eventFilter(QObject* watched,QEvent* event){
    if(watched!=m_observedWindow||!isVisible()||!isEnabled())return false;
    const auto report=[this](const QPointF& position){const auto local=mapFromScene(position);if(contains(local))emit pressed(local);};
    if(event->type()==QEvent::MouseButtonPress||event->type()==QEvent::MouseButtonDblClick){
        const auto* mouse=static_cast<QMouseEvent*>(event);
        if(mouse->button()==Qt::LeftButton&&mouse->source()!=Qt::MouseEventSynthesizedByQt)report(mouse->position());
    }else if(event->type()==QEvent::TouchBegin||event->type()==QEvent::TouchUpdate){
        for(const auto& point:static_cast<QTouchEvent*>(event)->points())if(point.state()==QEventPoint::State::Pressed)report(point.position());
    }
    return false;
}
}
