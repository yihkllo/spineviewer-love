#include "core/viewer_controller.h"
#include "spinelove/asset_path.h"
#include "core/qa_frame_probe.h"
#include "core/legacy_preferences.h"
#include "render/spine_scene.h"
#include "render/ui_pointer_observer.h"
#include "spinelove/plugin_registry.h"
#include "plugin_api/plugin_loader.h"
#include "render/legacy_image_provider.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QFontDatabase>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTranslator>
#include <QTimer>
#include <QScreen>
#include <QSurfaceFormat>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QSettings>
#include <QDir>
#include <algorithm>
#include <functional>
#include "core/window_resolution_presets.h"
#include <cstdio>
#include <QDebug>

static QVariantMap qaState(const slqt::ViewerController& controller,QQuickWindow* window=nullptr){
    auto state=controller.state();const auto* host=controller.plugins();
    if(const auto snapshot=controller.snapshot())state["verifiedAnimationTime"]=snapshot->animationTime;
    if(window)if(const auto* clock=window->findChild<QObject*>("viewerFrameClock"))state["frameDriverRunning"]=clock->property("running");
    if(window){
        QVariantList scenes;
        std::function<void(QQuickItem*)> visit=[&](QQuickItem* item){
            if(const auto key=item->property("qaStateKey").toString();!key.isEmpty())state[key]=item->property("qaState");
            if(auto* scene=qobject_cast<slqt::SpineScene*>(item)){
                const auto frame=scene->snapshot();const auto origin=item->mapToScene(QPointF{});
                scenes.append(QVariantMap{{"name",item->objectName()},{"visible",item->isVisible()},
                    {"x",origin.x()},{"y",origin.y()},{"width",item->width()},{"height",item->height()},
                    {"dpr",window->devicePixelRatio()},{"frameWidth",frame?frame->size.width():0},{"frameHeight",frame?frame->size.height():0},
                    {"bufferWidth",scene->fixedColorBufferWidth()},{"bufferHeight",scene->fixedColorBufferHeight()}});
            }
            for(auto* child:item->childItems())visit(child);
        };visit(window->contentItem());state["qaScenes"]=scenes;
    }
    state["plugins"]=host->property("state");
    if(const auto* module=host->property("module").value<QObject*>()){
        state["pluginModule"]=module->property("state");
        if(const auto* media=module->property("media").value<QObject*>())state["pluginMedia"]=media->property("state");
    }
    return state;
}

class LegacyTranslations final : public QTranslator {
public:
    void setLanguage(const QString& code) {
        m_strings.clear();QFile file(":/lang/"+code+".txt");
        if(!file.open(QIODevice::ReadOnly))return;
        const auto lines=QString::fromUtf8(file.readAll()).split('\n');
        for(auto line:lines){if(line.endsWith('\r'))line.chop(1);const auto equal=line.indexOf('=');if(equal>0)m_strings.insert(line.left(equal),line.mid(equal+1));}
    }
    QString translate(const char*,const char* source,const char*,int) const override {return m_strings.value(QString::fromUtf8(source));}
    bool isEmpty() const override {return false;}
private:QHash<QString,QString> m_strings;
};

