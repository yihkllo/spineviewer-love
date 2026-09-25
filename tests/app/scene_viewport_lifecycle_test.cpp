#include "render/spine_scene.h"
#include <QQuickWindow>
#include <QtTest>
#include <cmath>

class ProbeSource final:public slqt::SceneSource {
public:
    QSize pixels;int updates=0;
    void setViewport(QSizeF size,qreal dpr)override{if(size.isEmpty())return;pixels={int(std::ceil(size.width()*dpr)),int(std::ceil(size.height()*dpr))};++updates;}
    std::shared_ptr<const slqt::SceneSnapshot> snapshot()const override{return {};}
};
class ViewportLifecycleTest final:public QObject {
    Q_OBJECT
private slots:
    void lazy_scene_uses_window_dpr_on_attach(){
        ProbeSource source;slqt::SpineScene scene;scene.setSize({800,450});scene.setFrameSource(&source);
        QCOMPARE(source.updates,0);
        QQuickWindow window;QVERIFY(window.devicePixelRatio()>1);scene.setParentItem(window.contentItem());
        QCOMPARE(source.pixels,QSize(qCeil(800*window.devicePixelRatio()),qCeil(450*window.devicePixelRatio())));
        QCOMPARE(scene.fixedColorBufferWidth(),source.pixels.width());QCOMPARE(scene.fixedColorBufferHeight(),source.pixels.height());
        const auto last=source.pixels;scene.setParentItem(nullptr);QCOMPARE(source.pixels,last);
    }
    void replacing_a_loader_does_not_restore_detached_scene_geometry(){
        ProbeSource source;QQuickWindow window;slqt::SpineScene oldScene,newScene;
        oldScene.setSize({800,450});oldScene.setFrameSource(&source);oldScene.setParentItem(window.contentItem());
        newScene.setSize({800,600});newScene.setFrameSource(&source);newScene.setParentItem(window.contentItem());
        const QSize expected(qCeil(800*window.devicePixelRatio()),qCeil(600*window.devicePixelRatio()));
        QCOMPARE(source.pixels,expected);oldScene.setParentItem(nullptr);QCOMPARE(source.pixels,expected);
        newScene.setSize({600,900});QCOMPARE(source.pixels,QSize(qCeil(600*window.devicePixelRatio()),qCeil(900*window.devicePixelRatio())));
        newScene.setParentItem(nullptr);
    }
};
QTEST_MAIN(ViewportLifecycleTest)
#include "scene_viewport_lifecycle_test.moc"
