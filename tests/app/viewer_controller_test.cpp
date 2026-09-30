#include "core/viewer_controller.h"
#include "viewer_load_benchmark.h"
#include "core/window_geometry.h"
#include "core/legacy_preferences.h"
#include "runtime_fixture.h"
#include <QQmlPropertyMap>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <algorithm>
#include <QtTest>

using slqt::ViewerController;
namespace {
void write(const QString& path,const QByteArray& bytes){QFile f(path);if(!f.open(QIODevice::WriteOnly)||f.write(bytes)!=bytes.size())qFatal("Cannot create fixture");}
QString model(const QString& folder,const QString& name){
    QDir().mkpath(folder);const auto fixture=sl_test::Fixture("3.6");
    write(folder+"/"+name+".atlas",QByteArray::fromStdString(fixture.atlasData.front()));
    write(folder+"/"+name+".json",QByteArray::fromStdString(fixture.skeletonData.front()));
    QImage texture(32,32,QImage::Format_RGBA8888);texture.fill(Qt::white);texture.save(folder+"/probe.png");
    return folder+"/"+name+".json";
}
void key(QQuickWindow& w,QEvent::Type type,int code,bool repeat=false){QKeyEvent e(type,code,Qt::NoModifier,{},repeat);QCoreApplication::sendEvent(&w,&e);}
QString firstLayer(const ViewerController& c){return c.state().value("loadedSpines").toList().value(0).toMap().value("name").toString();}
void setup(QQuickWindow& w,ViewerController& c){w.resize(800,600);c.setWindow(&w);c.setViewport({800,600},1);}
}

