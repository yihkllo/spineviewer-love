#include "viewer_controller.h"
#include "window_geometry.h"
#include "../render/scene_input_region.h"
#include "window_resolution_presets.h"
#include <QQuickWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QCursor>
#include <QDateTime>
#include <QRandomGenerator>
#include <QQuickItem>
#include <QScopedValueRollback>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <cmath>
#include <algorithm>
#if defined(Q_OS_WIN)
#define NOMINMAX
#include <Windows.h>
#endif

namespace slqt {
namespace {
QSize legacyFrameAllowance(QQuickWindow* window,bool nativeFrame,bool resizeEnabled){
#if defined(Q_OS_WIN)
    if(QGuiApplication::platformName()=="windows"){
        DWORD style=WS_POPUP|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX;
        if(nativeFrame)style|=WS_CAPTION;
        if(resizeEnabled)style|=WS_THICKFRAME;
        RECT r{};const auto dpi=GetDpiForWindow(reinterpret_cast<HWND>(window->winId()));
        AdjustWindowRectExForDpi(&r,style,FALSE,WS_EX_ACCEPTFILES,dpi?dpi:96);
        return {r.right-r.left,r.bottom-r.top};
    }
#endif
    return {0,0};
}
void resizeLikeLegacy(QQuickWindow* window,QSize physical,bool nativeFrame,bool resizeEnabled){
    const qreal dpr=window->devicePixelRatio();
    const QRect work=window->screen()->availableGeometry();
    const QSize frame=legacyFrameAllowance(window,nativeFrame,resizeEnabled);
    const QSize client=legacyClientSize(physical,frame,
        QSize(qRound(work.width()*dpr),qRound(work.height()*dpr)),!nativeFrame);
    const QSize size(qRound(client.width()/dpr),qRound(client.height()/dpr));
    const auto margins=window->frameMargins();
    const QSize outer=size+QSize(margins.left()+margins.right(),margins.top()+margins.bottom());
    window->setGeometry(QRect(work.topLeft()+QPoint((work.width()-outer.width())/2+margins.left(),
        (work.height()-outer.height())/2+margins.top()),size));
}
#if defined(Q_OS_WIN)
bool writeNativeStyle(HWND hwnd,int index,LONG_PTR value){
    SetLastError(ERROR_SUCCESS);
    const auto previous=SetWindowLongPtrW(hwnd,index,value);
    return previous!=0||GetLastError()==ERROR_SUCCESS;
}
bool setNativePetFrame(QQuickWindow* window){
    const auto hwnd=reinterpret_cast<HWND>(window->winId());
    const LONG_PTR style=GetWindowLongPtrW(hwnd,GWL_STYLE);
    const LONG_PTR clean=(style&~(WS_CAPTION|WS_THICKFRAME|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX))|WS_POPUP;
    if(!writeNativeStyle(hwnd,GWL_STYLE,clean))return false;
    const LONG_PTR extended=GetWindowLongPtrW(hwnd,GWL_EXSTYLE);
    if(!writeNativeStyle(hwnd,GWL_EXSTYLE,(extended&~WS_EX_APPWINDOW)|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE))return false;
    return SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_FRAMECHANGED)!=FALSE;
}
bool setNativeResizeFrame(QQuickWindow* window,bool enabled){
    const auto hwnd=reinterpret_cast<HWND>(window->winId());
    const LONG_PTR style=GetWindowLongPtrW(hwnd,GWL_STYLE);
    if(!writeNativeStyle(hwnd,GWL_STYLE,enabled?(style|WS_THICKFRAME):(style&~WS_THICKFRAME)))return false;
    if(SetWindowPos(hwnd,nullptr,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED))return true;
    writeNativeStyle(hwnd,GWL_STYLE,style);
    SetWindowPos(hwnd,nullptr,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    return false;
}
bool setNativeColorKey(QQuickWindow* window,bool enabled,bool topmost){
    const auto hwnd=reinterpret_cast<HWND>(window->winId());
    const LONG_PTR style=GetWindowLongPtrW(hwnd,GWL_EXSTYLE);
    COLORREF oldKey=0;BYTE oldAlpha=255;DWORD oldFlags=0;
    const bool hadAttributes=GetLayeredWindowAttributes(hwnd,&oldKey,&oldAlpha,&oldFlags)!=FALSE;
    const bool changed=writeNativeStyle(hwnd,GWL_EXSTYLE,enabled?(style|WS_EX_LAYERED):(style&~WS_EX_LAYERED));
    const bool attributed=changed&&(!enabled||SetLayeredWindowAttributes(hwnd,RGB(0,0,0),255,LWA_COLORKEY));
    if(attributed&&SetWindowPos(hwnd,topmost?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_FRAMECHANGED))return true;
    writeNativeStyle(hwnd,GWL_EXSTYLE,style);
    if(hadAttributes)SetLayeredWindowAttributes(hwnd,oldKey,oldAlpha,oldFlags);
    SetWindowPos(hwnd,(style&WS_EX_TOPMOST)?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    return false;
}
void captureNativeWindowPolicy(QQuickWindow* window){
    const auto hwnd=reinterpret_cast<HWND>(window->winId());
    COLORREF key=0;BYTE alpha=0;DWORD flags=0;
    window->setProperty("_slNativeResizeEffective",(GetWindowLongPtrW(hwnd,GWL_STYLE)&WS_THICKFRAME)!=0);
    window->setProperty("_slNativeTopmostEffective",(GetWindowLongPtrW(hwnd,GWL_EXSTYLE)&WS_EX_TOPMOST)!=0);
    window->setProperty("_slNativeColorKeyEffective",GetLayeredWindowAttributes(hwnd,&key,&alpha,&flags)&&(flags&LWA_COLORKEY));
}
#endif
}
bool ViewerController::windowCommand(const QString& c,const QVariant& v){
    if(!m_window)return false;
    if(c=="pet.menu"){
        const auto point=v.toMap();showDesktopPetMenu(QPointF(point.value("x").toDouble(),point.value("y").toDouble()));return true;
    }
    if(c=="plugin.portrait"){
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
        const bool enabled=v.toBool();
        if(enabled==m_pluginPortrait)return true;
        m_pluginPortrait=enabled;
        if(enabled){
            m_pluginPortraitReturnGeometry={};
            if(m_window->visibility()==QWindow::Windowed){
                m_pluginPortraitReturnGeometry=m_window->geometry();
                m_pluginPortraitReturnVisibility=int(m_window->visibility());
                const int height=qRound(m_window->height()*m_window->devicePixelRatio());
                resizeLikeLegacy(m_window,QSize(qRound(height/2.),height),m_nativeFrame,m_resizeEnabled);
            }
        }else if(m_pluginPortraitReturnGeometry.isValid()){
            if(m_window->visibility()==QWindow::FullScreen){m_fullscreenReturnGeometry=m_pluginPortraitReturnGeometry;m_fullscreenReturnVisibility=m_pluginPortraitReturnVisibility;}
            else {m_window->showNormal();m_window->setGeometry(m_pluginPortraitReturnGeometry);}
            m_pluginPortraitReturnGeometry={};
        }
#endif
        return true;
    }
    if(c=="pet.enter"){enterDesktopPet();return true;}
    if(c=="pet.exit"){exitDesktopPet();return true;}
    if(c=="pet.random"){m_petRandom=v.toBool();m_nextPetMotion=0;}
    else if(c=="pet.next"){
        m_nextPetMotion=0;
        if(live2dMode())m_live2d->command("animation.step",1);
        else runtime()->StepToNextMotion();
    }
    else if(c=="window.resize"){if(m_resizeEnabled&&!m_petMode)m_window->startSystemResize(Qt::Edges(v.toInt()));}
    else if(c=="window.toggleResize"){
#if defined(Q_OS_WIN)
        if(!m_petMode){
            const auto hwnd=reinterpret_cast<HWND>(m_window->winId());
            if(m_resizeEnabled)m_window->setProperty("_slNativeResizeBeforeDisable",(GetWindowLongPtrW(hwnd,GWL_STYLE)&WS_THICKFRAME)!=0);
            const bool nativeEnabled=!m_resizeEnabled&&m_window->property("_slNativeResizeBeforeDisable").toBool();
            if(!setNativeResizeFrame(m_window,nativeEnabled)){fail(tr("Could not change the native window resize policy."));return true;}
        }
#endif
        m_resizeEnabled=!m_resizeEnabled;
    }
    else if(c=="window.invertWheel"){m_invertWheel=!m_invertWheel;m_wheelRemainder=0;}
    else if(c=="window.toggleChrome"){
        m_nativeFrame=!m_nativeFrame;m_window->setFlag(Qt::FramelessWindowHint,!m_nativeFrame);m_window->show();
#if defined(Q_OS_WIN)
        if(!m_resizeEnabled&&!m_petMode){
            m_window->setProperty("_slNativeResizeBeforeDisable",(GetWindowLongPtrW(reinterpret_cast<HWND>(m_window->winId()),GWL_STYLE)&WS_THICKFRAME)!=0);
            if(!setNativeResizeFrame(m_window,false))fail(tr("Could not retain the disabled window resize policy."));
        }
        if(m_clickThrough&&!m_petMode&&!setNativeColorKey(m_window,true,true))fail(tr("Could not retain native black color-key transparency."));
#endif
    }
    else if(c=="window.toggleClickThrough"){
#if defined(Q_OS_WIN)
        const bool enabled=!m_clickThrough;
        if(!setNativeColorKey(m_window,enabled,enabled)){fail(tr("Could not change native black color-key transparency."));return true;}
        m_clickThrough=enabled;
#endif
    }
    else if(c=="window.matchCanvas"){
        const float scale=runtime()->SkeletonScale();if(std::isfinite(scale)&&scale>.0001f){runtime()->SetBaseSize(m_viewport.width()/scale,m_viewport.height()/scale);fit();}
    }
    else if(c=="window.restoreCanvas"){runtime()->ClearBaseSize();fit();}
    else if(c=="settings.renderSize.reset"){
        if(m_petMode)return true;
        runtime()->ClearBaseSize();
        return windowCommand("settings.resolution",0);
    }
    else if(c=="settings.renderSize"){
        if(m_petMode)return true;
        const auto values=v.toMap();bool widthOk=false,heightOk=false;
        const int width=values.value("width").toInt(&widthOk),height=values.value("height").toInt(&heightOk);
        if(!widthOk||!heightOk||width<64||height<64||width>8192||height>8192||qint64(width)*height>33554432){
            fail(tr("Render size must be between 64 and 8192 pixels per side, up to 32 megapixels."));return true;
        }
        auto* desktop=m_window->findChild<QObject*>("desktopViewer");
        const double dpr=m_window->devicePixelRatio();
        const double currentWidth=m_window->width()*dpr;
        const double fallback=std::max(0.0,currentWidth-m_viewport.width())/std::clamp(currentWidth/1920.0,0.5,3.0);
        const double panel=std::clamp(desktop?desktop->property("renderPanelUnits").toDouble():fallback,0.0,1800.0);
        double fullWidth=width+panel*0.5;
        if(fullWidth>960.0){
            fullWidth=width/(1.0-panel/1920.0);
            if(fullWidth>5760.0)fullWidth=width+panel*3.0;
        }
        return windowCommand("settings.resolution.custom",QVariantMap{
            {"width",std::max(320,int(std::floor(fullWidth)))},{"height",std::max(240,height)}});
    }
    else if(c=="settings.resolution.custom"){
        if(m_petMode||!m_window->screen())return true;
        const auto values=v.toMap();bool widthOk=false,heightOk=false;
        const int width=values.value("width").toInt(&widthOk),height=values.value("height").toInt(&heightOk);
        if(!widthOk||!heightOk||width<320||height<240||width>16384||height>16384){
            fail(tr("Enter a width from 320 to 16384 and a height from 240 to 16384."));return true;
        }
        runtime()->ClearBaseSize();
        if(m_window->visibility()==QWindow::FullScreen)windowCommand("window.fullscreen",{});
        if(m_window->visibility()==QWindow::Maximized)m_window->showNormal();
        const qreal dpr=m_window->devicePixelRatio();
        const QRect work=m_window->screen()->availableGeometry();
        const auto margins=m_window->frameMargins();
        const int maxWidth=std::max(1,work.width()-margins.left()-margins.right());
        const int maxHeight=std::max(1,work.height()-margins.top()-margins.bottom());
        QSize size(qRound(width/dpr),qRound(height/dpr));
        const double factor=std::min(1.0,std::min(double(maxWidth)/size.width(),double(maxHeight)/size.height()));
        size=QSize(std::clamp(int(std::floor(size.width()*factor)),std::min(m_window->minimumWidth(),maxWidth),maxWidth),
                   std::clamp(int(std::floor(size.height()*factor)),std::min(m_window->minimumHeight(),maxHeight),maxHeight));
        const QSize outer=size+QSize(margins.left()+margins.right(),margins.top()+margins.bottom());
        m_window->setGeometry(QRect(work.topLeft()+QPoint((work.width()-outer.width())/2+margins.left(),
            (work.height()-outer.height())/2+margins.top()),size));
        m_state["resolutionPreset"]=-1;m_settings.setValue("resolutionPreset",-1);
        m_settings.setValue("customWindowWidth",qRound(m_window->width()*dpr));
        m_settings.setValue("customWindowHeight",qRound(m_window->height()*dpr));
        const double uiScale=std::clamp(m_window->width()*dpr/1920.0,0.5,3.0);
        m_state["uiScale"]=uiScale;m_state["baseFontPixels"]=16.0*uiScale;m_state["titleScale"]=uiScale;
        m_settings.setValue("uiScale",uiScale);m_settings.setValue("baseFontPixels",16.0*uiScale);m_settings.setValue("titleScale",uiScale);
    }
    else if(c=="settings.resolution"){
        const auto* preset=window_resolution_presets::Get(v.toInt());if(!preset)return true;
        runtime()->ClearBaseSize();
        if(m_window->visibility()==QWindow::FullScreen)windowCommand("window.fullscreen",{});
        if(m_defaultWindowSize.isEmpty())m_defaultWindowSize=m_window->size();
        m_state["resolutionPreset"]=v.toInt();m_settings.setValue("resolutionPreset",v.toInt());
        const qreal windowDpr=m_window->devicePixelRatio();
        QSize size=preset->width>0?QSize(qRound(preset->width/windowDpr),qRound(preset->height/windowDpr)):m_defaultWindowSize;
        const QRect available=m_window->screen()->availableGeometry();
        const double factor=std::min(1.0,std::min(double(available.width())/size.width(),double(available.height())/size.height()));
        size=QSize(std::max(1,int(std::floor(size.width()*factor))),std::max(1,int(std::floor(size.height()*factor))));
        if(preset->width>0)resizeLikeLegacy(m_window,QSize(preset->width,preset->height),m_nativeFrame,m_resizeEnabled);
        else{
            m_window->resize(size);
            m_window->setPosition(available.topLeft()+QPoint((available.width()-size.width())/2,(available.height()-size.height())/2));
        }
        const auto* primary=QGuiApplication::primaryScreen();
        const float defaultScale=primary?float(primary->geometry().width()*primary->devicePixelRatio()/1920.0):1.0f;
        const float uiScale=window_resolution_presets::UiScale(v.toInt(),defaultScale);
        m_state["baseFontPixels"]=16.0f*uiScale;m_state["titleScale"]=uiScale;m_state["uiScale"]=uiScale;
        m_settings.setValue("uiScale",uiScale);m_settings.setValue("baseFontPixels",16.0f*uiScale);m_settings.setValue("titleScale",uiScale);
    }
    else if(c=="window.fullscreen"){
        if(m_window->visibility()==QWindow::FullScreen){
            if(m_fullscreenReturnVisibility==int(QWindow::Maximized))m_window->showMaximized();
            else {m_window->showNormal();if(m_fullscreenReturnGeometry.isValid())m_window->setGeometry(m_fullscreenReturnGeometry);}
        }else{m_fullscreenReturnVisibility=int(m_window->visibility());m_fullscreenReturnGeometry=m_window->geometry();m_window->showFullScreen();}
#if defined(Q_OS_WIN)
        if(!m_resizeEnabled&&m_window->visibility()!=QWindow::FullScreen&&!m_petMode)setNativeResizeFrame(m_window,false);
        if(m_clickThrough&&!m_petMode&&!setNativeColorKey(m_window,true,true))fail(tr("Could not retain native black color-key transparency."));
#endif
    }
    else return false;
#if defined(Q_OS_WIN)
    captureNativeWindowPolicy(m_window);
#endif
    refresh();record();return true;
}
void ViewerController::enterDesktopPet(){
    if(m_petMode||!m_window||!m_state.value("loaded").toBool())return;
    m_petReturnMask=m_window->mask();m_petMask={};m_petCurrentRegion={};m_petCanvasOrigin={};
    m_petReturnGeometry=m_window->geometry();m_petReturnFlags=m_window->flags();m_petReturnMinimum=m_window->minimumSize();
    const QPoint clientOrigin=m_window->mapToGlobal(QPoint(0,0));
    auto* desktop=m_window->findChild<QObject*>("desktopViewer");
    const qreal panelWidth=desktop?desktop->property("canvasLeft").toReal():m_window->width()-m_viewport.width()/m_dpr;
    const int panel=std::clamp(qRound(panelWidth),0,qMax(0,m_window->width()-1));
    const QRect pet(clientOrigin+QPoint(panel,0),QSize(qMax(1,m_window->width()-panel),m_window->height()));
    m_petStageOrigin=pet.topLeft();m_petCanvasBounds={};
    m_window->setProperty("_slPetReturnVisibility",int(m_window->visibility()));
    m_window->setProperty("_slPetReturnColor",m_window->color());
    m_window->setProperty("_slPetReturnTitle",m_window->title());
    m_window->setProperty("_slPetPersistentGraphics",m_window->isPersistentGraphics());
    m_window->setProperty("_slPetPersistentSceneGraph",m_window->isPersistentSceneGraph());
#if defined(Q_OS_WIN)
    m_window->setProperty("_slPetReturnTopmost",(GetWindowLongPtrW(reinterpret_cast<HWND>(m_window->winId()),GWL_EXSTYLE)&WS_EX_TOPMOST)!=0);
#endif
    m_window->setPersistentGraphics(true);m_window->setPersistentSceneGraph(true);
    m_queuePlaying=false;m_live2d->command("queue.stop");m_live2d->command("pet.reset");m_petClock.restart();m_petMode=true;m_nextPetMotion=0;m_dragged=false;m_pointerMode=0;
    QCoreApplication::instance()->installEventFilter(this);
    m_state["petMode"]=true;publishState();
    m_window->setMinimumSize(QSize(1,1));m_window->setColor(Qt::transparent);
    m_window->setWindowState(Qt::WindowNoState);
    m_window->setProperty("_slPetReturnNormalGeometry",m_window->geometry());
    m_window->setFlags(Qt::Tool|Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::NoDropShadowWindowHint|Qt::WindowDoesNotAcceptFocus);
    m_petMask={};m_petCurrentRegion={};
    m_window->setTitle(QString());
    m_window->setGeometry(pet);m_window->show();m_clock.restart();refresh();record();
#if defined(Q_OS_WIN)
    if(!setNativePetFrame(m_window))fail(tr("Could not remove the desktop pet window frame."));
    QTimer::singleShot(0,this,[this]{
        if(m_petMode&&m_window){setNativePetFrame(m_window);captureNativeWindowPolicy(m_window);}
    });
    captureNativeWindowPolicy(m_window);
#endif
}
void ViewerController::exitDesktopPet(){
    if(!m_petMode||!m_window)return;
    if(m_petMenu)m_petMenu->close();
    m_petMoveTimer.stop();m_petMoveQueued=false;
    QCoreApplication::instance()->removeEventFilter(this);
    m_petMode=false;m_pointerMode=0;m_dragged=true;m_live2d->command("live2d.endDrag");m_live2d->command("pet.reset");
    if(auto* grabber=m_window->mouseGrabberItem())grabber->ungrabMouse();
    m_state["petMode"]=false;m_state["petDragging"]=false;publishState();
    m_window->setWindowState(Qt::WindowNoState);
    m_window->setFlags(m_petReturnFlags);m_window->setTitle(m_window->property("_slPetReturnTitle").toString());m_window->setColor(m_window->property("_slPetReturnColor").value<QColor>());m_window->setMinimumSize(m_petReturnMinimum);
    m_window->setMask(m_petReturnMask);m_petMask={};m_petCurrentRegion={};m_petCanvasOrigin={};
    const auto returnVisibility=QWindow::Visibility(m_window->property("_slPetReturnVisibility").toInt());
    const QRect normalGeometry=m_window->property("_slPetReturnNormalGeometry").toRect();
    m_window->setGeometry((returnVisibility==QWindow::FullScreen||returnVisibility==QWindow::Maximized)&&normalGeometry.isValid()
        ?normalGeometry:m_petReturnGeometry);
    if(returnVisibility==QWindow::FullScreen)m_window->showFullScreen();
    else if(returnVisibility==QWindow::Maximized)m_window->showMaximized();
    else m_window->showNormal();
#if defined(Q_OS_WIN)
    const auto hwnd=reinterpret_cast<HWND>(m_window->winId());
    if(m_clickThrough){
        if(!setNativeColorKey(m_window,true,m_window->property("_slPetReturnTopmost").toBool()))fail(tr("Could not restore native black color-key transparency."));
    }else SetWindowPos(hwnd,m_window->property("_slPetReturnTopmost").toBool()?HWND_TOPMOST:HWND_NOTOPMOST,
        0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    if(!m_resizeEnabled&&returnVisibility!=QWindow::FullScreen)setNativeResizeFrame(m_window,false);
    captureNativeWindowPolicy(m_window);
#endif
    m_window->setPersistentGraphics(m_window->property("_slPetPersistentGraphics").toBool());
    m_window->setPersistentSceneGraph(m_window->property("_slPetPersistentSceneGraph").toBool());
    m_window->requestActivate();m_clock.restart();refresh();record();
}
void ViewerController::updateDesktopPetCanvas(SceneSnapshot& frame){
    if(!m_petMode||live2dMode()||!m_window||m_petCanvasAdjusting)return;
    QScopedValueRollback<bool> adjusting(m_petCanvasAdjusting,true);
    const qreal dpr=m_window->devicePixelRatio();
    QRectF bounds=sceneVisibleBounds(frame);
    if(!bounds.isEmpty()){
        const QPointF baseScreen=m_petStageOrigin;
        QRect desktop;
        for(auto* screen:QGuiApplication::screens())desktop=desktop.united(screen->geometry());
        const QRectF visibleArea((QPointF(desktop.topLeft())-baseScreen)*dpr,QSizeF(desktop.size())*dpr);
        bounds=bounds.intersected(visibleArea.adjusted(-64*dpr,-64*dpr,64*dpr,64*dpr));
        if(!bounds.isEmpty()){
            if(m_petCanvasBounds.isEmpty()||!m_petCanvasBounds.adjusted(16*dpr,16*dpr,-16*dpr,-16*dpr).contains(bounds)){
                const int left=int(std::floor((bounds.left()/dpr-48)/64))*64;
                const int top=int(std::floor((bounds.top()/dpr-48)/64))*64;
                const int right=int(std::ceil((bounds.right()/dpr+48)/64))*64;
                const int bottom=int(std::ceil((bounds.bottom()/dpr+48)/64))*64;
                const QRectF expanded(QPointF(left,top)*dpr,QSizeF(right-left,bottom-top)*dpr);
                m_petCanvasBounds=m_petCanvasBounds.united(expanded);
            }
            const QPointF origin=m_petCanvasBounds.topLeft();
            const QPoint localShift(qRound((origin.x()-m_petCanvasOrigin.x())/dpr),qRound((origin.y()-m_petCanvasOrigin.y())/dpr));
            const QRect geometry((m_petStageOrigin+origin/dpr).toPoint(),
                QSize(std::max(1,qRound(m_petCanvasBounds.width()/dpr)),std::max(1,qRound(m_petCanvasBounds.height()/dpr))));
            const QPoint move=geometry.topLeft()-m_window->position();
            m_petCanvasOrigin=origin;
            if(m_pointerMode==5)m_petDragWindow+=move;
            m_petCurrentRegion.translate(-localShift);
            if(m_window->geometry()!=geometry)m_window->setGeometry(geometry);
        }
    }
    const auto shift=[this](auto& draw){
        for(auto& vertex:draw.vertices){vertex.pos.x-=float(m_petCanvasOrigin.x());vertex.pos.y-=float(m_petCanvasOrigin.y());}
    };
    for(auto& draw:frame.draws){shift(draw);for(auto& mask:draw.masks)shift(mask);}
    frame.size=QSize(std::max(1,int(std::ceil(m_window->width()*dpr))),std::max(1,int(std::ceil(m_window->height()*dpr))));
}
void ViewerController::applyDesktopPetMove(){
    if(!m_petMoveQueued)return;
    m_petMoveQueued=false;
    if(!m_petMode||!m_window)return;
    m_petStageOrigin=QPointF(m_petMoveTarget)-m_petCanvasOrigin/m_dpr;
    if(m_window->position()!=m_petMoveTarget)m_window->setPosition(m_petMoveTarget);
}
void ViewerController::showDesktopPetMenu(QPointF position){
    if(!m_petMode||!m_window||m_petMenu)return;
    auto* menu=new QMenu;
    menu->setObjectName("petContextMenu");menu->setWindowFlag(Qt::WindowStaysOnTopHint,true);
    m_petMenu=menu;
    auto font=QApplication::font();font.setPixelSize(std::clamp(qRound(m_state.value("baseFontPixels",16).toDouble()/m_dpr),12,24));menu->setFont(font);
    const bool dark=m_state.value("darkTheme",false).toBool();
    menu->setStyleSheet(QStringLiteral("QMenu { background: %1; color: %2; border: 1px solid #b485f5; padding: 6px; } QMenu::item { padding: 9px 20px; border-radius: 5px; } QMenu::item:selected { background: #b485f5; color: black; }")
        .arg(dark?QStringLiteral("#28232f"):QStringLiteral("#f5edff"),dark?QStringLiteral("white"):QStringLiteral("black")));
    auto* random=menu->addAction(tr("Random Motion"));random->setCheckable(true);random->setChecked(m_petRandom);
    connect(random,&QAction::toggled,this,[this](bool checked){dispatch("pet.random",checked);});
    auto* next=menu->addAction(tr("Next Motion"));next->setObjectName("petNext");
    connect(next,&QAction::triggered,this,[this]{QTimer::singleShot(0,this,[this]{if(m_petMode)dispatch("pet.next");});});
    auto* leave=menu->addAction(tr("Exit Desktop Pet"));leave->setObjectName("petExit");
    connect(leave,&QAction::triggered,this,[this]{QTimer::singleShot(0,this,[this]{if(m_petMode)dispatch("pet.exit");});});
    connect(menu,&QMenu::aboutToHide,this,[this,menu]{
        m_modalPanels.remove("petMenu");m_petMenu=nullptr;m_clock.restart();syncPluginSuspension();menu->deleteLater();
    });
    m_modalPanels.insert("petMenu");m_clock.restart();syncPluginSuspension();
    menu->popup(m_window->mapToGlobal(position.toPoint()));
}
void ViewerController::updateDesktopPetRegion(){
    if(!m_petMode||!m_window||!m_snapshot)return;
    QRegion current;
    if(live2dMode()){
        const auto bounds=m_live2d->state().value("renderBounds").toMap();
        current=paddedInputRegion(QRectF(bounds.value("x").toDouble(),bounds.value("y").toDouble(),
            bounds.value("width").toDouble(),bounds.value("height").toDouble()),m_dpr,m_viewport);
    }else current=sceneInputRegion(*m_snapshot,m_dpr);
    QRegion next=current.united(m_petCurrentRegion);
    m_petCurrentRegion=current;
    if(next.isEmpty())next=QRegion(QRect(m_window->width()/2,m_window->height()/2,1,1));
    if(next!=m_petMask){m_window->setMask(next);m_petMask=next;}
}
void ViewerController::updateDesktopPet(){
    if(!m_petMode||!m_window)return;
    if(live2dMode()&&m_viewport.width()>0&&m_viewport.height()>0){
        const QPoint global=QCursor::pos();
        const QPoint local=m_window->mapFromGlobal(global);
        const QPointF p=QPointF(local)*m_dpr;
        const double x=p.x()/m_viewport.width()*2-1,y=1-p.y()/m_viewport.height()*2;
        m_live2d->command("live2d.drag",QVariantMap{{"x",std::clamp(x,-1.0,1.0)},{"y",std::clamp(y,-1.0,1.0)}});
        QPointF physicalScreen=QPointF(global)*m_dpr;
#if defined(Q_OS_WIN)
        POINT nativeCursor{};if(GetCursorPos(&nativeCursor))physicalScreen=QPointF(nativeCursor.x,nativeCursor.y);
#endif
        m_live2d->command("pet.pointer",QVariantMap{{"x",x},{"y",y},
            {"screenX",physicalScreen.x()},{"screenY",physicalScreen.y()},{"nowMs",m_petClock.elapsed()},{"dragging",m_pointerMode!=0}});
    }
    const qint64 now=m_petClock.elapsed();
    if(m_petRandom){
        if(m_nextPetMotion==0)m_nextPetMotion=now+QRandomGenerator::global()->bounded(10000,25000);
        else if(now>=m_nextPetMotion){
            const int count=int(m_state.value("animations").toList().size());
            if(count>1){const int selected=QRandomGenerator::global()->bounded(count);if(live2dMode())m_live2d->command("animation.play",selected);else runtime()->PlayMotionByIndex(size_t(selected));}
            m_nextPetMotion=now+QRandomGenerator::global()->bounded(10000,25000);
        }
    }
}
}