int main(int argc,char** argv){
    auto format=QSurfaceFormat::defaultFormat();format.setAlphaBufferSize(8);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);
    app.setApplicationName("SpineLoveEX");app.setOrganizationName("SpineLoveEX");
    app.setWindowIcon(QIcon(QStringLiteral(":/main/resources/app.ico")));
    const auto args=app.arguments();
    slqt::loadPlugins(QCoreApplication::applicationDirPath()+QStringLiteral("/pro"));
    if(const auto code=slqt::PluginRegistry::runCommand(args))return *code;
    app.setProperty("qaSilent",args.contains("--qa-silent"));
    const int isolatedSettings=args.indexOf("--settings-dir");
    if(isolatedSettings>=0&&isolatedSettings+1<args.size()){
        QDir().mkpath(args[isolatedSettings+1]);
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,args[isolatedSettings+1]);
    }
    QSettings preferences(QSettings::defaultFormat(),QSettings::UserScope,"SpineLoveEX","QtMigration");
    const int legacyArg=args.indexOf("--legacy-data-dir");
    if(legacyArg>=0&&legacyArg+1<args.size()){
        const QDir source(args[legacyArg+1]);
        QSettings legacy(source.filePath("spine_window_resolution.ini"),QSettings::IniFormat);
        const int preset=legacy.value("Window/ResolutionPreset",0).toInt();
        if(window_resolution_presets::Get(preset))preferences.setValue("resolutionPreset",preset);
        const auto language=slqt::readLegacyText(source.filePath("spine_language.txt")).trimmed();
        if(QStringList{"en","zh_CN"}.contains(language))preferences.setValue("language",language);
        const auto favorites=source.filePath("spine_favorites.txt");
        if(QFileInfo::exists(favorites))preferences.setValue("favorites",slqt::readLegacyFavorites(favorites));
    }
    const float desktopScale=app.primaryScreen()?float(app.primaryScreen()->geometry().width()*app.primaryScreen()->devicePixelRatio()/1920.0):1.f;
    preferences.setValue("uiScale",window_resolution_presets::UiScale(preferences.value("resolutionPreset",0).toInt(),desktopScale));
    if(args.contains("--opengl"))QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    const int fontId=QFontDatabase::addApplicationFont(slqt::packagedAssetPath("NotoSansSC-Regular.ttf"));
    if(fontId>=0){const auto names=QFontDatabase::applicationFontFamilies(fontId);if(!names.isEmpty())app.setFont(QFont(names.front()));}
    qmlRegisterType<slqt::SpineScene>("SpineLove",1,0,"SpineScene");
    qmlRegisterType<slqt::UiPointerObserver>("SpineLove",1,0,"UiPointerObserver");
    slqt::PluginRegistry::runStartup();
    qmlRegisterUncreatableType<slqt::ViewerController>("SpineLove",1,0,"ViewerController","Created by the application");
    slqt::ViewerController controller;
    LegacyTranslations translator;translator.setLanguage(controller.state().value("language").toString());
    const bool translationInstalled=app.installTranslator(&translator);
    qInfo().noquote()<<"Language startup:"<<controller.state().value("language").toString()<<"installed:"<<translationInstalled
        <<"direct:"<<translator.translate("SettingsDialog","Setting",nullptr,-1)<<"application:"<<QCoreApplication::translate("SettingsDialog","Setting");
    QQmlApplicationEngine engine;engine.addImageProvider("legacy-texture",new slqt::LegacyImageProvider);
    engine.setUiLanguage(controller.state().value("language").toString());
    engine.rootContext()->setContextProperty("backend",&controller);
    QObject::connect(&controller,&slqt::ViewerController::languageRequested,&engine,[&](const QString& language){
        app.removeTranslator(&translator);
        translator.setLanguage(language);
        app.installTranslator(&translator);
        engine.setUiLanguage(language);
        engine.retranslate();
    });
    QObject::connect(&engine,&QQmlApplicationEngine::objectCreationFailed,&app,[]{QCoreApplication::exit(2);},Qt::QueuedConnection);
    engine.load(QUrl("qrc:/Main.qml"));if(engine.rootObjects().isEmpty())return 2;
    engine.retranslate();
    qInfo().noquote()<<"Language engine:"<<engine.uiLanguage()<<"application:"<<QCoreApplication::translate("SettingsDialog","Setting");
    auto* window=qobject_cast<QQuickWindow*>(engine.rootObjects().front());
    if(window)QTimer::singleShot(250,&engine,[window]{
        std::function<void(QQuickItem*)> findSetting=[&](QQuickItem* item){
            if(item->objectName()=="entry_settings")qInfo().noquote()<<"Language QML setting label:"<<item->property("text").toString();
            for(auto* child:item->childItems())findSetting(child);
        };
        findSetting(window->contentItem());
    });
    if(window&&window->screen()&&!args.contains("--fixed-size")){
        const auto desktop=window->screen()->geometry();const qreal dpr=window->devicePixelRatio();
        window->resize(qMax(qRound(640/dpr),desktop.width()*9/10),qMax(qRound(480/dpr),desktop.height()*9/10));
        window->setPosition(desktop.x()+(desktop.width()-window->width())/2,desktop.y()+(desktop.height()-window->height())/4);
    }
    controller.setWindow(window);
    const int frameReport=args.indexOf("--frame-report");
    const int frameDuration=args.indexOf("--frame-report-duration");
    if(window&&frameReport>=0&&frameReport+1<args.size())slqt::installFrameProbe(app,window,controller,args[frameReport+1],frameDuration>=0?std::clamp(args.value(frameDuration+1).toInt(),2000,120000):6500);
    if(window&&window->screen()&&!args.contains("--fixed-size")){
        const int presetIndex=preferences.value("resolutionPreset",0).toInt();
        const auto* preset=window_resolution_presets::Get(presetIndex);
        if(preset&&preset->width>0)controller.dispatch("settings.resolution",presetIndex);
        else if(presetIndex==-1)controller.dispatch("settings.resolution.custom",QVariantMap{
            {"width",preferences.value("customWindowWidth",1280)},
            {"height",preferences.value("customWindowHeight",720)}});
    }
    const int viewport=args.indexOf("--viewport");
    if(window&&viewport>=0&&viewport+1<args.size()){
        const auto parts=args[viewport+1].split('x');if(parts.size()==2){const int w=parts[0].toInt(),h=parts[1].toInt();if(w>0&&h>0)window->resize(qRound(w/window->devicePixelRatio()),qRound(h/window->devicePixelRatio()));}
    }
    if(window)window->show();
    const int open=args.indexOf("--open");if(open>=0&&open+1<args.size())QTimer::singleShot(0,&controller,[&controller,args,open]{controller.openPaths({args[open+1]});});
    const int script=args.indexOf("--commands");
    int captureAfterMs=1800;
    auto qaLastAction=std::make_shared<QElapsedTimer>();qaLastAction->start();
    if(script>=0&&script+1<args.size()){
        QFile file(args[script+1]);
        QMap<int,QJsonArray> schedule;
        if(file.open(QIODevice::ReadOnly))for(const auto& command:QJsonDocument::fromJson(file.readAll()).array()){
            const auto object=command.toObject();const int atMs=std::clamp(object.value("atMs").toInt(500),0,110000);
            captureAfterMs=std::max(captureAfterMs,atMs+500);
            schedule[atMs].append(object);
        }
        auto pending=std::make_shared<QMap<int,QJsonArray>>(std::move(schedule));
        auto scriptClock=std::make_shared<QElapsedTimer>();scriptClock->start();
        auto* scriptTimer=new QTimer(&controller);scriptTimer->setTimerType(Qt::PreciseTimer);scriptTimer->setInterval(5);
        QObject::connect(scriptTimer,&QTimer::timeout,&controller,[&controller,window,pending,scriptClock,scriptTimer,qaLastAction]{
            while(!pending->isEmpty()&&pending->firstKey()<=scriptClock->elapsed()){
                const auto commands=pending->take(pending->firstKey());
                for(const auto& entry:commands){
                    const auto object=entry.toObject();const auto command=object.value("command").toString();
                    if(command=="qa.window.restore"){if(window)window->showNormal();qaLastAction->restart();continue;}
                    if(command=="qa.state.dump"){
                        auto state=qaState(controller,window);if(window){state["windowVisibility"]=int(window->visibility());state["windowGeometry"]=QVariantMap{{"x",window->x()},{"y",window->y()},{"width",window->width()},{"height",window->height()}};}
                        QFile output(object.value("value").toObject().value("path").toString());
                        if(output.open(QIODevice::WriteOnly))output.write(QJsonDocument(QJsonObject::fromVariantMap(state)).toJson());
                        continue;
                    }
                    controller.dispatch(command,object.value("value").toVariant());
                    qaLastAction->restart();
                }
            }
            if(pending->isEmpty())scriptTimer->stop();
        });scriptTimer->start();
    }
    const int capture=args.indexOf("--screenshot");
    if(capture>=0&&capture+1<args.size()){
        auto* captureTimer=new QTimer(&app);captureTimer->setInterval(100);
        auto elapsed=std::make_shared<QElapsedTimer>();elapsed->start();
        QObject::connect(captureTimer,&QTimer::timeout,&app,[&,captureTimer,elapsed]{
            if(elapsed->elapsed()<captureAfterMs||(script>=0&&qaLastAction->elapsed()<1000))return;
            if(window&&elapsed->elapsed()<120000){
                bool hold=false;
                std::function<void(QQuickItem*)> visit=[&](QQuickItem* item){hold|=item->property("qaCaptureHold").toBool();for(auto* child:item->childItems())visit(child);};
                visit(window->contentItem());
                if(hold){window->requestUpdate();return;}
            }
            if(controller.state().value("exportRunning").toBool()&&elapsed->elapsed()<120000)return;
            captureTimer->stop();
            bool saved=window&&window->grabWindow().save(args[capture+1]);
            auto stateMap=qaState(controller,window);
            stateMap["settingsFile"]=preferences.fileName();
            if(window){stateMap["windowGeometry"]=QVariantMap{{"x",window->x()},{"y",window->y()},{"width",window->width()},{"height",window->height()}};stateMap["windowVisibility"]=int(window->visibility());stateMap["windowFlags"]=int(window->flags());}
            if(window)stateMap["windowColor"]=window->color().name(QColor::HexArgb);
            if(window&&window->screen()){
                const auto available=window->screen()->availableGeometry();
                stateMap["screenAvailableGeometry"]=QVariantMap{{"x",available.x()},{"y",available.y()},{"width",available.width()},{"height",available.height()}};
            }
            if(window)for(const auto* property:{"_slNativeResizeEffective","_slNativeTopmostEffective","_slNativeColorKeyEffective"})
                stateMap[QString::fromLatin1(property)]=window->property(property);
            if(auto* desktop=window?window->findChild<QObject*>("desktopViewer"):nullptr){
                stateMap["verifiedCanvasLeft"]=desktop->property("canvasLeft");stateMap["verifiedTitleHeight"]=desktop->property("titleHeight");
            }
            QFile state(args[capture+1]+".json");if(state.open(QIODevice::WriteOnly))state.write(QJsonDocument(QJsonObject::fromVariantMap(stateMap)).toJson());
            app.exit(!saved?3:controller.state().value("exportFailed").toBool()?4:0);
        });
        captureTimer->start();
    }
    return app.exec();
}
