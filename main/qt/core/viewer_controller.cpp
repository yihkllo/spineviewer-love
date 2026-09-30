#include "viewer_controller.h"
#include "archive_cache.h"
#include "spinelove/asset_path.h"
#include "../render/scene_hit_test.h"
#include "../plugin_api/plugin_host.h"
#include "spinelove/plugin_module.h"
#include "spinelove/interaction_rules.h"
#include "../render/slot_outline.h"
#include "spinelove/texture_loader.h"
#include "window_resolution_presets.h"
#include <QApplication>
#include <QFileDialog>
#include <QColorDialog>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QQuickWindow>
#include <QQuickItem>
#include <QKeyEvent>
#include <QThreadPool>
#include <QScreen>
#include <QCursor>
#include <QCloseEvent>
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <algorithm>
#include <cmath>
#include <functional>

namespace slqt {
void applyWindowCorners(QWindow* window,bool round);
namespace {
class LayeredRenderer final:public SlSceneRenderer{
public:
    LayeredRenderer(SlSceneRenderer& target,std::vector<int> order,std::function<void(int)> before):m_target(target),m_order(std::move(order)),m_before(std::move(before)){}
    SlTextureId LoadTextureUtf8(const char* path,bool premultiplyAlpha)override{return m_target.LoadTextureUtf8(path,premultiplyAlpha);}
    void ReleaseTexture(SlTextureId texture)noexcept override{m_target.ReleaseTexture(texture);}
    void Submit(const SlDrawList& commands,const std::unordered_map<std::uint64_t,SlTextureId>& textures)override{
        if(m_next<m_order.size())m_before(m_order[m_next++]);
        m_target.Submit(commands,textures);
    }
private:
    SlSceneRenderer& m_target;
    std::vector<int> m_order;
    std::function<void(int)> m_before;
    size_t m_next=0;
};
QString from(const std::string& s){return QString::fromUtf8(s.data(),qsizetype(s.size()));}
QString modelName(const QString& path){auto name=QFileInfo(path).fileName();if(name.endsWith(".model3.json",Qt::CaseInsensitive)){name.chop(12);return name;}return AssetLibrary::skeletonStem(path);}
QVariantList indexes(const QSet<int>& s){auto list=s.values();std::sort(list.begin(),list.end());QVariantList v;for(int i:list)v.append(i);return v;}
std::string utf8(const QString& s){const auto b=s.toUtf8();return std::string(b.constData(),size_t(b.size()));}
float zoom(float scale,int steps,bool up,float low,float high){float f=std::pow(1.05f,float(std::abs(steps)));return std::clamp(up?scale*f:scale/f,low,high);}
struct UiLanguage{const char* id;const char* name;const char* code;};
constexpr UiLanguage uiLanguages[]{{"zh_CN","简体中文","zh-CN"},{"en","English","en-US"}};
QString supportedLanguage(const QString& requested){
    for(const auto& language:uiLanguages)
        if(requested==QLatin1String(language.id)||requested==QLatin1String(language.code))return QString::fromLatin1(language.id);
    return {};
}
QString startupLanguage(const QString& saved){const auto language=supportedLanguage(saved);return language.isEmpty()?QStringLiteral("zh_CN"):language;}
const QStringList& listKeys(){
    static const QStringList keys{"animations","skins","files","slots","queue","loadedSpines","capabilities",
        "languages","parameters","parts","expressions","gazeChannels"};
    return keys;
}
const QStringList& internalKeys(){
    static const QStringList keys{"live2dRevision","live2dDeviceGeneration","live2dRecoveryCount","exportFrameSerial",
        "exportAckId","exportRequestId","exportRequestSuccess","exportError","nativeHitResults","renderBounds"};
    return keys;
}
}
ViewerController::ViewerController(QObject* parent):QObject(parent),m_settings(QSettings::defaultFormat(),QSettings::UserScope,"SpineLoveEX","QtMigration") {
    m_lists=new QQmlPropertyMap(this);
    for(const auto& key:listKeys())m_lists->insert(key,key=="capabilities"?QVariant(QVariantMap{}):QVariant(QVariantList{}));
    m_hub.RebuildRuntimePool();
    m_live2d=std::make_shared<Live2DBridge>();
    m_plugins=new PluginHost(this);
    connect(m_plugins,&PluginHost::failure,this,&ViewerController::fail);
    connect(m_plugins,&PluginHost::settingsRequested,this,&ViewerController::applyPluginSettings);
    connect(m_plugins,&PluginHost::stateChanged,this,[this]{
        const bool open=m_plugins->isOpen();
        if(open&&!m_pluginsWasOpen){
            m_live2d->command("live2d.pausePreview");m_pointerMode=0;m_dragged=true;m_filePreviewPending=false;
        }
        windowCommand("plugin.portrait",m_plugins->state().value("portrait").toBool());
        m_pluginsWasOpen=open;syncPluginSuspension();m_clock.restart();refresh();record();
    });
    connect(qApp,&QGuiApplication::applicationStateChanged,this,[this](Qt::ApplicationState state){syncPluginSuspension();if(state==Qt::ApplicationActive&&checkFavoriteFiles())refresh();});
    const double uiScale=m_settings.value("uiScale",1.0).toDouble();
    m_favorites=m_settings.value("favorites").toStringList();
    m_favorites.removeDuplicates();
    std::sort(m_favorites.begin(),m_favorites.end());
    loadFavoriteFolders();
    checkFavoriteFiles();
    m_state={{"mode","spine"},{"uiScale",uiScale},{"baseFontPixels",16.0*uiScale},{"titleScale",uiScale},{"windowTitle","spinelove"},
        {"windowIcon",QUrl("qrc:/app.png")},{"exportAlpha",true},{"exportQueue",false},{"exportImageFps",30},{"exportVideoFps",60},{"exportGifFps",50},
        {"language",startupLanguage(m_settings.value("language").toString())},{"themeHue",0.74},{"themeSaturation",0.83},{"themeBrightness",1.0},{"darkTheme",false},{"themeCustomized",false},
        {"resolutionPreset",m_settings.value("resolutionPreset",0)},{"stageDecor",m_settings.value("stageDecor",true).toBool()},{"stageChecker",m_settings.value("stageChecker",false).toBool()&&!m_settings.value("stageDecor",true).toBool()},{"stageDecorStyle",std::clamp(m_settings.value("stageDecorStyle",0).toInt(),0,1)},{"infoCardHidden",m_settings.value("infoCardHidden",false).toBool()},{"exportButtonHidden",m_settings.value("exportButtonHidden",false).toBool()},{"spineRuntimeAvailable",true},{"slotHoverColor","#00ff00"},{"slotBoundsVisible",false}};
    QVariantList languages;for(const auto& language:uiLanguages)
        languages.append(QVariantMap{{"id",QString::fromLatin1(language.id)},{"name",QString::fromUtf8(language.name)}});
    m_state["languages"]=languages;
    m_settings.remove("renderSizeCustom");
    m_settings.remove("renderWindowWidth");
    m_settings.remove("renderWindowHeight");
    m_petMoveTimer.setSingleShot(true);m_petMoveTimer.setInterval(8);
    connect(&m_petMoveTimer,&QTimer::timeout,this,&ViewerController::applyDesktopPetMove);
    QThreadPool::globalInstance()->start([]{ExportService::removeStaleFrameFolders(24*60*60);});
    m_clock.start();refresh();record();
}
QObject* ViewerController::plugins()const{return m_plugins;}
void ViewerController::syncPluginSettings(){
    if(!m_plugins||m_state.isEmpty())return;
    const QString language=m_state.value("language",QStringLiteral("zh_CN")).toString();
    const int width=m_window?qRound(m_window->width()*m_window->devicePixelRatio()):m_state.value("windowWidth").toInt();
    const int height=m_window?qRound(m_window->height()*m_window->devicePixelRatio()):m_state.value("windowHeight").toInt();
    const QString resolution=QString::number(width)+"x"+QString::number(height);
    QVariantList resolutions;QSet<QString> resolutionKeys;
    const auto addResolution=[&](int w,int h){
        if(w<=0||h<=0)return;
        const QString value=QString::number(w)+"x"+QString::number(h);
        if(resolutionKeys.contains(value))return;
        resolutionKeys.insert(value);resolutions.append(QVariantMap{{"value",value},{"text",QString::number(w)+QStringLiteral(" × ")+QString::number(h)}});
    };
    addResolution(width,height);
    for(int i=1;i<window_resolution_presets::Count();++i){const auto* preset=window_resolution_presets::Get(i);addResolution(preset->width,preset->height);}
    QVariantList languages;
    for(const auto& language:uiLanguages)
        languages.append(QVariantMap{{"value",QString::fromLatin1(language.code)},{"text",QString::fromUtf8(language.name)}});
    int mode=3;
    if(m_window&&m_window->visibility()==QWindow::FullScreen){
        mode=m_settings.value("pluginFullscreenMode",1).toInt();if(mode!=0&&mode!=1)mode=1;
    }
    const bool english=language.startsWith("en",Qt::CaseInsensitive);
    const QVariantList modes{
        QVariantMap{{"value",0},{"text",english?QStringLiteral("Full screen"):QStringLiteral("全屏")}},
        QVariantMap{{"value",1},{"text",english?QStringLiteral("Borderless full screen"):QStringLiteral("无边框全屏")}},
        QVariantMap{{"value",3},{"text",english?QStringLiteral("Windowed"):QStringLiteral("窗口")}}
    };
    m_plugins->setHostSettings({{"language",language},{"languageOptions",languages},{"resolution",resolution},
        {"resolutionOptions",resolutions},{"windowMode",mode},{"windowModeOptions",modes}});
}
void ViewerController::applyPluginSettings(const QString& action,const QVariant& value){
    if(m_petMode||m_exportActive)return;
    if(action=="reset"){
        const auto defaults=value.toMap();
        for(const auto& key:{QStringLiteral("language"),QStringLiteral("resolution"),QStringLiteral("windowMode")})
            if(defaults.contains(key))applyPluginSettings(key,defaults.value(key));
        return;
    }
    if(action=="language"){
        const QString language=supportedLanguage(value.toString());
        if(!language.isEmpty()&&language!=m_state.value("language").toString()){
            m_state["language"]=language;savePreferences();emit languageRequested(language);refresh();
        }
    }else if(action=="resolution"&&m_window){
        const auto match=QRegularExpression(QStringLiteral("^\\s*(\\d{3,5})\\s*[xX×]\\s*(\\d{3,5})\\s*$")).match(value.toString());
        if(match.hasMatch())windowCommand("settings.resolution.custom",QVariantMap{{"width",match.captured(1).toInt()},{"height",match.captured(2).toInt()}});
    }else if(action=="windowMode"&&m_window){
        bool valid=false;const int mode=value.toInt(&valid);
        if(valid&&(mode==0||mode==1||mode==3)){
            if(mode==3){
                if(m_window->visibility()==QWindow::FullScreen)windowCommand("window.fullscreen",{});
                if(m_window->visibility()!=QWindow::Windowed)m_window->showNormal();
            }else{
                m_settings.setValue("pluginFullscreenMode",mode);
                if(m_window->visibility()!=QWindow::FullScreen)windowCommand("window.fullscreen",{});
            }
            refresh();
        }
    }
    syncPluginSettings();
}
void ViewerController::syncPluginSuspension(){
    m_plugins->setSuspended(modalOpen()||(m_window&&(!m_window->isVisible()||m_window->visibility()==QWindow::Minimized))||
        QGuiApplication::applicationState()==Qt::ApplicationSuspended||QGuiApplication::applicationState()==Qt::ApplicationHidden);
}
void ViewerController::setWindow(QQuickWindow* window){
    if(m_window)m_window->removeEventFilter(this);m_window=window;
    if(window){
        window->installEventFilter(this);m_defaultWindowSize=window->size();
        applyWindowCorners(window,!m_petMode);
        connect(window,&QWindow::visibilityChanged,this,[this]{syncPluginSuspension();m_clock.restart();refresh();});
        connect(window,&QWindow::widthChanged,this,[this]{syncPluginSettings();});
        connect(window,&QWindow::heightChanged,this,[this]{syncPluginSettings();});
    }
    syncPluginSettings();
}
void ViewerController::setViewport(QSizeF logical,qreal dpr){
    if(logical.isEmpty()||!std::isfinite(dpr)||dpr<=0)return;
    m_previewSize=logical;
    if(m_petMode&&!live2dMode()){m_dpr=dpr;return;}
    const QSize next=renderSize().isEmpty()?QSize(qMax(1,int(std::ceil(logical.width()*dpr))),qMax(1,int(std::ceil(logical.height()*dpr)))):renderSize();
    if(m_viewportInitialized&&next==m_viewport&&dpr==m_dpr)return;
    m_viewportInitialized=true;m_dpr=dpr;m_viewport=next;
    runtime()->SetViewportSize(m_viewport.width(),m_viewport.height());fit();refresh(false);record();
    if(!m_viewportNotificationPending){
        m_viewportNotificationPending=true;
        QTimer::singleShot(0,this,[this]{
            if(!m_viewportNotificationPending)return;
            publishState();
        });
    }
}
void ViewerController::fail(const QString& error){m_state["lastError"]=error;publishState();emit errorOccurred(error);}
void ViewerController::loadFavoriteFolders(){
    m_favoriteFolders.clear();
    const auto saved=QJsonDocument::fromJson(m_settings.value("folderFavorites").toString().toUtf8()).array();
    for(const auto& entry:saved){
        const auto o=entry.toObject();FavoriteFolder folder{o.value("id").toString(),o.value("name").toString().trimmed(),{}};
        if(folder.id.isEmpty()||favoriteFolder(folder.id))continue;
        for(const auto& item:o.value("items").toArray()){const auto p=item.toString();if(m_favorites.contains(p)&&!folder.items.contains(p))folder.items.append(p);}
        if(folder.id!=QStringLiteral("default")&&folder.name.isEmpty())continue;
        m_favoriteFolders.append(folder);
    }
    const auto found=std::find_if(m_favoriteFolders.begin(),m_favoriteFolders.end(),[](const FavoriteFolder& f){return f.id==QStringLiteral("default");});
    if(found==m_favoriteFolders.end())m_favoriteFolders.prepend(FavoriteFolder{QStringLiteral("default"),{},{}});
    else if(found!=m_favoriteFolders.begin()){auto folder=*found;m_favoriteFolders.erase(found);m_favoriteFolders.prepend(folder);}
    m_favoriteFolders.front().name.clear();
    for(const auto& p:m_favorites)if(std::none_of(m_favoriteFolders.begin(),m_favoriteFolders.end(),[&](const FavoriteFolder& f){return f.items.contains(p);}))m_favoriteFolders.front().items.append(p);
}
QStringList ViewerController::favoritePaths(const QVariantMap& m){
    QStringList paths;
    auto source=m.value("paths").toStringList();if(source.isEmpty())source.append(m.value("path").toString());
    for(const auto& raw:source){const auto p=ArchiveCache::displayPath(raw);if(!p.isEmpty()&&!paths.contains(p))paths.append(p);}
    return paths;
}
ViewerController::FavoriteFolder* ViewerController::favoriteFolder(const QString& id){
    for(auto& f:m_favoriteFolders)if(f.id==id)return &f;
    return nullptr;
}
void ViewerController::syncFavorites(){
    QStringList all;for(const auto& f:m_favoriteFolders)for(const auto& p:f.items)if(!all.contains(p))all.append(p);
    std::sort(all.begin(),all.end());m_favorites=all;
    if(!favoriteFolder(m_favoriteFolder))m_favoriteFolder=QStringLiteral("default");
    checkFavoriteFiles();++m_favoriteRevision;savePreferences();
}
bool ViewerController::checkFavoriteFiles(){
    QSet<QString> missing;QHash<QString,bool> roots;
    for(const auto& p:m_favorites){
        const QString root=p.startsWith(QLatin1String("//"))?p.section(QLatin1Char('/'),0,3):p.section(QLatin1Char('/'),0,0)+QLatin1Char('/');
        auto found=roots.find(root);if(found==roots.end())found=roots.insert(root,QFileInfo::exists(root));
        if(!found.value()||!ArchiveCache::exists(p))missing.insert(p);
    }
    if(missing==m_missingFavorites)return false;
    m_missingFavorites=missing;++m_favoriteRevision;return true;
}
QVariantList ViewerController::favoriteFolderRows()const{
    QVariantList rows;
    for(const auto& f:m_favoriteFolders){
        int count=0;for(const auto& p:f.items)if(AssetLibrary::isLive2DFileName(p)==live2dMode())++count;
        rows.append(QVariantMap{{"id",f.id},{"name",f.name},{"isDefault",f.id==QStringLiteral("default")},{"count",count},{"selected",f.id==m_favoriteFolder}});
    }
    return rows;
}
void ViewerController::savePreferences(){
    QJsonArray folders;
    for(const auto& f:m_favoriteFolders)folders.append(QJsonObject{{"id",f.id},{"name",f.name},{"items",QJsonArray::fromStringList(f.items)}});
    m_settings.setValue("folderFavorites",QString::fromUtf8(QJsonDocument(folders).toJson(QJsonDocument::Compact)));
    m_settings.setValue("favorites",m_favorites);m_settings.setValue("language",m_state.value("language"));m_settings.sync();}
bool ViewerController::inputBlocked()const{return modalOpen()||m_exportActive||m_plugins->isOpen()||m_state.value("replaceConfirmation").toMap().value("open").toBool();}
void ViewerController::openUrls(const QList<QUrl>& urls){if(inputBlocked())return;if(urls.isEmpty())return;QString error;const auto path=AssetLibrary::localPath(urls.front(),&error);if(path.isEmpty()){fail(error);return;}openPaths({path});}
void ViewerController::setMode(bool live){
    if(live==live2dMode())return;
    if(live2dMode()){m_live2dFiles=m_files;m_live2d->command("live2d.suspend");}
    else m_spineFiles=m_files;
    m_state["mode"]=live?"live2d":"spine";m_files=live?m_live2dFiles:m_spineFiles;for(auto& b:m_backgrounds)b.level=0;
    const int selected=int(runtime()->ActiveSkeletonIndex());
    m_currentPath=live?m_live2dPath:(selected>=0&&selected<m_layers.size()?m_layers[selected]:QString{});
    m_filePreviewPending=false;m_pointerMode=0;m_dragged=true;m_wheelRemainder=0;m_wheelTarget=0;
    m_hoverPosition={-1,-1};m_hoveredSlot.clear();m_listHoveredSlot.clear();m_clock.restart();
}
void ViewerController::openPaths(const QStringList& paths,bool confirmed){
    if(m_plugins->isOpen())return;
    if(m_exportActive){fail(tr("An export is in progress."));return;}
    if(paths.isEmpty())return;
    m_filePreviewPending=false;
    if(ArchiveCache::isArchive(paths.first())&&QFileInfo(paths.first()).isFile()){openArchive(paths.first());return;}
    QStringList resolved;
    for(const auto& path:paths){QString error;const auto real=ArchiveCache::resolve(path,&error);if(real.isEmpty()){fail(error);return;}resolved.append(real);}
    if(resolved!=paths){openPaths(resolved,confirmed);return;}
    if(paths.size()==1&&QFileInfo(paths.first()).isDir()){scanFolder(paths.first(),false,true);return;}
    if(AssetLibrary::isLive2DFileName(paths.first())){
        const auto entry=AssetLibrary::inspect(paths.first());if(!entry.isValid()){fail(entry.error);return;}
        setMode(true);m_live2dPath=entry.path;m_currentPath=entry.path;
        if(paths.size()>1){m_live2dFiles.clear();for(const auto& path:paths)if(AssetLibrary::isLive2DFileName(path))m_live2dFiles.append(QFileInfo(path).absoluteFilePath());std::sort(m_live2dFiles.begin(),m_live2dFiles.end());}
        else if(const QString shown=ArchiveCache::displayPath(entry.path);!m_live2dFiles.contains(shown)&&!m_live2dFiles.contains(entry.path))m_live2dFiles.append(shown);
        m_files=m_live2dFiles;for(auto& b:m_backgrounds)b.level=0;m_live2dHidden=false;m_live2d->command("live2d.open",entry.path);m_lastLiveError.clear();
        m_clock.restart();refresh();record();return;
    }
    if(!confirmed&&m_layers.size()>1){m_pendingPaths=paths;m_state["replaceConfirmation"]=QVariantMap{{"open",true},{"title",tr("Warning")},{"message",tr("Replace the currently loaded Spine layers?")}};publishState();return;}
    const bool profile=QCoreApplication::instance()->property("qaProfileLoads").toBool();
    QElapsedTimer loadClock;if(profile)loadClock.start();
    auto stamp=[&]{return profile?loadClock.nsecsElapsed()/1e6:0.;};
    const auto bundle=AssetLibrary::readSpineBundle(paths);
    const double readMs=stamp();
    if(!bundle.isValid()){fail(bundle.error);return;}
    auto lane=m_hub.LaneForVersionText(bundle.items.first().spineVersion.toUtf8().constData());
    if(lane==SlRuntimeHub::RuntimeLane::Unknown){fail(tr("Unsupported Spine version: ")+bundle.items.first().spineVersion);return;}
    setMode(false);
    const float priorScale=runtime()->SkeletonScale(),priorSpeed=runtime()->TimeScale();
    const auto previousLane=m_hub.CurrentLane();m_hub.ActivateLane(lane);auto* r=runtime();
    const bool hadLoaded=r->ContainsDrawableContent();
    if(hadLoaded)m_lastAnimationName=r->ActiveMotionName();
    r->SetResetViewOnLoad(m_resetOnLoad);r->SetViewportSize(m_viewport.width(),m_viewport.height());
    SlMemoryBundleRequest req;req.binarySkeleton=bundle.binarySkeleton;
    for(const auto& x:bundle.atlasBytes)req.atlasText.emplace_back(x.constData(),size_t(x.size()));
    for(const auto& x:bundle.skeletonBytes)req.skeletonBytes.emplace_back(x.constData(),size_t(x.size()));
    for(const auto& x:bundle.textureDirectories)req.textureRoots.push_back(utf8(x));
    if(!r->LoadBundleFromMemory(req)){
        const auto error=from(r->LastRuntimeIssue());
        if(lane!=previousLane)m_hub.ActivateLane(previousLane);
        else {
            m_layerControls.clear();m_layers.clear();for(auto& b:m_backgrounds)b.level=0;
            for(int i=0;i<qMin(int(r->LoadedSkeletonCount()),int(bundle.items.size()));++i)m_layers.append(bundle.items[i].path);
            r->ChooseSkeleton(0);m_currentPath=m_layers.isEmpty()?QString{}:m_layers.front();
            m_queue.clear();m_queuePlaying=false;m_queueIndex=0;m_selectedSkins.clear();m_tracks.clear();m_hiddenSlots.clear();
            m_pinnedSlot.clear();m_hoveredSlot.clear();m_listHoveredSlot.clear();updateSlotFilter();
        }
        fail(error);refresh();record();return;
    }
    const double runtimeMs=stamp();
    r->SetSkeletonScale(priorScale);r->SetTimeScale(priorSpeed);
    if(r->IsMirroredHorizontally())r->ToggleMirrorX();while(r->QuarterTurns()!=0)r->RotateClockwise();
    r->SetPremultipliedAlpha(m_pma);r->SetBlendWindowSeconds(m_mix);
    if(std::find(r->MotionNames().begin(),r->MotionNames().end(),m_lastAnimationName)!=r->MotionNames().end())r->PlayMotionByName(m_lastAnimationName.c_str());
    r->TickPlayback(0);if(m_resetOnLoad)r->SetViewOffset(0,0);else r->CenterOpeningPoseInView();
    m_layerControls.clear();m_layers.clear();for(auto& b:m_backgrounds)b.level=0;for(const auto& i:bundle.items)m_layers.append(i.path);
    m_currentPath=m_layers.front();m_version=bundle.items.front().spineVersion;m_queue.clear();m_queuePlaying=false;m_queueIndex=0;
    m_selectedSkins.clear();m_tracks.clear();m_hiddenSlots.clear();m_hoveredSlot.clear();m_listHoveredSlot.clear();m_pinnedSlot.clear();m_outlineCache.clear();
    const auto names=r->LookNames();std::vector<std::string> cached;
    if(m_cachedWasMix){for(const auto& name:m_cachedMixSkins)cached.push_back(utf8(name));}
    else if(!m_cachedSkin.isEmpty())cached.push_back(utf8(m_cachedSkin));
    const auto restored=interaction::restoreSkinSelection(names,cached,m_skinMix,m_cachedWasMix,r->ActiveLookName());
    m_lastMixedSkin=-1;
    for(const auto& name:restored){const auto found=std::find(names.begin(),names.end(),name);if(found!=names.end()){const int index=int(found-names.begin());m_selectedSkins.insert(index);if(m_skinMix)m_lastMixedSkin=index;}}
    if(restored.size()>1)r->ComposeLooks(restored);else if(restored.size()==1&&restored.front()!=r->ActiveLookName())r->ApplyLookByName(restored.front().c_str());
    m_selectedBackground=-1;
    m_previewIndex=qMax(0,int(m_files.indexOf(ArchiveCache::displayPath(m_currentPath))));m_state["mode"]="spine";m_state["lastError"]="";
    updateSlotFilter();fit();m_clock.restart();
    const double restoreMs=stamp();refresh();const double stateMs=stamp();record();
    if(profile)setProperty("_qaLoadTiming",QVariantMap{{"readMs",readMs},{"runtimeMs",runtimeMs-readMs},
        {"restoreMs",restoreMs-runtimeMs},{"stateMs",stateMs-restoreMs},{"recordMs",stamp()-stateMs},{"totalMs",stamp()}});
}
void ViewerController::openArchive(const QString& path){
    QString error;const auto files=ArchiveCache::listSpine(path,&error);
    if(files.isEmpty()){
        QString extractError;const auto folder=ArchiveCache::extract(path,&extractError);
        if(!folder.isEmpty()&&!AssetLibrary::scanLive2D(folder).isEmpty()){scanFolder(folder,false,true);return;}
    }
    if(files.isEmpty()){fail(error.isEmpty()?tr("No playable Spine model was found in %1. Export JSON or binary skeleton data from Spine first.").arg(QFileInfo(path).fileName()):error);return;}
    if(live2dMode())setMode(false);
    m_folder=QFileInfo(path).absolutePath();m_files=files;m_spineFiles=files;m_favoritesOnly=false;
    m_previewIndex=qMax(0,int(m_files.indexOf(ArchiveCache::displayPath(m_currentPath))));
    m_state["folderRevision"]=m_state.value("folderRevision",0).toInt()+1;
    if(files.size()==1){openPaths(files);return;}
    refresh();record();
}
void ViewerController::scanFolder(const QString& folder,bool openAll,bool allowModeFallback){
    auto files=live2dMode()?AssetLibrary::scanLive2D(folder):AssetLibrary::scanSpine(folder);
    if(files.isEmpty()&&allowModeFallback){
        const auto alternatives=live2dMode()?AssetLibrary::scanSpine(folder):AssetLibrary::scanLive2D(folder);
        if(!alternatives.isEmpty()){setMode(!live2dMode());files=alternatives;}
    }
    m_folder=folder;m_files=files;
    m_previewIndex=qMax(0,int(m_files.indexOf(ArchiveCache::displayPath(m_currentPath))));m_favoritesOnly=false;
    const auto announce=[this]{m_state["folderRevision"]=m_state.value("folderRevision",0).toInt()+1;refresh();};
    if(live2dMode()){m_live2dFiles=m_files;if(m_files.size()==1){openPaths({m_files.front()});announce();return;}}
    else m_spineFiles=m_files;
    if(openAll){openPaths(m_files);announce();}else {announce();record();}
}
void ViewerController::fit(){
    if(m_plugins->isOpen()||!renderSize().isEmpty())return;
    auto* r=runtime();if(!r->ContainsDrawableContent())return;
    if(!m_window||m_petMode||m_window->visibility()==QWindow::FullScreen)return;
    const auto b=r->BaseSize();const float scale=r->CanvasScale();
    if(b.x<=0||b.y<=0||!std::isfinite(scale)||scale<=0)return;
    const int panelPixels=qMax(0,qRound(m_window->width()*m_dpr)-m_viewport.width());
    const int chromePixels=qMax(0,qRound(m_window->height()*m_dpr)-m_viewport.height());
    const auto available=m_window->screen()->availableGeometry().size();
    const int width=std::clamp(panelPixels+int(std::ceil(b.x*scale)),320,qMax(320,qRound(available.width()*m_dpr)));
    const int height=std::clamp(chromePixels+int(std::ceil(b.y*scale)),240,qMax(240,qRound(available.height()*m_dpr)));
    m_window->resize(qRound(width/m_dpr),qRound(height/m_dpr));
}
void ViewerController::addLayer(const QString& source){
    QString path=source;
    if(ArchiveCache::isArchive(path)&&QFileInfo(path).isFile()){
        QString error;const auto folder=ArchiveCache::extract(path,&error);if(folder.isEmpty()){fail(error);return;}
        const auto files=AssetLibrary::scanSpine(folder);
        if(files.isEmpty()){fail(tr("No playable Spine model was found in %1. Export JSON or binary skeleton data from Spine first.").arg(QFileInfo(path).fileName()));return;}
        path=files.front();
    }
    else{QString error;path=ArchiveCache::resolve(path,&error);if(path.isEmpty()){fail(error);return;}}
    auto entry=AssetLibrary::inspect(path);if(!entry.isValid()||!entry.isSpine()){fail(entry.error);return;}
    const int already=int(m_layers.indexOf(entry.path));if(already>=0){selectLayer(already);refresh();record();return;}
    if(m_layers.isEmpty()){openPaths({entry.path});refresh();return;}
    if(m_hub.LaneForVersionText(entry.spineVersion.toUtf8().constData())!=m_hub.CurrentLane()){fail(tr("Additional layers must use the same Spine runtime version."));return;}
    const auto bundle=AssetLibrary::readSpineBundle({entry.path});if(!bundle.isValid()){fail(bundle.error);return;}
    SlMemoryBundleRequest req;req.binarySkeleton=bundle.binarySkeleton;
    for(const auto& text:bundle.atlasBytes)req.atlasText.emplace_back(text.constData(),size_t(text.size()));
    for(const auto& bytes:bundle.skeletonBytes)req.skeletonBytes.emplace_back(bytes.constData(),size_t(bytes.size()));
    for(const auto& path:bundle.textureDirectories)req.textureRoots.push_back(utf8(path));
    saveLayerControls();auto* r=runtime();if(!r->AddLayerFromMemory(req)){fail(from(r->LastRuntimeIssue()));return;}
    size_t i=r->LoadedSkeletonCount()-1;while(i>0&&r->PromoteSkeleton(i))--i;r->ChooseSkeleton(0);
    m_layers.prepend(entry.path);m_currentPath=entry.path;m_showLayers=true;
    const auto& motions=r->MotionNames();if(!motions.empty()&&std::find(motions.begin(),motions.end(),r->ActiveMotionName())==motions.end())r->PlayMotionByIndex(0);
    const auto& skins=r->LookNames();if(!skins.empty()&&std::find(skins.begin(),skins.end(),r->ActiveLookName())==skins.end())r->ApplyLookByIndex(0);
    restoreLayerControls();
    refresh();record();
}
void ViewerController::saveLayerControls(){
    const int index=int(runtime()->ActiveSkeletonIndex());if(index<0||index>=m_layers.size())return;
    m_layerControls[m_layers[index]]={m_selectedSkins,m_tracks,m_hiddenSlots,m_slotQuery,m_pinnedSlot,m_lastMixedSkin};
}
void ViewerController::restoreLayerControls(){
    const int index=int(runtime()->ActiveSkeletonIndex());if(index<0||index>=m_layers.size())return;
    const auto controls=m_layerControls.value(m_layers[index]);
    m_selectedSkins=controls.selectedSkins;m_tracks=controls.tracks;m_hiddenSlots=controls.hiddenSlots;
    m_slotQuery=controls.slotQuery;m_pinnedSlot=controls.pinnedSlot;m_lastMixedSkin=controls.lastMixedSkin;
    m_hoveredSlot.clear();m_listHoveredSlot.clear();
    const auto& names=runtime()->LookNames();
    if(!m_skinMix||m_selectedSkins.isEmpty()){
        m_selectedSkins.clear();const auto current=std::find(names.begin(),names.end(),runtime()->ActiveLookName());
        if(current!=names.end())m_selectedSkins.insert(int(current-names.begin()));else if(!names.empty())m_selectedSkins.insert(0);
    }
    updateSlotFilter();
}
bool ViewerController::hasBackground()const{return std::any_of(m_backgrounds.begin(),m_backgrounds.end(),[](const BackgroundLayer& b){return b.visible;});}
ViewerController::BackgroundLayer* ViewerController::activeBackground(){return m_selectedBackground>=0&&m_selectedBackground<m_backgrounds.size()?&m_backgrounds[m_selectedBackground]:nullptr;}
void ViewerController::clearBackgrounds(){for(const auto& b:m_backgrounds)m_recorder.ReleaseTexture(b.texture);m_backgrounds.clear();m_selectedBackground=-1;}
bool ViewerController::selectLayer(int index){
    if(index<0||index>=m_layers.size())return false;
    m_selectedBackground=-1;
    saveLayerControls();if(!runtime()->ChooseSkeleton(size_t(index)))return false;
    m_currentPath=m_layers[index];restoreLayerControls();return true;
}
QStringList ViewerController::visibleFiles()const{if(!m_favoritesOnly)return m_files;QStringList source;for(const auto& f:m_favoriteFolders)if(f.id==m_favoriteFolder)source=f.items;std::sort(source.begin(),source.end());QStringList visible;for(const auto& p:source)if(AssetLibrary::isLive2DFileName(p)==live2dMode())visible.append(p);return visible;}
bool ViewerController::previewFile(int delta,bool commit){
    auto list=m_files.isEmpty()?visibleFiles():m_files;list.removeIf([this](const QString& p){return m_missingFavorites.contains(p);});if(list.isEmpty())return false;
    int base=int(list.indexOf(ArchiveCache::displayPath(m_currentPath)));if(base<0)base=m_previewIndex;
    m_previewIndex=(base+delta+list.size())%list.size();m_currentPath=list[m_previewIndex];
    if(commit)openPaths({m_currentPath});else refresh();
    return true;
}
void ViewerController::playQueueItem(){
    if(m_queue.isEmpty())return;m_queueIndex%=m_queue.size();runtime()->PlayMotionByName(utf8(m_queue[m_queueIndex]).c_str());runtime()->RestartMotion(false);
}
void ViewerController::updateSlotFilter(){
    auto* r=runtime();std::vector<std::string> hidden;
    const auto& slotNames=r->SlotCatalog();
    for(int i=0;i<int(slotNames.size());++i){if(interaction::slotNameMatches(slotNames[size_t(i)],utf8(m_slotQuery)))m_hiddenSlots.insert(i);if(m_hiddenSlots.contains(i))hidden.push_back(slotNames[size_t(i)]);}
    r->SetSlotVisibilityRule(nullptr);
    r->SetHiddenSlots(hidden);
}
void ViewerController::tick(){
    const float dt=std::min(0.1f,float(m_clock.nsecsElapsed()/1e9));m_clock.restart();
    if(m_plugins->isOpen()||modalOpen()||m_exportActive||!m_window||!m_window->isVisible()||m_window->visibility()==QWindow::Minimized)return;
    if(m_petMode&&m_pointerMode==5)return;
    m_animationTime+=dt;
    if(m_petMode)updateDesktopPet();
    if(!live2dMode())runtime()->TickPlayback(dt);
    if(!live2dMode()&&m_queuePlaying&&!m_queue.isEmpty()){
        float track=0,last=0,start=0,end=0;runtime()->ReadMotionClock(&track,&last,&start,&end);
        if(interaction::queueReachedEnd(track,end)){m_queueIndex=(m_queueIndex+1)%m_queue.size();playQueueItem();refresh();}
    }
    if(live2dMode()){
        const quint64 revision=m_live2d->revision();
        if(revision!=m_live2dSeenRevision){m_live2dSeenRevision=revision;refresh();}
    }
    record();
    if(!live2dMode()&&m_hoverEnabled&&!m_petMode)hover(m_hoverPosition);
}
void ViewerController::recordSlotOverlay(){
    if(!m_hoverEnabled||m_exportActive||m_petMode)return;
    const auto transform=runtime()->ViewTransform();
    const QColor qcolor(m_state.value("slotHoverColor").toString());
    const SlColor color{float(qcolor.redF()),float(qcolor.greenF()),float(qcolor.blueF()),1.f};
    QStringList names;if(!m_listHoveredSlot.isEmpty())names.append(m_listHoveredSlot);
    if(!m_hoveredSlot.isEmpty()&&!names.contains(m_hoveredSlot))names.append(m_hoveredSlot);
    for(const auto& name:names){
        ReadSlotMeshData mesh;
        if(!runtime()->ReadSlotMesh(utf8(name),mesh)||mesh.worldVertices.size()<2){
            const auto b=runtime()->MeasureSlotBounds(utf8(name));if(b.z==0)continue;
            mesh.worldVertices={b.x,b.y,b.x+b.z,b.y,b.x+b.z,b.y+b.w,b.x,b.y+b.w};mesh.hullLength=4;mesh.isRegion=true;
        }
        const auto outline=m_outlineCache.build(utf8(name),mesh,m_recorder.texture(mesh.textureHandle),transform,color,3.2f,m_viewport);
        m_recorder.Submit(outline.draws,{{1,1}});
    }
}
void ViewerController::record(){
    if(m_petMode&&m_pointerMode==5&&m_snapshot)return;
    const bool stageBehind=(m_state.value("stageDecor",true).toBool()||m_state.value("stageChecker",false).toBool())&&!m_petMode&&!m_plugins->isActive();
    const bool decorBackground=stageBehind&&m_state.value("stageDecor",true).toBool();
    const bool decor=stageBehind&&!m_exportActive;
    m_recorder.begin(m_viewport,(m_captureAlpha||m_petMode||decor)?QColor(Qt::transparent):m_clearColor);
    if(decorBackground&&m_exportActive&&!m_captureAlpha&&m_decorTexture)m_recorder.sprite(m_decorTexture,{0,0,float(m_viewport.width()),float(m_viewport.height())});
    const int layerCount=live2dMode()?(m_state.value("loaded").toBool()?1:0):int(m_layers.size());int drawnLevel=-1;
    const auto drawLevels=[&](int upTo){
        if(m_captureAlpha||m_petMode)return;
        for(int level=drawnLevel+1;level<=upTo;++level)for(int i=int(m_backgrounds.size())-1;i>=0;--i){
            const auto& b=m_backgrounds[i];if(!b.visible||std::min(b.level,layerCount)!=level)continue;
            const auto img=m_recorder.texture(b.texture);m_recorder.sprite(b.texture,{float(b.offset.x()),float(b.offset.y()),img.width()*b.scale,img.height()*b.scale});
        }
        drawnLevel=std::max(drawnLevel,upTo);
    };
    if(!m_plugins->isActive()&&!live2dMode()){
        std::vector<int> order;for(int i=layerCount-1;i>=0;--i)if(runtime()->SkeletonLayerVisible(size_t(i)))order.push_back(i);
        LayeredRenderer layered(m_recorder,std::move(order),[&](int index){drawLevels(layerCount-1-index);});
        m_hub.RenderCurrentRuntime(layered);drawLevels(layerCount);recordSlotOverlay();
    }
    else if(live2dMode()&&!m_plugins->isActive()&&layerCount>0){
        drawLevels(0);
        const auto markerId=m_live2dHidden?live2dHiddenMarker:live2dLayerMarker;
        SlDrawList marker;marker.commands.resize(1);marker.commands.front().textureId=markerId;
        m_recorder.Submit(marker,{{markerId,SlTextureId(markerId)}});
        drawLevels(layerCount);
    }
    else drawLevels(layerCount);
    auto snapshot=std::const_pointer_cast<SceneSnapshot>(m_recorder.finish());
    snapshot->capture=m_captureRequest;
    snapshot->animationTime=m_animationTime;
    if(live2dMode()&&!m_plugins->isActive()){
        snapshot->live2d=m_live2d;snapshot->titleHeight=(m_petMode||m_state.value("fullscreen").toBool())?0:float(42*m_state.value("titleScale",1).toDouble());
        snapshot->live2dShaderPath=packagedAssetPath("render_d3d11/shaders/sprite.hlsl");
    }
    if(m_petMode&&!live2dMode())updateDesktopPetCanvas(*snapshot);
    m_snapshot=std::move(snapshot);
    if(m_petMode)updateDesktopPetRegion();
    emit frameChanged();
}
void ViewerController::refresh(bool notify){
    m_state["renderWidth"]=renderSize().width();m_state["renderHeight"]=renderSize().height();
    m_state["pluginsOpen"]=m_plugins->isOpen();m_state["pluginActive"]=m_plugins->isActive();
    const auto live=live2dMode()?m_live2d->state():QVariantMap{};
    auto* r=runtime();m_state["loaded"]=r->ContainsDrawableContent();m_state["devicePixelRatio"]=m_dpr;
    m_state["currentFileName"]=modelName(m_currentPath);m_state["scale"]=r->SkeletonScale();m_state["timeScale"]=r->TimeScale();m_state["defaultMix"]=m_mix;
    m_state["pma"]=m_pma;m_state["resetViewOnLoad"]=m_resetOnLoad;m_state["canvasWidth"]=m_viewport.width();m_state["canvasHeight"]=m_viewport.height();
    const auto size=r->SkeletonContentSize(),offset=r->ViewOffset();m_state["contentWidth"]=size.x;m_state["contentHeight"]=size.y;m_state["offsetX"]=offset.x;m_state["offsetY"]=offset.y;
    QVariantList motions;int active=-1;const auto& names=r->MotionNames();for(int i=0;i<int(names.size());++i){motions.append(QVariantMap{{"name",from(names[i])},{"duration",r->MotionDuration(names[i].c_str())}});if(names[i]==r->ActiveMotionName())active=i;}
    m_state["animations"]=motions;m_state["currentAnimation"]=active;
    QStringList skins;for(const auto& s:r->LookNames())skins.append(from(s));m_state["skins"]=skins;m_state["skinMix"]=m_skinMix;m_state["selectedSkins"]=indexes(m_selectedSkins);m_state["selectedTracks"]=indexes(m_tracks);
    m_state["files"]=fileRows(live);m_state["favoritesOnly"]=m_favoritesOnly;
    m_state["favoriteFolders"]=favoriteFolderRows();m_state["favoriteFolder"]=m_favoriteFolder;
    QVariantList layers;for(int i=0;i<m_layers.size();++i)layers.append(QVariantMap{{"name",QFileInfo(m_layers[i]).completeBaseName()},{"visible",r->SkeletonLayerVisible(size_t(i))},{"selected",m_selectedBackground<0&&r->ActiveSkeletonIndex()==size_t(i)}});
    m_state["loadedSpines"]=layers;m_state["showLoadedSpines"]=m_showLayers;
    QVariantList slotRows;for(int i=0;i<int(r->SlotCatalog().size());++i)slotRows.append(QVariantMap{{"name",from(r->SlotCatalog()[i])},{"visible",!m_hiddenSlots.contains(i)}});
    m_state["slots"]=slotRows;m_state["slotQuery"]=m_slotQuery;m_state["slotHoverEnabled"]=m_hoverEnabled;m_state["hoveredSlot"]=m_hoveredSlot;m_state["pinnedSlot"]=m_pinnedSlot;
    const auto pinnedBounds=runtime()->MeasureSlotBounds(utf8(m_pinnedSlot));
    m_state["slotBounds"]=m_pinnedSlot.isEmpty()||pinnedBounds.z==0?QVariantMap{}:QVariantMap{{"x",pinnedBounds.x},{"y",pinnedBounds.y},{"width",pinnedBounds.z},{"height",pinnedBounds.w}};
    QVariantList queue;for(const auto& s:m_queue)queue.append(QVariantMap{{"name",s},{"duration",r->MotionDuration(utf8(s).c_str())}});
    m_state["queue"]=queue;m_state["queuePlaying"]=m_queuePlaying;m_state["queueIndex"]=m_queueIndex;
    {
        const int characters=live2dMode()?(live.value("loaded").toBool()?1:0):int(m_layers.size());
        if(characters>0&&m_lastCharacterCount==0)m_selectedBackground=-1;
        if(characters==0&&m_selectedBackground<0&&!m_backgrounds.isEmpty())m_selectedBackground=0;
        m_lastCharacterCount=characters;
    }
    m_state["renderBackground"]=(m_petMode?QColor(Qt::transparent):m_clearColor).name(QColor::HexArgb);m_state["hasBackgroundImage"]=hasBackground();{QVariantList rows;for(int i=0;i<m_backgrounds.size();++i)rows.append(QVariantMap{{"name",m_backgrounds[i].name},{"visible",m_backgrounds[i].visible},{"selected",m_selectedBackground==i},{"scale",m_backgrounds[i].scale}});m_state["backgrounds"]=rows;}
    {
        const bool live2dLayer=live2dMode();
        const int count=live2dLayer?(live.value("loaded").toBool()?1:0):int(m_layers.size());QVariantList stack;
        for(int level=count;level>=0;--level){
            for(int i=0;i<m_backgrounds.size();++i)if(std::min(m_backgrounds[i].level,count)==level)
                stack.append(QVariantMap{{"kind","background"},{"index",i},{"name",m_backgrounds[i].name},{"visible",m_backgrounds[i].visible},{"selected",m_selectedBackground==i}});
            if(level>0&&live2dLayer)stack.append(QVariantMap{{"kind","live2d"},{"index",0},{"name",modelName(m_currentPath)},{"visible",!m_live2dHidden},{"selected",m_selectedBackground<0}});
            else if(level>0){const int s=count-level;stack.append(QVariantMap{{"kind","spine"},{"index",s},{"name",QFileInfo(m_layers[s]).completeBaseName()},{"visible",r->SkeletonLayerVisible(size_t(s))},{"selected",m_selectedBackground<0&&r->ActiveSkeletonIndex()==size_t(s)}});}
        }
        m_state["layerStack"]=stack;
        if(stack.size()>1)m_showLayers=true;
        if(stack.isEmpty())m_showLayers=false;
        m_state["showLoadedSpines"]=m_showLayers;
    }m_state["fullscreen"]=m_window&&m_window->visibility()==QWindow::FullScreen;
    m_state["petMode"]=m_petMode;m_state["petRandom"]=m_petRandom;m_state["resizeEnabled"]=m_resizeEnabled;m_state["wheelInverted"]=m_invertWheel;m_state["clickThrough"]=m_clickThrough;m_state["resizeBorderPhysical"]=8;
    m_state["petDragging"]=m_petMode&&m_pointerMode==5;
    m_state["windowWidth"]=m_window?qRound(m_window->width()*m_window->devicePixelRatio()):m_viewport.width();
    m_state["windowHeight"]=m_window?qRound(m_window->height()*m_window->devicePixelRatio()):m_viewport.height();
    if(r->ContainsDrawableContent()){m_cachedWasMix=m_skinMix;if(m_skinMix){m_cachedMixSkins.clear();for(int i=0;i<int(r->LookNames().size());++i)if(m_selectedSkins.contains(i))m_cachedMixSkins.append(from(r->LookNames()[i]));}else m_cachedSkin=from(r->ActiveLookName());}
    QVariantMap capabilities;
    const QStringList global={"file.open","file.folder","file.play","file.favorite","file.reveal","file.addSpine","file.favoritesView","favorites.folderSelect","favorites.folderCreate","favorites.folderRename","favorites.folderDelete","favorites.move","favorites.copy","favorites.remove","favorites.unfavorite","background.open","background.clear","background.select","background.visible","background.remove","background.move","background.reset","background.color","settings.language","settings.resolution","theme.hue","theme.saturation","theme.brightness","theme.fontSize","theme.dark","theme.reset","stage.decor","stage.checker","stage.decorStyle","view.hideInfoCard","view.hideExportButton","window.move","window.minimize","window.maximize","window.close","window.fullscreen","spine.pma","spine.resetOnLoad"};
    for(const auto& c:global)capabilities[c]=true;
    capabilities["settings.resolution.custom"]=!m_petMode;
    capabilities["settings.renderSize"]=!m_petMode;
    capabilities["settings.renderSize.reset"]=!m_petMode;
    const QStringList loaded={"view.scale","view.reset","playback.speed","playback.mix","animation.play","skin.mixMode","skin.select","skin.toggle","spine.mirror","spine.rotate","track.toggle","track.apply","track.clear","slot.toggle","slot.setVisible","slot.only","slot.clear","slot.excludeQuery","slot.hoverEnabled","slot.hoverRow","slot.pickColor","slot.bounds","queue.add","queue.remove","queue.move","queue.play","queue.stop","queue.clear","layer.select","layer.up","layer.down","layer.move","layer.visible","layer.stackMove","layer.remove"};
    for(const auto& c:loaded)capabilities[c]=r->ContainsDrawableContent();
    for(const auto& c:QStringList{"view.scale","view.reset","playback.speed","playback.mix"})capabilities[c]=true;
    capabilities["mode.toggle"]=true;
    capabilities["background.setColor"]=true;capabilities["background.resetColor"]=true;capabilities["slot.setColor"]=true;
    for(const auto& command:QStringList{"window.resize","window.toggleResize","window.toggleChrome","window.invertWheel","window.matchCanvas","window.restoreCanvas","window.showControls"})capabilities[command]=true;
#if defined(Q_OS_WIN)
    capabilities["window.toggleClickThrough"]=true;
#endif
    if(live2dMode()){
        for(auto it=live.cbegin();it!=live.cend();++it)if(it.key()!="capabilities"&&it.key()!="mode")m_state[it.key()]=it.value();
        for(const auto& c:loaded)capabilities[c]=false;
        const auto liveCaps=live.value("capabilities").toMap();for(auto it=liveCaps.cbegin();it!=liveCaps.cend();++it)capabilities[it.key()]=it.value();
        capabilities["file.addSpine"]=false;
        const auto error=live.value("error").toString();
        if(!error.isEmpty()&&error!=m_lastLiveError){m_lastLiveError=error;const QByteArray raw=error.toUtf8();const QString shown=QCoreApplication::translate("Live2D",raw.constData());QTimer::singleShot(0,this,[this,shown]{fail(shown);});}
    }
    const bool drawable=m_state.value("loaded").toBool();
    for(const auto& c:QStringList{"layer.select","layer.stackMove","layer.remove"})capabilities[c]=true;
    if(live2dMode())capabilities["layer.visible"]=drawable;
    capabilities["pet.enter"]=drawable&&!m_petMode;capabilities["pet.exit"]=m_petMode;capabilities["pet.next"]=m_petMode;capabilities["pet.random"]=m_petMode;
    for(const auto& command:QStringList{"export.alpha","export.queue","export.imageFps","export.videoFps","export.gifFps","export.png","export.jpg"})capabilities[command]=drawable&&!m_exportActive;
    for(const auto& command:QStringList{"export.pngFrames","export.jpgFrames","export.mp4","export.webm","export.gif"})capabilities[command]=drawable&&!m_exportActive;
    capabilities["plugins"]=!m_exportActive&&!m_petMode&&m_plugins->hasModules();
    if(m_plugins->isOpen()){
        for(auto it=capabilities.begin();it!=capabilities.end();++it)it.value()=false;
        for(const auto& command:QStringList{"window.move","window.close","window.minimize","window.maximize","window.fullscreen","window.resize"})capabilities[command]=true;
    }
    if(m_exportActive){
        for(auto it=capabilities.begin();it!=capabilities.end();++it)it.value()=false;
        for(const auto& command:QStringList{"window.move","window.close","window.minimize","export.cancel"})capabilities[command]=true;
    }
    m_state["capabilities"]=capabilities;
    syncPluginSettings();
    if(notify)publishState();
}
void ViewerController::publishState(){
    m_viewportNotificationPending=false;
    QVariantMap scalars=m_state;
    for(const auto& key:internalKeys())scalars.remove(key);
    for(const auto& key:listKeys()){
        const auto it=scalars.constFind(key);if(it==scalars.cend())continue;
        if(m_lists->value(key)!=it.value())m_lists->insert(key,it.value());
        scalars.erase(it);
    }
    if(scalars==m_publishedState)return;
    m_publishedState=std::move(scalars);emit stateChanged();
}
QVariantList ViewerController::fileRows(const QVariantMap& live){
    const bool liveMode=live2dMode();
    const auto livePath=QDir::fromNativeSeparators(live.value("currentModelPath").toString());
    const bool liveLoaded=live.value("loaded").toBool();
    auto& c=m_fileRows;
    if(c.valid&&c.live==liveMode&&c.favoritesOnly==m_favoritesOnly&&c.current==m_currentPath&&c.files==m_files
        &&c.favorites==m_favorites&&c.favoriteRevision==m_favoriteRevision&&c.favoriteFolder==m_favoriteFolder&&c.layers==m_layers&&(!liveMode||(c.livePath==livePath&&c.liveLoaded==liveLoaded)))return c.rows;
    QVariantList rows;
    const auto current=ArchiveCache::displayPath(m_currentPath);QSet<QString> layers;for(const auto& l:m_layers)layers.insert(ArchiveCache::displayPath(l));
    for(const auto& p:visibleFiles()){const auto shown=ArchiveCache::displayPath(p);rows.append(QVariantMap{{"path",p},{"name",modelName(p)},{"parent",QFileInfo(shown).absolutePath()},{"favorite",m_favorites.contains(shown)},{"folders",[&]{QStringList ids;for(const auto& f:m_favoriteFolders)if(f.items.contains(shown))ids.append(f.id);return ids;}()},{"current",shown==current},{"missing",m_favoritesOnly&&m_missingFavorites.contains(shown)},{"loaded",liveMode?(liveLoaded&&p==livePath):layers.contains(shown)}});}
    c.files=m_files;c.favorites=m_favorites;c.favoriteRevision=m_favoriteRevision;c.favoriteFolder=m_favoriteFolder;c.layers=m_layers;c.current=m_currentPath;c.livePath=livePath;
    c.live=liveMode;c.liveLoaded=liveLoaded;c.favoritesOnly=m_favoritesOnly;c.rows=rows;c.valid=true;
    return rows;
}
void ViewerController::dispatch(const QString& c,const QVariant& v){
    if(c=="settings.modal"){
        const auto panel=v.toMap();const QString source=panel.isEmpty()?QStringLiteral("settings"):panel.value("source","settings").toString();
        const bool open=panel.isEmpty()?v.toBool():panel.value("open").toBool();
        if(open)m_modalPanels.insert(source);else m_modalPanels.remove(source);
        syncPluginSuspension();m_clock.restart();return;
    }
    if(c=="export.cancel"){m_exportService.cancel();return;}
    if(m_exportActive&&c!="window.close"&&c!="window.minimize"&&c!="window.move")return;
    if(c=="plugins"||c.startsWith("plugins.")){
        if(m_petMode)return;
        if(c.startsWith("plugins.module.")){
            if(auto* module=qobject_cast<PluginModule*>(m_plugins->module()))module->dispatch(c.mid(15),v);
        }else m_plugins->dispatch(c=="plugins"?QStringLiteral("open"):c.mid(8),v);
        return;
    }
    if(m_plugins->isOpen()&&c!="window.move"&&c!="window.close"&&c!="window.minimize"&&c!="window.maximize"&&c!="window.fullscreen"&&c!="window.resize")return;
    if(windowCommand(c,v))return;
    auto* r=runtime();const int index=v.toInt();const float f=v.toFloat();
    if(c=="mode.toggle"){
        setMode(!live2dMode());
        m_clock.restart();refresh();record();return;
    }
    if(live2dMode()&&(c.startsWith("live2d.")||c.startsWith("queue.")||c=="view.scale"||c=="view.reset"||c=="playback.speed"||c=="animation.play")){
        m_live2d->command(c,v);
        if(c!="live2d.gaze"&&c!="live2d.gazePose"&&c!="live2d.parameterValue"&&c!="live2d.partValue"&&c!="live2d.drag")record();
        return;
    }
    if(c=="file.open"){
        m_modal=true;auto files=QFileDialog::getOpenFileNames(nullptr,live2dMode()?tr("Import Model"):tr("Open Spine"),m_folder,live2dMode()?tr("Live2D models (*.model3.json *.zip)"):tr("Spine files (*.json *.skel *.skel.bytes *.json.txt *.skel.txt *.zip);;All files (*)"));m_modal=false;m_clock.restart();std::sort(files.begin(),files.end());if(!files.isEmpty())openPaths(files);return;
    }
    if(c=="file.folder"){m_modal=true;auto p=QFileDialog::getExistingDirectory(nullptr,tr("Select Folder"),m_folder);m_modal=false;m_clock.restart();if(!p.isEmpty())scanFolder(p,false,true);return;}
    if(c=="file.play"){
        const auto p=v.toString();
        if(m_missingFavorites.contains(ArchiveCache::displayPath(p))&&checkFavoriteFiles())refresh();
        if(!ArchiveCache::exists(p)){fail(tr("This file cannot be found. The drive it is on may not be connected.")+"\n\n"+QDir::toNativeSeparators(ArchiveCache::displayPath(p)));return;}
        openPaths({p});return;
    }
    if(c=="file.addSpine"){
        if(v.toString().isEmpty()){m_modal=true;auto files=QFileDialog::getOpenFileNames(nullptr,tr("Add Spine"),m_folder,tr("Spine files (*.json *.skel *.skel.bytes *.json.txt *.skel.txt *.zip);;All files (*)"));m_modal=false;m_clock.restart();for(const auto& f:files)addLayer(f);return;}
        addLayer(v.toString());return;}
    if(c=="file.favorite"){
        const auto p=ArchiveCache::displayPath(v.toString());auto* shown=m_favoritesOnly?favoriteFolder(m_favoriteFolder):nullptr;
        if(shown&&shown->items.contains(p))shown->items.removeAll(p);
        else if(m_favorites.contains(p))for(auto& f:m_favoriteFolders)f.items.removeAll(p);
        else if(!p.isEmpty())m_favoriteFolders.front().items.append(p);
        syncFavorites();
    }
    else if(c=="favorites.folderSelect"){if(favoriteFolder(v.toString())){m_favoriteFolder=v.toString();m_favoritesOnly=true;}}
    else if(c=="favorites.folderCreate"){
        const auto m=v.typeId()==QMetaType::QVariantMap?v.toMap():QVariantMap{{"name",v}};
        const auto name=m.value("name").toString().trimmed().left(40);if(name.isEmpty())return;
        FavoriteFolder folder{QUuid::createUuid().toString(QUuid::WithoutBraces),name,{}};
        const auto paths=favoritePaths(m);
        if(!paths.isEmpty()){
            folder.items=paths;
            if(m.value("action").toString()==QStringLiteral("move"))if(auto* from=favoriteFolder(m.value("from").toString()))for(const auto& p:paths)from->items.removeAll(p);
        }
        else{m_favoriteFolder=folder.id;m_favoritesOnly=true;}
        m_favoriteFolders.append(folder);syncFavorites();
    }
    else if(c=="favorites.folderRename"){
        const auto m=v.toMap();const auto name=m.value("name").toString().trimmed().left(40);auto* folder=favoriteFolder(m.value("id").toString());
        if(folder&&folder->id!=QStringLiteral("default")&&!name.isEmpty()){folder->name=name;syncFavorites();}
    }
    else if(c=="favorites.folderDelete"){
        const auto id=v.toString();
        const auto found=std::find_if(m_favoriteFolders.begin(),m_favoriteFolders.end(),[&](const FavoriteFolder& f){return f.id==id;});
        if(found!=m_favoriteFolders.end()&&id!=QStringLiteral("default")){
            const auto items=found->items;m_favoriteFolders.erase(found);
            for(const auto& p:items)if(std::none_of(m_favoriteFolders.begin(),m_favoriteFolders.end(),[&](const FavoriteFolder& f){return f.items.contains(p);}))m_favoriteFolders.front().items.append(p);
            syncFavorites();
        }
    }
    else if(c=="favorites.move"||c=="favorites.copy"){
        const auto m=v.toMap();const auto paths=favoritePaths(m);auto* to=favoriteFolder(m.value("to").toString());
        if(to&&!paths.isEmpty()){
            auto* from=c=="favorites.move"?favoriteFolder(m.value("from").toString()):nullptr;
            for(const auto& p:paths){
                if(from&&from!=to)from->items.removeAll(p);
                if(!to->items.contains(p))to->items.append(p);
            }
            syncFavorites();
        }
    }
    else if(c=="favorites.remove"){
        const auto m=v.toMap();const auto paths=favoritePaths(m);
        if(auto* folder=favoriteFolder(m.value("folder").toString());folder&&!paths.isEmpty()){for(const auto& p:paths)folder->items.removeAll(p);syncFavorites();}
    }
    else if(c=="favorites.unfavorite"){
        const auto paths=favoritePaths(v.toMap());
        if(!paths.isEmpty()){for(auto& f:m_favoriteFolders)for(const auto& p:paths)f.items.removeAll(p);syncFavorites();}
    }
    else if(c=="file.reveal"){const auto archive=ArchiveCache::archiveOf(v.toString());QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(archive.isEmpty()?v.toString():archive).absolutePath()));}
    else if(c=="file.favoritesView"){m_favoritesOnly=v.toBool();if(m_favoritesOnly)checkFavoriteFiles();}
    else if(c=="replace.confirm"){auto pending=std::exchange(m_pendingPaths,{});m_state["replaceConfirmation"]=QVariantMap{{"open",false}};openPaths(pending,true);return;}
    else if(c=="replace.cancel"){m_pendingPaths.clear();m_state["replaceConfirmation"]=QVariantMap{{"open",false}};}
    else if(c=="view.scale")r->SetSkeletonScale(std::clamp(f,.1f,5.f));
    else if(c=="view.reset")r->SetSkeletonScale(1);
    else if(c=="playback.speed")r->SetTimeScale(std::clamp(f,0.f,5.f));
    else if(c=="playback.mix"){m_mix=std::clamp(f,0.f,1.f);r->SetBlendWindowSeconds(m_mix);}
    else if(c=="spine.pma"){m_pma=v.toBool();r->SetPremultipliedAlpha(m_pma);}
    else if(c=="spine.resetOnLoad"){m_resetOnLoad=v.toBool();r->SetResetViewOnLoad(m_resetOnLoad);}
    else if(c=="animation.play"){if(index>=0&&index<int(r->MotionNames().size())){m_queuePlaying=false;m_queueIndex=0;r->PlayMotionByIndex(size_t(index));}}
    else if(c=="spine.mirror")r->ToggleMirrorX();
    else if(c=="spine.rotate")r->RotateClockwise();
    else if(c=="skin.mixMode"){
        m_skinMix=v.toBool();
        if(m_skinMix){const auto& names=r->LookNames();auto active=std::find(names.begin(),names.end(),r->ActiveLookName());m_selectedSkins.clear();m_lastMixedSkin=active!=names.end()?int(active-names.begin()):(names.empty()?-1:0);if(m_lastMixedSkin>=0)m_selectedSkins.insert(m_lastMixedSkin);}
        else {if(!m_selectedSkins.contains(m_lastMixedSkin)){m_lastMixedSkin=-1;for(int i:m_selectedSkins)m_lastMixedSkin=qMax(m_lastMixedSkin,i);}const int selected=m_lastMixedSkin>=0?m_lastMixedSkin:(r->LookNames().empty()?-1:0);m_selectedSkins.clear();if(selected>=0){m_selectedSkins.insert(selected);r->ApplyLookByIndex(size_t(selected));}m_lastMixedSkin=-1;}
    }
    else if(c=="skin.select"||c=="skin.toggle"){
        if(index>=0&&index<int(r->LookNames().size())){
            if(c=="skin.select"){m_selectedSkins={index};r->ApplyLookByIndex(size_t(index));}
            else {if(m_selectedSkins.contains(index)){m_selectedSkins.remove(index);if(m_lastMixedSkin==index){m_lastMixedSkin=-1;for(int i:m_selectedSkins)m_lastMixedSkin=qMax(m_lastMixedSkin,i);}}else{m_selectedSkins.insert(index);m_lastMixedSkin=index;}
                if(m_selectedSkins.isEmpty()){m_selectedSkins.insert(0);m_lastMixedSkin=0;}
                SlNameList selected;for(int i=0;i<int(r->LookNames().size());++i)if(m_selectedSkins.contains(i))selected.push_back(r->LookNames()[i]);if(selected.size()==1)r->ApplyLookByName(selected.front().c_str());else r->ComposeLooks(selected);}
        }
    }
    else if(c=="track.toggle"){if(m_tracks.contains(index))m_tracks.remove(index);else m_tracks.insert(index);}
    else if(c=="track.apply"){SlNameList names;for(int i=0;i<int(r->MotionNames().size());++i)if(m_tracks.contains(i))names.push_back(r->MotionNames()[i]);r->SetLayeredMotions(names,true);}
    else if(c=="track.clear"){m_tracks.clear();r->SetLayeredMotions({});}
    else if(c=="slot.toggle"){if(m_hiddenSlots.contains(index))m_hiddenSlots.remove(index);else m_hiddenSlots.insert(index);m_slotQuery.clear();updateSlotFilter();}
    else if(c=="slot.setVisible"){const auto m=v.toMap();const bool visible=m.value("visible").toBool();for(const auto& i:m.value("indices").toList()){const int n=i.toInt();if(visible)m_hiddenSlots.remove(n);else m_hiddenSlots.insert(n);}m_slotQuery.clear();updateSlotFilter();}
    else if(c=="slot.only"){QSet<int> keep;for(const auto& i:v.toList())keep.insert(i.toInt());m_hiddenSlots.clear();const int total=int(runtime()->SlotCatalog().size());for(int i=0;i<total;++i)if(!keep.contains(i))m_hiddenSlots.insert(i);m_slotQuery.clear();updateSlotFilter();}
    else if(c=="slot.clear"){m_hiddenSlots.clear();m_slotQuery.clear();updateSlotFilter();}
    else if(c=="slot.excludeQuery"){m_slotQuery=v.toString();m_hiddenSlots.clear();updateSlotFilter();}
    else if(c=="slot.hoverEnabled"){m_hoverEnabled=v.toBool();if(!m_hoverEnabled)m_hoveredSlot.clear();}
    else if(c=="slot.hoverRow")m_listHoveredSlot=v.toString();
    else if(c=="slot.bounds")m_state["slotBoundsVisible"]=v.toBool();
    else if(c=="slot.pickColor"){m_modal=true;auto color=QColorDialog::getColor(QColor(m_state.value("slotHoverColor").toString()),nullptr);m_modal=false;m_clock.restart();if(color.isValid())m_state["slotHoverColor"]=color.name();}
    else if(c=="slot.setColor"){const QColor color(v.toString());if(color.isValid())m_state["slotHoverColor"]=color.name();}
    else if(c=="queue.add"){if(!m_queuePlaying&&index>=0&&index<int(r->MotionNames().size()))m_queue.append(from(r->MotionNames()[index]));}
    else if(c=="queue.remove"){if(!m_queuePlaying&&index>=0&&index<m_queue.size())m_queue.removeAt(index);}
    else if(c=="queue.move"){const auto m=v.toMap();const int from=m.value("from",-1).toInt(),to=m.value("to",-1).toInt();if(!m_queuePlaying&&from>=0&&from<m_queue.size()&&to>=0&&to<m_queue.size()&&from!=to)m_queue.move(from,to);}
    else if(c=="queue.clear"){m_queuePlaying=false;m_queue.clear();m_queueIndex=0;}
    else if(c=="queue.play"){if(!m_queue.isEmpty()){m_queuePlaying=true;m_queueIndex=0;playQueueItem();}}
    else if(c=="queue.stop"){m_queuePlaying=false;m_queueIndex=0;}
    else if(c=="layer.select"){if(live2dMode())m_selectedBackground=-1;else selectLayer(index);}
    else if(c=="layer.visible"&&live2dMode())m_live2dHidden=!m_live2dHidden;
    else if(c=="layer.visible")r->SetSkeletonLayerVisible(size_t(index),!r->SkeletonLayerVisible(size_t(index)));
    else if(c=="layer.up"||c=="layer.down"){
        const int target=index+(c=="layer.up"?-1:1);if(index>=0&&target>=0&&index<m_layers.size()&&target<m_layers.size()){
            bool ok=c=="layer.up"?r->PromoteSkeleton(size_t(index)):r->DemoteSkeleton(size_t(index));if(ok)m_layers.swapItemsAt(index,target);}
    }
    else if(c=="layer.move"){
        const auto m=v.toMap();int from=m.value("from").toInt();const int to=m.value("to").toInt();
        if(from>=0&&to>=0&&from<m_layers.size()&&to<m_layers.size()){
            while(from>to&&r->PromoteSkeleton(size_t(from))){m_layers.swapItemsAt(from,from-1);--from;}
            while(from<to&&r->DemoteSkeleton(size_t(from))){m_layers.swapItemsAt(from,from+1);++from;}
        }
    }
    else if(c=="layer.remove"&&live2dMode()){
        if(index==0&&m_state.value("loaded").toBool()){
            m_live2d->command("live2d.clear");m_live2dPath.clear();m_currentPath.clear();
            for(auto& b:m_backgrounds)if(b.level>0)--b.level;
        }
    }
    else if(c=="layer.remove"){
        const int count=int(m_layers.size());
        if(index>=0&&index<count){
            const bool activeRemoved=int(r->ActiveSkeletonIndex())==index;
            if(!activeRemoved)saveLayerControls();
            const QString removed=m_layers[index];const int position=count-1-index;
            if(m_hub.RemoveCurrentRuntimeSkeleton(size_t(index))){
                m_layerControls.remove(removed);m_layers.removeAt(index);
                for(auto& b:m_backgrounds)if(b.level>position)--b.level;
                if(activeRemoved){m_queue.clear();m_queuePlaying=false;m_queueIndex=0;}
                if(m_layers.isEmpty()){
                    m_currentPath.clear();m_selectedSkins.clear();m_tracks.clear();m_hiddenSlots.clear();
                    m_pinnedSlot.clear();m_hoveredSlot.clear();m_listHoveredSlot.clear();
                }
                else{m_currentPath=m_layers[int(r->ActiveSkeletonIndex())];restoreLayerControls();}
                updateSlotFilter();
            }
        }
    }
    else if(c=="layer.stackMove"){
        const auto m=v.toMap();const int from=m.value("from").toInt(),to=m.value("to").toInt();
        const int count=live2dMode()?(m_state.value("loaded").toBool()?1:0):int(m_layers.size());
        struct Entry{bool background;int index;};QList<Entry> stack;
        for(int level=count;level>=0;--level){
            for(int i=0;i<m_backgrounds.size();++i)if(std::min(m_backgrounds[i].level,count)==level)stack.append({true,i});
            if(level>0)stack.append({false,count-level});
        }
        if(from>=0&&to>=0&&from<stack.size()&&to<stack.size()&&from!=to){
            stack.move(from,to);
            QList<int> current;for(int i=0;i<count;++i)current.append(i);
            int target=0;
            for(const auto& e:stack){
                if(live2dMode())break;
                if(e.background)continue;
                int at=int(current.indexOf(e.index));
                while(at>target&&r->PromoteSkeleton(size_t(at))){m_layers.swapItemsAt(at,at-1);current.swapItemsAt(at,at-1);--at;}
                ++target;
            }
            const SlTextureId selected=m_selectedBackground>=0&&m_selectedBackground<m_backgrounds.size()?m_backgrounds[m_selectedBackground].texture:0;
            QList<BackgroundLayer> ordered;int below=count;
            for(const auto& e:stack){if(e.background){auto b=m_backgrounds[e.index];b.level=below;ordered.append(b);}else --below;}
            m_backgrounds=ordered;m_selectedBackground=-1;
            for(int i=0;selected&&i<m_backgrounds.size();++i)if(m_backgrounds[i].texture==selected)m_selectedBackground=i;
        }
    }
    else if(c=="background.open"){
        QStringList paths;if(!v.toString().isEmpty())paths.append(v.toString());
        else{m_modal=true;paths=QFileDialog::getOpenFileNames(nullptr,tr("Background"),{},tr("Images (*.png *.jpg *.jpeg *.webp *.bmp)"));m_modal=false;m_clock.restart();}
        for(const auto& p:paths){
            const auto next=m_recorder.LoadTextureUtf8(p.toUtf8().constData(),false);
            if(!next){fail(tr("Could not load background image: %1").arg(p));return;}
            BackgroundLayer layer;layer.texture=next;layer.name=QFileInfo(p).completeBaseName();
            m_backgrounds.prepend(layer);if(m_lastCharacterCount==0)m_selectedBackground=0;else if(m_selectedBackground>=0)++m_selectedBackground;m_showLayers=true;
        }
    }
    else if(c=="background.clear")clearBackgrounds();
    else if(c=="background.select"){if(index>=0&&index<m_backgrounds.size())m_selectedBackground=index;}
    else if(c=="background.visible"){if(index>=0&&index<m_backgrounds.size())m_backgrounds[index].visible=!m_backgrounds[index].visible;}
    else if(c=="background.reset"){if(index>=0&&index<m_backgrounds.size()){m_backgrounds[index].offset={0,0};m_backgrounds[index].scale=1;}}
    else if(c=="background.remove"){
        if(index>=0&&index<m_backgrounds.size()){
            m_recorder.ReleaseTexture(m_backgrounds[index].texture);m_backgrounds.removeAt(index);
            if(m_selectedBackground==index)m_selectedBackground=-1;else if(m_selectedBackground>index)--m_selectedBackground;
        }
    }
    else if(c=="background.move"){
        const auto m=v.toMap();const int from=m.value("from").toInt(),to=m.value("to").toInt();
        if(from>=0&&to>=0&&from<m_backgrounds.size()&&to<m_backgrounds.size()&&from!=to){
            const bool wasSelected=m_selectedBackground==from;m_backgrounds.move(from,to);
            if(wasSelected)m_selectedBackground=to;
            else if(m_selectedBackground>=0){if(from<m_selectedBackground&&to>=m_selectedBackground)--m_selectedBackground;else if(from>m_selectedBackground&&to<=m_selectedBackground)++m_selectedBackground;}
        }
    }
    else if(c=="background.color"){m_modal=true;auto color=QColorDialog::getColor(m_clearColor);m_modal=false;if(color.isValid())m_clearColor=color;}
    else if(c=="background.setColor"){const QColor color(v.toString());if(color.isValid())m_clearColor=color;}
    else if(c=="background.resetColor")m_clearColor=Qt::black;
    else if(c=="stage.decor"){m_state["stageDecor"]=v.toBool();m_settings.setValue("stageDecor",v.toBool());m_state["stageChecker"]=false;m_settings.setValue("stageChecker",false);}
    else if(c=="stage.decorStyle"){m_state["stageDecorStyle"]=std::clamp(index,0,1);m_settings.setValue("stageDecorStyle",std::clamp(index,0,1));}
    else if(c=="stage.checker"){m_state["stageChecker"]=v.toBool();m_settings.setValue("stageChecker",v.toBool());if(v.toBool()){m_state["stageDecor"]=false;m_settings.setValue("stageDecor",false);}}
    else if(c=="settings.language"){const auto language=supportedLanguage(v.toString());if(language.isEmpty())return;m_state["language"]=language;savePreferences();emit languageRequested(language);}
    else if(c=="settings.resolution"){
        const auto* preset=window_resolution_presets::Get(index);if(preset&&m_window){m_state["resolutionPreset"]=index;if(preset->width>0)m_window->resize(qRound(preset->width/m_dpr),qRound(preset->height/m_dpr));}
    }
    else if(c=="theme.hue")m_state["themeHue"]=v;
    else if(c=="theme.saturation")m_state["themeSaturation"]=v;
    else if(c=="theme.brightness")m_state["themeBrightness"]=v;
    else if(c=="theme.dark")m_state["darkTheme"]=v;
    else if(c=="view.hideInfoCard"){m_state["infoCardHidden"]=v.toBool();m_settings.setValue("infoCardHidden",v.toBool());}
    else if(c=="view.hideExportButton"){m_state["exportButtonHidden"]=v.toBool();m_settings.setValue("exportButtonHidden",v.toBool());}
    else if(c=="theme.fontSize")m_state["baseFontPixels"]=std::clamp(f,10.f,50.f);
    else if(c=="theme.reset"){m_state["themeHue"]=.74;m_state["themeSaturation"]=.83;m_state["themeBrightness"]=1.;m_state["darkTheme"]=false;m_state["baseFontPixels"]=16.0*m_state.value("uiScale",1.0).toDouble();}
    else if(c.startsWith("window.")&&m_window){
        if(c=="window.close")m_window->close();else if(c=="window.minimize")m_window->showMinimized();
        else if(c=="window.move")m_window->startSystemMove();
        else if(c=="window.maximize"){if(m_window->visibility()==QWindow::Maximized)m_window->showNormal();else m_window->showMaximized();}
        else if(c=="window.fullscreen"){if(m_window->visibility()==QWindow::FullScreen)m_window->showNormal();else m_window->showFullScreen();}
    }
    else if(c.startsWith("export.")){
        if(c=="export.alpha")m_state["exportAlpha"]=v.toBool();else if(c=="export.queue")m_state["exportQueue"]=v.toBool();
        else if(c=="export.imageFps")m_state["exportImageFps"]=std::clamp(index,1,120);else if(c=="export.videoFps")m_state["exportVideoFps"]=std::clamp(index,1,120);
        else if(c=="export.gifFps"){if(ExportService::gifFpsChoices().contains(index))m_state["exportGifFps"]=index;}
        else {beginExport(c,v);return;}
    }
    if(c.startsWith("theme."))m_state["themeCustomized"]=true;
    refresh();record();
}
bool ViewerController::petHitTest(qreal x,qreal y) const{
    if(!m_petMode)return true;
    const QPointF point=QPointF(x,y)*m_dpr;
    if(live2dMode()){
        const auto bounds=m_live2d->state().value("renderBounds").toMap();
        return QRectF(bounds.value("x").toDouble(),bounds.value("y").toDouble(),
            bounds.value("width").toDouble(),bounds.value("height").toDouble()).contains(point);
    }
    return m_snapshot&&sceneHitTest(*m_snapshot,point);
}
QPointF ViewerController::renderPosition(QPointF point)const{
    if(renderSize().isEmpty()||m_previewSize.isEmpty())return point*m_dpr;
    return {point.x()*m_viewport.width()/m_previewSize.width(),point.y()*m_viewport.height()/m_previewSize.height()};
}
void ViewerController::pointerPress(QPointF p,Qt::MouseButton button,Qt::KeyboardModifiers mods){
    if(inputBlocked())return;
    if(m_petMode&&!petHitTest(p.x(),p.y())){m_pointerMode=0;m_dragged=true;return;}
    if(!m_petMode&&!live2dMode()&&m_hoverEnabled&&button==Qt::LeftButton){hover(p);m_pinnedSlot=m_hoveredSlot;refresh();}
    p=renderPosition(p);m_pointerStart=p;m_pointerLast=p;m_dragged=false;m_pointerMode=0;
    if(m_petMode&&button==Qt::LeftButton&&m_window){
        m_pointerMode=5;m_petDragCursor=QCursor::pos();m_petDragWindow=m_window->position();
        m_state["petDragging"]=true;publishState();return;
    }
    if(live2dMode()&&button==Qt::LeftButton&&m_state.value("loaded").toBool()&&!activeBackground()&&!m_live2dHidden)m_pointerMode=4;
}
void ViewerController::pointerMove(QPointF p,Qt::MouseButtons buttons,Qt::KeyboardModifiers mods){
    if(m_petMode){
        if(inputBlocked()||m_pointerMode!=5||!(buttons&Qt::LeftButton)||!m_window)return;
        const QPoint delta=QCursor::pos()-m_petDragCursor;
        if(!delta.isNull())m_dragged=true;
        if(m_dragged){
            QRect desktop;for(auto* screen:QGuiApplication::screens())desktop=desktop.united(screen->geometry());
            QPoint next=m_petDragWindow+delta;
            const QRect body=m_petCurrentRegion.isEmpty()?QRect(QPoint{},m_window->size()):m_petCurrentRegion.boundingRect();
            const int marginX=std::max(1,std::min(body.width(),qRound(80/m_dpr)));
            const int marginY=std::max(1,std::min(body.height(),qRound(80/m_dpr)));
            next.setX(std::clamp(next.x(),desktop.left()-body.right()-1+marginX,desktop.right()+1-body.left()-marginX));
            next.setY(std::clamp(next.y(),desktop.top()-body.bottom()-1+marginY,desktop.bottom()+1-body.top()-marginY));
            m_petMoveTarget=next;m_petMoveQueued=true;
            if(!m_petMoveTimer.isActive())m_petMoveTimer.start();
        }
        return;
    }
    if(inputBlocked()||!(buttons&Qt::LeftButton))return;p=renderPosition(p);const auto delta=p-m_pointerLast;
    BackgroundLayer* bg=activeBackground();if(!bg&&(mods&Qt::ControlModifier)&&m_backgrounds.size()==1)bg=&m_backgrounds.front();
    const int mode=(buttons&Qt::RightButton)?3:(bg?2:(live2dMode()?4:1));
    if(m_pointerMode!=mode){m_pointerMode=mode;m_pointerLast=p;return;}
    if(delta.manhattanLength()>0)m_dragged=true;
    if((buttons&Qt::RightButton)&&m_window)m_window->setPosition(m_window->position()+QPoint(qRound(delta.x()*m_previewSize.width()/m_viewport.width()),qRound(delta.y()*m_previewSize.height()/m_viewport.height())));
    else if(bg)bg->offset+=delta;
    else if(live2dMode())m_live2d->command("view.pan",QVariantMap{{"x",delta.x()},{"y",delta.y()}});
    else if(mods&Qt::ShiftModifier)runtime()->PanAllByPixels(qRound(delta.x()),qRound(delta.y()));
    else runtime()->PanByPixels(qRound(delta.x()),qRound(delta.y()));
    m_pointerLast=p;refresh();record();
}
void ViewerController::pointerRelease(QPointF p,Qt::MouseButton button,Qt::KeyboardModifiers mods){
    if(inputBlocked())return;
    if(m_petMode){
        m_petMoveTimer.stop();applyDesktopPetMove();
        if(button==Qt::LeftButton&&m_pointerMode==5&&!m_dragged){if(live2dMode()){p*=m_dpr;m_live2d->command("live2d.tap",QVariantMap{{"x",p.x()/m_viewport.width()*2-1},{"y",1-p.y()/m_viewport.height()*2}});}else runtime()->StepToNextMotion();}
        m_pointerMode=0;m_clock.restart();refresh();record();return;
    }
    if(live2dMode()){
        if(button==Qt::MiddleButton)m_live2d->command("view.reset");
        else if(button==Qt::LeftButton&&!m_dragged&&m_pointerMode==4){
            p=renderPosition(p);
            m_live2d->command("live2d.tap",QVariantMap{{"x",p.x()/m_viewport.width()*2-1},{"y",1-p.y()/m_viewport.height()*2}});
        }
        m_live2d->command("live2d.endDrag");m_pointerMode=0;record();return;
    }
    if(button==Qt::MiddleButton){if(mods&Qt::ShiftModifier)runtime()->ResetViewScaleAll();else{runtime()->ResetViewScale();if(!m_resetOnLoad)runtime()->CenterOpeningPoseInView();}fit();}
    if(button==Qt::LeftButton&&!m_dragged&&m_pointerMode==0&&mods==Qt::NoModifier&&!m_hoverEnabled)runtime()->StepToNextMotion();
    m_pointerMode=0;
    refresh();record();
}
void ViewerController::wheel(QPointF p,int delta,Qt::MouseButtons buttons,Qt::KeyboardModifiers mods){
    if(m_petMode&&!petHitTest(p.x(),p.y()))return;
    if(inputBlocked()||(!m_petMode&&(buttons&Qt::LeftButton)))return;p=renderPosition(p);
    BackgroundLayer* bg=activeBackground();if(!bg&&(mods&Qt::ControlModifier)&&m_backgrounds.size()==1)bg=&m_backgrounds.front();
    const int target=m_petMode?4:(bg?1:(live2dMode()?3:2));
    if(m_wheelTarget!=target){m_wheelRemainder=0;m_wheelTarget=target;}
    m_wheelRemainder+=delta;const int steps=m_wheelRemainder/120;m_wheelRemainder%=120;if(!steps)return;
    if(target==4){if(live2dMode())m_live2d->command("view.zoom",QVariantMap{{"steps",steps},{"inverted",m_invertWheel},{"centerOnly",true}});else runtime()->SetSkeletonScale(interaction::wheelScale(runtime()->SkeletonScale(),steps,m_invertWheel));}
    else if(target==1){float old=bg->scale;bg->scale=zoom(old,steps,steps>0,.05f,20);bg->offset=p+(bg->offset-p)*(bg->scale/old);}
    else if(target==3&&m_state.value("loaded").toBool()){
        const double shift=m_state.value("fullscreen").toBool()?0:42*m_state.value("titleScale",1).toDouble()*.5;
        QPointF center(m_viewport.width()*.5,m_viewport.height()*.5+shift);
        m_live2d->command("view.zoom",QVariantMap{{"steps",steps},{"inverted",m_invertWheel},{"x",p.x()},{"y",p.y()},{"originX",center.x()},{"originY",center.y()},{"centerOnly",false}});
    }
    else if(runtime()->ContainsDrawableContent()){
        auto* r=runtime();const int selected=int(r->ActiveSkeletonIndex());
        const float groupRatio=interaction::wheelGroupRatio(r->SkeletonScale(),steps,m_invertWheel);
        for(int i=0;i<int(r->LoadedSkeletonCount());++i){
            if(mods&Qt::ShiftModifier){if(!r->SkeletonLayerVisible(size_t(i)))continue;}else if(i!=selected)continue;
            float old=r->SkeletonScaleAt(size_t(i));float scale=(mods&Qt::ShiftModifier)?interaction::layerScaleWithRatio(old,groupRatio):interaction::wheelScale(old,steps,m_invertWheel);
            const auto o=r->ViewOffsetAt(size_t(i));QPointF center(m_viewport.width()*.5,m_viewport.height()*.5);
            auto anchor=center+QPointF(o.x,o.y);auto moved=p+(anchor-p)*(scale/old)-center;
            r->SetSkeletonScaleAt(size_t(i),scale);r->SetViewOffsetAt(size_t(i),float(moved.x()),float(moved.y()));
        }
    }
    refresh();record();
}
void ViewerController::hover(QPointF p){
    if(live2dMode()){
        if(inputBlocked())return;
        if(p.x()<0||p.y()<0)m_live2d->command("live2d.endDrag");
        else{p=renderPosition(p);m_live2d->command("live2d.drag",QVariantMap{{"x",p.x()/m_viewport.width()*2-1},{"y",1-p.y()/m_viewport.height()*2}});}
        return;
    }
    m_hoverPosition=p;
    if(!m_hoverEnabled||inputBlocked())return;p=renderPosition(p);
    const QString previous=std::exchange(m_hoveredSlot,QString{});
    const auto done=[&]{if(m_hoveredSlot!=previous)refresh();};
    if(p.x()<0||p.y()<0||p.x()>=m_viewport.width()||p.y()>=m_viewport.height()){done();return;}
    SlMatrix4 inverse{};if(!SlMatrixInverse(runtime()->ViewTransform(),inverse)){done();return;}
    const SlVec3 local=SlVec3Transform(SlVec3(float(p.x()),float(p.y()),0.f),inverse);
    const auto& names=runtime()->SlotCatalog();
    for(int i=int(names.size())-1;i>=0;--i){
        if(m_hiddenSlots.contains(i))continue;const auto& name=names[size_t(i)];ReadSlotMeshData mesh;
        if(runtime()->ReadSlotMesh(name,mesh)&&mesh.worldVertices.size()>=2){if(interaction::slotMeshContains(local.x,local.y,mesh)){m_hoveredSlot=from(name);break;}}
        else{const auto b=runtime()->MeasureSlotBounds(name);if(b.z!=0&&QRectF(b.x,b.y,b.z,b.w).normalized().contains(QPointF(local.x,local.y))){m_hoveredSlot=from(name);break;}}
    }
    done();
}
bool ViewerController::eventFilter(QObject* watched,QEvent* e){
    if(m_petMode&&(e->type()==QEvent::KeyPress||e->type()==QEvent::KeyRelease||e->type()==QEvent::ShortcutOverride||e->type()==QEvent::Shortcut)){
        e->accept();return true;
    }
    if(watched==m_window&&e->type()==QEvent::Close&&m_exportActive){
        static_cast<QCloseEvent*>(e)->ignore();
        if(!m_exportClosing){
            m_exportClosing=true;m_exportService.cancel();
            auto* closing=new QTimer(this);closing->setInterval(50);
            connect(closing,&QTimer::timeout,this,[this,closing]{
                if(m_exportActive||m_exportService.isBusy())return;
                closing->stop();closing->deleteLater();if(m_window)m_window->close();
            });closing->start();
        }
        return true;
    }
    if(watched==m_window&&(e->type()==QEvent::WindowDeactivate||e->type()==QEvent::UngrabMouse)){
        const bool petDrag=m_petMode&&m_pointerMode==5;
        if(petDrag){m_petMoveTimer.stop();applyDesktopPetMove();}
        m_dragged=true;m_pointerMode=0;m_filePreviewPending=false;
        if(petDrag){m_state["petDragging"]=false;m_clock.restart();publishState();}
        if(auto* module=qobject_cast<PluginModule*>(m_plugins->module()))module->dispatch("input.cancel");
    }
    if(watched==m_window&&m_plugins->isOpen()&&!modalOpen()&&e->type()==QEvent::KeyPress&&static_cast<QKeyEvent*>(e)->key()==Qt::Key_F11){dispatch("window.fullscreen");return true;}
    if(watched!=m_window||inputBlocked())return false;
    if(e->type()!=QEvent::KeyPress&&e->type()!=QEvent::KeyRelease)return false;
    auto* focus=m_window->activeFocusItem();
    if(focus&&focus!=m_window->contentItem()&&focus->objectName()!="spineScene"){m_filePreviewPending=false;return false;}
    auto* key=static_cast<QKeyEvent*>(e);const bool press=e->type()==QEvent::KeyPress;
    if(key->key()==Qt::Key_F11&&press){dispatch("window.fullscreen");return true;}
    if(key->key()==Qt::Key_Up||key->key()==Qt::Key_Down){
        if(press){const bool previewed=previewFile(key->key()==Qt::Key_Up?-1:1,live2dMode());m_filePreviewPending=!live2dMode()&&previewed;}
        else if(!key->isAutoRepeat()&&m_filePreviewPending){m_filePreviewPending=false;openPaths({m_currentPath});}
        return true;
    }
    if((key->key()==Qt::Key_Left||key->key()==Qt::Key_Right)&&press){
        if(live2dMode())m_live2d->command("animation.step",key->key()==Qt::Key_Left?-1:1);
        else if(key->key()==Qt::Key_Left)runtime()->StepToPreviousMotion();else runtime()->StepToNextMotion();refresh();record();return true;
    }
    return false;
}
}