class ControllerTest : public QObject {
    Q_OBJECT
private slots:
    void renderSizeIsIndependentOfWindowAndPreview(){
        QQuickWindow w;ViewerController c;setup(w,c);
        w.create();
        const auto geometry=w.geometry();const auto preset=c.state().value("resolutionPreset");
        c.dispatch("settings.renderSize",QVariantMap{{"width",3840},{"height",2160}});
        QCOMPARE(w.geometry(),geometry);QCOMPARE(c.snapshot()->size,QSize(3840,2160));
        QCOMPARE(c.state().value("resolutionPreset"),preset);
        c.setViewport({480,270},2);
        QCOMPARE(c.snapshot()->size,QSize(3840,2160));
        c.dispatch("settings.renderSize",QVariantMap{{"width",3840},{"height",4320}});
        QCOMPARE(c.snapshot()->size,QSize(3840,4320));QCOMPARE(w.geometry(),geometry);
        c.setViewport({240,270},2);
        QCOMPARE(c.snapshot()->size,QSize(3840,4320));
        c.dispatch("settings.renderSize",QVariantMap{{"width",16384},{"height",16384}});
        QCOMPARE(c.snapshot()->size,QSize(3840,4320));
        c.dispatch("settings.renderSize",QVariantMap{{"width",16385},{"height",64}});
        QCOMPARE(c.snapshot()->size,QSize(3840,4320));
        c.dispatch("settings.renderSize.reset");
        QVERIFY(c.renderSize().isEmpty());QCOMPARE(c.snapshot()->size,QSize(480,540));
        QCOMPARE(w.geometry(),geometry);
    }
    void backgroundDraggingUsesRenderPixelsInScaledPreview(){
        QTemporaryDir d;QQuickWindow w;ViewerController c;setup(w,c);
        const QString path=d.path()+"/background.png";
        QImage picture(32,32,QImage::Format_RGBA8888);picture.fill(Qt::cyan);QVERIFY(picture.save(path));
        c.openPaths({model(d.path(),"stage")});
        c.dispatch("background.open",path);
        c.dispatch("settings.renderSize",QVariantMap{{"width",3840},{"height",2160}});
        c.setViewport({960,540},1);
        const auto before=c.snapshot()->draws.front().vertices.front().pos;
        c.pointerPress({100,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerMove({100,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerMove({125,150},Qt::LeftButton,Qt::NoModifier);
        c.pointerRelease({125,150},Qt::LeftButton,Qt::NoModifier);
        QCOMPARE(c.snapshot()->draws.front().vertices.front().pos.x,before.x);
        c.pointerPress({100,100},Qt::LeftButton,Qt::ControlModifier);
        c.pointerMove({100,100},Qt::LeftButton,Qt::ControlModifier);
        c.pointerMove({125,150},Qt::LeftButton,Qt::ControlModifier);
        c.pointerRelease({125,150},Qt::LeftButton,Qt::ControlModifier);
        const auto after=c.snapshot()->draws.front().vertices.front().pos;
        QCOMPARE(after.x-before.x,100.f);QCOMPARE(after.y-before.y,200.f);
        c.dispatch("background.open",path);
        const auto lower=c.snapshot()->draws[0].vertices.front().pos,upper=c.snapshot()->draws[1].vertices.front().pos;
        c.pointerPress({100,100},Qt::LeftButton,Qt::ControlModifier);
        c.pointerMove({100,100},Qt::LeftButton,Qt::ControlModifier);
        c.pointerMove({125,150},Qt::LeftButton,Qt::ControlModifier);
        c.pointerRelease({125,150},Qt::LeftButton,Qt::ControlModifier);
        QCOMPARE(c.snapshot()->draws[0].vertices.front().pos.x,lower.x);
        QCOMPARE(c.snapshot()->draws[1].vertices.front().pos.x,upper.x);
        c.dispatch("background.select",0);
        c.pointerPress({100,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerMove({100,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerMove({125,150},Qt::LeftButton,Qt::NoModifier);
        QCOMPARE(c.snapshot()->draws[1].vertices.front().pos.x-upper.x,100.f);
        QCOMPARE(c.snapshot()->draws[0].vertices.front().pos.x,lower.x);
    }
    void frameClockRespectsPlaybackSuspension(){
        QTemporaryDir d;QQuickWindow w;ViewerController c;setup(w,c);w.show();
        c.openPaths({model(d.path(),"frame-clock")});c.resetFrameClock();
        auto time=[&]{return c.snapshot()->animationTime;};
        QTest::qWait(20);c.advanceFrame();const double running=time();QVERIFY(running>0);
        c.dispatch("settings.modal",true);QTest::qWait(20);c.advanceFrame();QCOMPARE(time(),running);
        c.dispatch("settings.modal",false);QTest::qWait(20);c.advanceFrame();QVERIFY(time()>running);
        const double beforeMinimize=time();w.showMinimized();QTest::qWait(20);c.advanceFrame();QCOMPARE(time(),beforeMinimize);
        w.showNormal();c.resetFrameClock();QTest::qWait(20);c.advanceFrame();QVERIFY(time()>beforeMinimize);QVERIFY(time()-beforeMinimize<.08);
        const double beforePlugins=time();c.dispatch("plugins");QTest::qWait(20);c.advanceFrame();QCOMPARE(time(),beforePlugins);
        c.dispatch("plugins.close");QTest::qWait(20);c.advanceFrame();QVERIFY(time()>beforePlugins);
    }
    void legacyPortraitIncludesBorderlessClientAllowance(){
        const QSize work(3840,2080),frame(24,24);
        const auto normal=slqt::legacyClientSize({2880,1620},frame,work,true);
        QCOMPARE(normal,QSize(2904,1644));
        const auto portrait=slqt::legacyClientSize({qRound(normal.height()/2.),normal.height()},frame,work,true);
        QCOMPARE(portrait,QSize(846,1668));
        QCOMPARE(slqt::legacyClientSize({810,1620},{0,0},work,true),QSize(810,1620));
        const auto fitted=slqt::legacyClientSize({2000,4000},frame,{1920,1040},true);
        QCOMPARE(fitted,QSize(532,1040));
        QCOMPARE(slqt::legacyClientSize({800,1600},{24,70},work,false),QSize(800,1600));
    }
    void identicalViewportDoesNotRepublishState(){
        QQuickWindow w;ViewerController c;setup(w,c);
        QSignalSpy changes(&c,&ViewerController::stateChanged);
        c.setViewport({800,600},1);QCOMPARE(changes.count(),0);
        c.setViewport({400,300},2);QCOMPARE(changes.count(),0);
        QCOMPARE(c.state().value("devicePixelRatio").toDouble(),2.0);
        QTRY_COMPARE(changes.count(),1);
        changes.clear();c.setViewport({400,300},2);QCOMPARE(changes.count(),0);
    }
    void proSelectorBlocksBaseInputAndPreservesLoadedModel(){
        QTemporaryDir d;const auto a=model(d.path(),"before"),b=model(d.path(),"after");
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({a});c.dispatch("view.scale",2.0);
        c.dispatch("plugins");QVERIFY(c.state().value("pluginsOpen").toBool());QVERIFY(!c.state().value("pluginActive").toBool());
        QVERIFY(!c.state().value("capabilities").toMap().value("file.open").toBool());
        c.openPaths({b});c.openUrls({QUrl::fromLocalFile(b)});c.dispatch("view.scale",3.0);c.dispatch("mode.toggle");
        QCOMPARE(firstLayer(c),QString("before"));QCOMPARE(c.state().value("scale").toDouble(),2.0);QCOMPARE(c.state().value("mode").toString(),QString("spine"));
        c.dispatch("plugins.close");QVERIFY(!c.state().value("pluginsOpen").toBool());
        QVERIFY(c.state().value("capabilities").toMap().value("file.open").toBool());QCOMPARE(firstLayer(c),QString("before"));
    }
    void controllerUsesTheRequestedSettingsFormat(){
        QSettings preferences(QSettings::IniFormat,QSettings::UserScope,"SpineLoveEX","QtMigration");
        preferences.setValue("language","en");preferences.sync();
        {ViewerController c;QCOMPARE(c.state().value("language").toString(),QString("en"));}
        preferences.setValue("language","ja_JP");preferences.sync();
        {ViewerController c;QCOMPARE(c.state().value("language").toString(),QString("zh_CN"));}
        preferences.remove("language");preferences.sync();
        ViewerController c;QCOMPARE(c.state().value("language").toString(),QString("zh_CN"));
    }
    void onlyChineseAndEnglishAreOffered(){
        ViewerController c;
        c.dispatch("settings.language","ja_JP");QCOMPARE(c.state().value("language").toString(),QString("zh_CN"));
        c.dispatch("settings.language","en");QCOMPARE(c.state().value("language").toString(),QString("en"));
        const auto languages=qobject_cast<QQmlPropertyMap*>(c.lists())->value("languages").toList();
        QCOMPARE(languages.size(),2);
        QCOMPARE(languages[0].toMap().value("id").toString(),QString("zh_CN"));
        QCOMPARE(languages[1].toMap().value("id").toString(),QString("en"));
        c.dispatch("settings.language","zh_CN");
    }
    void legacyFavoritesDecodeBomDropDuplicatesAndKeepMissing(){
        QTemporaryDir d;const auto a=model(d.path(),QString::fromUtf8("中文模型"));
        const auto rows=a+"\r\n"+a+"\r\n"+d.path()+"/missing.json\r\n";
        const auto cache=d.path()+"/favorites.txt";
        QStringList expected{a,QDir::cleanPath(d.path()+"/missing.json")};std::sort(expected.begin(),expected.end());
        write(cache,QByteArray::fromHex("efbbbf")+rows.toUtf8());QCOMPARE(slqt::readLegacyFavorites(cache),expected);
        QByteArray utf16=QByteArray::fromHex("fffe");for(QChar ch:rows){utf16.append(char(ch.unicode()&255));utf16.append(char(ch.unicode()>>8));}
        write(cache,utf16);QCOMPARE(slqt::readLegacyFavorites(cache),expected);
        write(cache,QByteArray::fromHex("efbbbf")+"zh_CN\r\n");QCOMPARE(slqt::readLegacyText(cache).trimmed(),QString("zh_CN"));
    }
    void favoriteFoldersGroupMoveCopyAndPersist(){
        QTemporaryDir d;const auto a=model(d.path(),"first"),b=model(d.path(),"second");
        const auto names=[](ViewerController& c){QStringList out;for(const auto& row:c.state().value("files").toList())out.append(row.toMap().value("name").toString());return out;};
        const auto folders=[](ViewerController& c){return c.state().value("favoriteFolders").toList();};
        QString custom;
        {
            QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({d.path()});
            QCOMPARE(folders(c).size(),1);QVERIFY(folders(c)[0].toMap().value("isDefault").toBool());
            c.dispatch("file.favorite",a);
            c.dispatch("favorites.folderCreate",QString::fromUtf8("角色"));
            QCOMPARE(folders(c).size(),2);custom=folders(c)[1].toMap().value("id").toString();
            QCOMPARE(c.state().value("favoriteFolder").toString(),custom);QVERIFY(c.state().value("favoritesOnly").toBool());QVERIFY(names(c).isEmpty());
            c.dispatch("favorites.copy",QVariantMap{{"path",a},{"to",custom}});
            c.dispatch("favorites.copy",QVariantMap{{"path",b},{"to",custom}});
            QCOMPARE(names(c),(QStringList{"first","second"}));
            QCOMPARE(folders(c)[1].toMap().value("count").toInt(),2);
            c.dispatch("file.favorite",a);QCOMPARE(names(c),QStringList{"second"});
            c.dispatch("favorites.folderSelect","default");QCOMPARE(names(c),QStringList{"first"});
            c.dispatch("favorites.move",QVariantMap{{"path",a},{"from","default"},{"to",custom}});QVERIFY(names(c).isEmpty());
            c.dispatch("favorites.folderRename",QVariantMap{{"id",custom},{"name","group"}});
            c.dispatch("favorites.folderRename",QVariantMap{{"id","default"},{"name","nope"}});
        }
        {
            QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({d.path()});
            QCOMPARE(folders(c).size(),2);QCOMPARE(folders(c)[1].toMap().value("name").toString(),QString("group"));
            QVERIFY(folders(c)[0].toMap().value("name").toString().isEmpty());
            c.dispatch("favorites.folderSelect",custom);QCOMPARE(names(c),(QStringList{"first","second"}));
            const auto row=c.state().value("files").toList()[0].toMap();QCOMPARE(row.value("folders").toStringList(),QStringList{custom});QVERIFY(row.value("favorite").toBool());
            c.dispatch("favorites.remove",QVariantMap{{"path",b},{"folder",custom}});QCOMPARE(names(c),QStringList{"first"});
            c.dispatch("favorites.folderDelete",custom);
            QCOMPARE(folders(c).size(),1);QCOMPARE(c.state().value("favoriteFolder").toString(),QString("default"));QCOMPARE(names(c),QStringList{"first"});
            c.dispatch("favorites.folderCreate",QVariantMap{{"name","moved"},{"path",a},{"action","move"},{"from","default"}});
            QCOMPARE(c.state().value("favoriteFolder").toString(),QString("default"));QVERIFY(names(c).isEmpty());
            QCOMPARE(folders(c)[1].toMap().value("count").toInt(),1);
            c.dispatch("file.favoritesView",false);c.dispatch("file.favorite",a);
            QCOMPARE(folders(c)[1].toMap().value("count").toInt(),0);
            c.dispatch("favorites.folderDelete",folders(c)[1].toMap().value("id"));
        }
    }
    void missingFavoritesSurviveUntilTheirDriveReturns(){
        QTemporaryDir d;const auto drive=d.path()+"/drive",away=d.path()+"/away";
        const auto a=model(drive,"gone"),b=model(d.path(),"stay");
        const auto rows=[](ViewerController& c){QVariantMap out;for(const auto& row:c.state().value("files").toList())out.insert(row.toMap().value("name").toString(),row.toMap().value("missing"));return out;};
        {
            QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({d.path()});
            c.dispatch("favorites.copy",QVariantMap{{"paths",QStringList{a,b}},{"to","default"}});
        }
        QVERIFY(QDir().rename(drive,away));
        {
            QQuickWindow w;ViewerController c;setup(w,c);
            c.dispatch("settings.language","en");c.dispatch("settings.language","zh_CN");
            c.dispatch("file.favoritesView",true);
            const auto shown=rows(c);
            QCOMPARE(shown.size(),2);QVERIFY(shown.value("gone").toBool());QVERIFY(!shown.value("stay").toBool());
            QCOMPARE(c.state().value("favoriteFolders").toList()[0].toMap().value("count").toInt(),2);
            c.dispatch("file.play",a);
            QVERIFY(!c.state().value("lastError").toString().isEmpty());QVERIFY(c.state().value("loadedSpines").toList().isEmpty());
        }
        QVERIFY(QDir().rename(away,drive));
        {
            QQuickWindow w;ViewerController c;setup(w,c);
            c.dispatch("file.favoritesView",true);
            const auto shown=rows(c);
            QCOMPARE(shown.size(),2);QVERIFY(!shown.value("gone").toBool());QVERIFY(!shown.value("stay").toBool());
            c.dispatch("favorites.unfavorite",QVariantMap{{"paths",QStringList{a,b}}});
            QVERIFY(rows(c).isEmpty());
        }
    }
    void favoriteFoldersTakeSeveralModelsAtOnce(){
        QTemporaryDir d;const auto a=model(d.path(),"first"),b=model(d.path(),"second"),e=model(d.path(),"third");
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({d.path()});
        const auto names=[&]{QStringList out;for(const auto& row:c.state().value("files").toList())out.append(row.toMap().value("name").toString());return out;};
        c.dispatch("favorites.copy",QVariantMap{{"paths",QStringList{a,b,e}},{"to","default"}});
        c.dispatch("file.favoritesView",true);QCOMPARE(names(),(QStringList{"first","second","third"}));
        c.dispatch("favorites.folderCreate",QVariantMap{{"name","group"},{"paths",QStringList{a,b}},{"action","move"},{"from","default"}});
        QCOMPARE(names(),QStringList{"third"});
        const auto group=c.state().value("favoriteFolders").toList()[1].toMap().value("id").toString();
        c.dispatch("favorites.move",QVariantMap{{"paths",QStringList{a,b}},{"from",group},{"to","default"}});
        QCOMPARE(names(),(QStringList{"first","second","third"}));
        c.dispatch("favorites.remove",QVariantMap{{"paths",QStringList{a,e}},{"folder","default"}});QCOMPARE(names(),QStringList{"second"});
        c.dispatch("favorites.copy",QVariantMap{{"paths",QStringList{a,b}},{"to",group}});
        c.dispatch("favorites.unfavorite",QVariantMap{{"paths",QStringList{a,b}}});
        QVERIFY(names().isEmpty());
        for(const auto& row:c.state().value("favoriteFolders").toList())QCOMPARE(row.toMap().value("count").toInt(),0);
        c.dispatch("favorites.folderDelete",group);
    }
    void repeatedArrowPreviewsUntilRelease(){
        QTemporaryDir d;const auto a=model(d.path(),"model2");model(d.path(),"model10");model(d.path(),"model20");
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({d.path()});c.openPaths({a});
        key(w,QEvent::KeyPress,Qt::Key_Down);
        QCOMPARE(c.state().value("currentFileName").toString(),QString("model10"));QCOMPARE(firstLayer(c),QString("model2"));
        key(w,QEvent::KeyRelease,Qt::Key_Down,true);QCOMPARE(firstLayer(c),QString("model2"));
        key(w,QEvent::KeyPress,Qt::Key_Down,true);QCOMPARE(c.state().value("currentFileName").toString(),QString("model20"));
        key(w,QEvent::KeyRelease,Qt::Key_Down);QCOMPARE(firstLayer(c),QString("model20"));
    }
    void releaseWithoutPreviewDoesNotReload(){
        QTemporaryDir d;const auto a=model(d.path(),"model");QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({a});
        c.dispatch("spine.mirror");const auto before=c.snapshot();
        key(w,QEvent::KeyRelease,Qt::Key_Down);
        QVERIFY2(c.snapshot()==before,"A key release without a viewer preview reloaded the model and reset its transforms");
    }
    void editorOwnsArrowKeys(){
        QTemporaryDir d;const auto a=model(d.path(),"model2");model(d.path(),"model10");
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({d.path()});c.openPaths({a});
        QQuickItem editor(w.contentItem());editor.setFlag(QQuickItem::ItemAcceptsInputMethod);editor.forceActiveFocus();
        key(w,QEvent::KeyPress,Qt::Key_Down);key(w,QEvent::KeyRelease,Qt::Key_Down);
        QCOMPARE(firstLayer(c),QString("model2"));QCOMPARE(c.state().value("currentFileName").toString(),QString("model2"));
    }
    void nestedModalKeepsStageBlocked(){
        QQuickWindow w;ViewerController c;setup(w,c);
        c.dispatch("settings.modal",QVariantMap{{"source","settings"},{"open",true}});
        c.dispatch("settings.modal",QVariantMap{{"source","error"},{"open",true}});
        c.dispatch("settings.modal",QVariantMap{{"source","error"},{"open",false}});
        QTemporaryDir d;const auto a=model(d.path(),"model");c.openUrls({QUrl::fromLocalFile(a)});
        QVERIFY(!c.state().value("loaded").toBool());
        c.dispatch("settings.modal",QVariantMap{{"source","settings"},{"open",false}});
        c.openUrls({QUrl::fromLocalFile(a)});QVERIFY(c.state().value("loaded").toBool());
    }
    void emptyDirectoryAndModeRoundTrip(){
        QTemporaryDir d;const auto a=model(d.path()+"/models","model");QDir().mkpath(d.path()+"/empty");
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({QFileInfo(a).absolutePath()});c.openPaths({a});
        c.openPaths({d.path()+"/empty"});QVERIFY(c.state().value("files").toList().isEmpty());
        QVERIFY(c.state().value("lastError").toString().isEmpty());
        c.dispatch("mode.toggle");c.dispatch("mode.toggle");
        QVERIFY(c.state().value("files").toList().isEmpty());QCOMPARE(firstLayer(c),QString("model"));
    }
    void directoryFallbackRestoresCurrentModeIdentity(){
        QTemporaryDir d;const auto a=model(d.path()+"/models","model");QDir().mkpath(d.path()+"/live");
        write(d.path()+"/live/first.model3.json","{}");write(d.path()+"/live/second.model3.json","{}");
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({a});
        c.openPaths({d.path()+"/live"});QCOMPARE(c.state().value("mode").toString(),QString("live2d"));
        QCOMPARE(c.state().value("files").toList().size(),2);
        c.openPaths({d.path()+"/models"});QCOMPARE(c.state().value("mode").toString(),QString("spine"));
        QCOMPARE(c.state().value("currentFileName").toString(),QString("model"));
        c.dispatch("mode.toggle");QCOMPARE(c.state().value("files").toList().size(),2);
    }
    void replacementDialogRejectsDroppedFiles(){
        QTemporaryDir d;const auto a=model(d.path(),"first"),b=model(d.path(),"second"),other=model(d.path(),"third");
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({a});c.dispatch("file.addSpine",b);c.openPaths({other});
        QVERIFY(c.state().value("replaceConfirmation").toMap().value("open").toBool());
        c.openUrls({QUrl::fromLocalFile(a)});c.dispatch("replace.confirm");
        QCOMPARE(firstLayer(c),QString("third"));
    }
    void incompleteJsonPreservesWorkingModel(){
        QTemporaryDir d;const auto a=model(d.path(),"valid"),b=model(d.path(),"broken");
        auto invalid=QJsonDocument::fromJson(QByteArray::fromStdString(sl_test::Fixture("3.6").skeletonData.front())).object();
        invalid.remove("bones");write(b,QJsonDocument(invalid).toJson());
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({a});c.openPaths({b});
        QVERIFY(!c.state().value("lastError").toString().isEmpty());
        QVERIFY(c.state().value("loaded").toBool());QCOMPARE(firstLayer(c),QString("valid"));
    }
    void layerPickersFollowTheSelectedModel(){
        QTemporaryDir d;const auto a=model(d.path(),"first"),b=model(d.path(),"second");
        QFile source(a);QVERIFY(source.open(QIODevice::ReadOnly));
        auto json=QJsonDocument::fromJson(source.readAll()).object();source.close();
        auto skins=json.value("skins").toObject();skins["alternate"]=skins.value("default");json["skins"]=skins;write(a,QJsonDocument(json).toJson());
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({a});
        const int alternate=c.state().value("skins").toStringList().indexOf("alternate");QVERIFY(alternate>=0);
        c.dispatch("skin.select",alternate);c.dispatch("slot.toggle",0);c.dispatch("track.toggle",0);
        c.dispatch("file.addSpine",b);
        QVERIFY(c.state().value("slots").toList()[0].toMap().value("visible").toBool());
        QVERIFY(c.state().value("selectedTracks").toList().isEmpty());
        QCOMPARE(c.state().value("selectedSkins").toList(),QVariantList{0});
        c.dispatch("layer.select",1);
        QVERIFY(!c.state().value("slots").toList()[0].toMap().value("visible").toBool());
        QCOMPARE(c.state().value("selectedTracks").toList(),QVariantList{0});
        QCOMPARE(c.state().value("selectedSkins").toList(),QVariantList{alternate});
        c.dispatch("layer.up",1);c.dispatch("layer.select",1);
        QVERIFY(c.state().value("slots").toList()[0].toMap().value("visible").toBool());
        c.dispatch("layer.select",0);c.dispatch("slot.clear");
        QVERIFY(c.state().value("slots").toList()[0].toMap().value("visible").toBool());
        QCOMPARE(c.snapshot()->draws.size(),size_t(2));
    }
    void backgroundsInterleaveWithLayers(){
        QTemporaryDir d;const auto a=model(d.path(),"first"),b=model(d.path(),"second"),image=d.path()+"/sky.png";
        QImage picture(4,4,QImage::Format_RGBA8888);picture.fill(Qt::red);QVERIFY(picture.save(image));
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({a});c.dispatch("file.addSpine",b);c.dispatch("background.open",image);
        const auto kinds=[&]{QStringList out;for(const auto& row:c.state().value("layerStack").toList())out.append(row.toMap().value("kind").toString()+":"+row.toMap().value("name").toString());return out;};
        const auto backgroundDraw=[&]{const auto s=c.snapshot();for(int i=0;i<int(s->draws.size());++i)if(s->textures.value(s->draws[size_t(i)].textureId).size()==QSize(4,4))return i;return -1;};
        QCOMPARE(kinds(),QStringList({"spine:second","spine:first","background:sky"}));
        QCOMPARE(backgroundDraw(),0);
        c.dispatch("layer.stackMove",QVariantMap{{"from",2},{"to",1}});
        QCOMPARE(kinds(),QStringList({"spine:second","background:sky","spine:first"}));
        QCOMPARE(backgroundDraw(),1);
        c.dispatch("layer.stackMove",QVariantMap{{"from",1},{"to",0}});
        QCOMPARE(kinds(),QStringList({"background:sky","spine:second","spine:first"}));
        QCOMPARE(backgroundDraw(),2);
        c.dispatch("layer.stackMove",QVariantMap{{"from",2},{"to",1}});
        QCOMPARE(kinds(),QStringList({"background:sky","spine:first","spine:second"}));
        QCOMPARE(firstLayer(c),QString("first"));
        QCOMPARE(backgroundDraw(),2);
        c.dispatch("background.open",image);
        QCOMPARE(kinds().last(),QString("background:sky"));
        QVERIFY(!c.state().value("backgrounds").toList()[0].toMap().value("selected").toBool());
    }
    void layerCommandsStayAvailableInLive2DMode(){
        QTemporaryDir d;const auto image=d.path()+"/sky.png";
        QImage picture(4,4,QImage::Format_RGBA8888);picture.fill(Qt::red);QVERIFY(picture.save(image));
        QQuickWindow w;ViewerController c;setup(w,c);c.dispatch("mode.toggle");c.dispatch("background.open",image);c.dispatch("background.open",image);
        QCOMPARE(c.state().value("mode").toString(),QString("live2d"));
        const auto caps=c.state().value("capabilities").toMap();
        for(const auto& name:{"layer.select","layer.stackMove","layer.remove"})QVERIFY2(caps.value(name).toBool(),name);
        c.dispatch("background.select",1);
        c.dispatch("layer.stackMove",QVariantMap{{"from",1},{"to",0}});
        QVERIFY(c.state().value("backgrounds").toList()[0].toMap().value("selected").toBool());
    }
    void backgroundsAloneAreSelectedForDragging(){
        QTemporaryDir d;const auto a=model(d.path(),"first"),image=d.path()+"/sky.png";
        QImage picture(4,4,QImage::Format_RGBA8888);picture.fill(Qt::red);QVERIFY(picture.save(image));
        QQuickWindow w;ViewerController c;setup(w,c);
        const auto selected=[&]{return c.state().value("backgrounds").toList().value(0).toMap().value("selected").toBool();};
        c.dispatch("background.open",image);QVERIFY(selected());
        c.openPaths({a});QVERIFY(!selected());
        c.dispatch("layer.remove",0);QVERIFY(selected());
        const auto before=c.snapshot()->draws.front().vertices.front().pos;
        c.pointerPress({100,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerMove({100,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerMove({140,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerRelease({140,100},Qt::LeftButton,Qt::NoModifier);
        QVERIFY(c.snapshot()->draws.front().vertices.front().pos.x>before.x);
    }
    void layerCardAppearsWithBackgroundsAndStaysUntilEmpty(){
        QTemporaryDir d;const auto a=model(d.path(),"first"),b=model(d.path(),"second"),image=d.path()+"/sky.png";
        QImage picture(4,4,QImage::Format_RGBA8888);picture.fill(Qt::red);QVERIFY(picture.save(image));
        QQuickWindow w;ViewerController c;setup(w,c);
        const auto shown=[&]{return c.state().value("showLoadedSpines").toBool();};
        c.openPaths({a});QVERIFY(!shown());
        c.dispatch("background.open",image);QVERIFY(shown());
        c.dispatch("background.remove",0);QVERIFY(shown());
        c.openPaths({b});QVERIFY(shown());
        c.dispatch("layer.remove",0);QVERIFY(!shown());
        c.dispatch("background.open",image);QVERIFY(shown());
        c.dispatch("background.clear");QVERIFY(!shown());
        c.openPaths({a});c.dispatch("file.addSpine",b);QVERIFY(shown());
    }
    void removingLayersKeepsBackgroundsInPlace(){
        QTemporaryDir d;const auto a=model(d.path(),"first"),b=model(d.path(),"second"),image=d.path()+"/sky.png";
        QImage picture(4,4,QImage::Format_RGBA8888);picture.fill(Qt::red);QVERIFY(picture.save(image));
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({a});c.dispatch("file.addSpine",b);c.dispatch("background.open",image);
        c.dispatch("layer.stackMove",QVariantMap{{"from",2},{"to",1}});
        const auto kinds=[&]{QStringList out;for(const auto& row:c.state().value("layerStack").toList())out.append(row.toMap().value("kind").toString()+":"+row.toMap().value("name").toString());return out;};
        QCOMPARE(kinds(),QStringList({"spine:second","background:sky","spine:first"}));
        QVERIFY(c.state().value("showLoadedSpines").toBool());
        c.dispatch("background.remove",0);
        QVERIFY(c.state().value("showLoadedSpines").toBool());
        c.dispatch("background.open",image);
        c.dispatch("layer.stackMove",QVariantMap{{"from",2},{"to",1}});
        c.dispatch("layer.remove",1);
        QCOMPARE(kinds(),QStringList({"spine:second","background:sky"}));
        QCOMPARE(c.state().value("currentFileName").toString(),QString("second"));
        QVERIFY(c.state().value("loaded").toBool());
        QCOMPARE(c.snapshot()->draws.size(),size_t(2));
        c.dispatch("background.remove",0);
        QVERIFY(c.state().value("showLoadedSpines").toBool());
        c.dispatch("background.open",image);
        c.dispatch("layer.remove",0);
        QCOMPARE(kinds(),QStringList({"background:sky"}));
        QVERIFY(!c.state().value("loaded").toBool());
        QVERIFY(c.state().value("showLoadedSpines").toBool());
        c.dispatch("background.select",0);
        const auto before=c.snapshot()->draws.front().vertices.front().pos;
        c.pointerPress({100,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerMove({100,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerMove({130,100},Qt::LeftButton,Qt::NoModifier);
        c.pointerRelease({130,100},Qt::LeftButton,Qt::NoModifier);
        QVERIFY(c.snapshot()->draws.front().vertices.front().pos.x>before.x);
        c.dispatch("background.remove",0);
        QVERIFY(!c.state().value("showLoadedSpines").toBool());
        c.dispatch("background.open",image);
        QCOMPARE(c.snapshot()->draws.size(),size_t(1));
        c.openPaths({a});
        QCOMPARE(kinds(),QStringList({"spine:first","background:sky"}));
    }
    void failedSecondLayerDoesNotLeaveOldRows(){
        QTemporaryDir d;const auto old=model(d.path(),"old"),good=model(d.path(),"new"),bad=model(d.path(),"bad");
        auto atlas=QByteArray::fromStdString(sl_test::Fixture("3.6").atlasData.front());atlas.replace("\nprobe\n","\nmissing-region\n");
        write(d.path()+"/bad.atlas",atlas);
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({old});c.openPaths({good,bad});
        QVERIFY(!c.state().value("lastError").toString().isEmpty());
        QCOMPARE(c.state().value("loadedSpines").toList().size(),1);QCOMPARE(firstLayer(c),QString("new"));
        QCOMPARE(c.state().value("currentFileName").toString(),QString("new"));
    }
    void failedOtherVersionKeepsThePreviousLane(){
        QTemporaryDir d;const auto old=model(d.path(),"working"),bad=model(d.path(),"bad");
        const auto fixture=sl_test::Fixture("3.8");auto atlas=QByteArray::fromStdString(fixture.atlasData.front());atlas.replace("\nprobe\n","\nmissing-region\n");
        write(d.path()+"/bad.atlas",atlas);write(bad,QByteArray::fromStdString(fixture.skeletonData.front()));
        QQuickWindow w;ViewerController c;setup(w,c);c.openPaths({old});c.openPaths({bad});
        QVERIFY(!c.state().value("lastError").toString().isEmpty());QVERIFY(c.state().value("loaded").toBool());
        QCOMPARE(firstLayer(c),QString("working"));QCOMPARE(c.snapshot()->draws.size(),size_t(1));
    }
    void backgroundsStackAndReorder(){
        QTemporaryDir d;const auto first=d.path()+"/first.png",second=d.path()+"/second.png";
        QImage picture(4,4,QImage::Format_RGBA8888);picture.fill(Qt::cyan);QVERIFY(picture.save(first));picture.fill(Qt::red);QVERIFY(picture.save(second));
        QQuickWindow w;ViewerController c;setup(w,c);
        c.dispatch("background.open",first);c.dispatch("background.open",second);
        auto rows=c.state().value("backgrounds").toList();QCOMPARE(rows.size(),2);
        QCOMPARE(rows[0].toMap().value("name").toString(),QString("second"));QVERIFY(rows[0].toMap().value("selected").toBool());
        QCOMPARE(c.snapshot()->draws.size(),size_t(2));
        c.dispatch("background.select",0);
        c.dispatch("background.move",QVariantMap{{"from",0},{"to",1}});
        rows=c.state().value("backgrounds").toList();
        QCOMPARE(rows[1].toMap().value("name").toString(),QString("second"));QVERIFY(rows[1].toMap().value("selected").toBool());
        c.dispatch("background.visible",0);QCOMPARE(c.snapshot()->draws.size(),size_t(1));QVERIFY(c.state().value("hasBackgroundImage").toBool());
        c.dispatch("background.visible",1);QVERIFY(!c.state().value("hasBackgroundImage").toBool());
        c.dispatch("background.remove",1);rows=c.state().value("backgrounds").toList();
        QCOMPARE(rows.size(),1);QCOMPARE(rows[0].toMap().value("name").toString(),QString("first"));QVERIFY(rows[0].toMap().value("selected").toBool());
        c.dispatch("background.clear");QVERIFY(c.state().value("backgrounds").toList().isEmpty());
    }
    void badBackgroundKeepsTheCurrentArtwork(){
        QTemporaryDir d;const auto path=d.path()+"/background.png",bad=d.path()+"/bad.png";
        QImage picture(4,4,QImage::Format_RGBA8888);picture.fill(Qt::cyan);QVERIFY(picture.save(path));write(bad,"not an image");
        QQuickWindow w;ViewerController c;setup(w,c);c.dispatch("background.open",path);
        const auto before=c.snapshot();QCOMPARE(before->draws.size(),size_t(1));
        c.dispatch("background.open",bad);QVERIFY(!c.state().value("lastError").toString().isEmpty());QCOMPARE(c.snapshot(),before);
    }
};
int main(int argc,char** argv){
    QApplication app(argc,argv);app.setQuitOnLastWindowClosed(false);
    QTemporaryDir preferences;QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,preferences.path());
    const auto args=app.arguments();if(args.size()==4&&args[1]=="--load-benchmark")return viewerLoadBenchmark(args[2],args[3]);
    ControllerTest tests;return QTest::qExec(&tests,argc,argv);
}
#include "viewer_controller_test.moc"
