#pragma once
#include <QObject>
#include <QVariantMap>
#include <QTimer>
#include <QElapsedTimer>
#include <QSettings>
#include <QPointF>
#include <QSet>
#include <QPointer>
#include <QRegion>
#include <QQmlPropertyMap>
#include "spinelove/spine_runtime_registry.h"
#include "spinelove/scene_snapshot.h"
#include "asset_library.h"
#include "spinelove/live2d_bridge.h"
#include "export_service.h"
#include "spinelove/scene_capture_request.h"
#include "../render/scene_export_batch.h"

class QQuickWindow;
class QMenu;
class QQmlEngine;
namespace slqt {
class PluginHost;
class ViewerController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap state READ publishedState NOTIFY stateChanged)
    Q_PROPERTY(QObject* lists READ lists CONSTANT)
    Q_PROPERTY(QObject* plugins READ plugins CONSTANT)
public:
    explicit ViewerController(QObject* parent=nullptr);
    QVariantMap state() const { return m_state; }
    QVariantMap publishedState() const { return m_publishedState; }
    QObject* lists() const { return m_lists; }
    QObject* plugins() const;
    Q_INVOKABLE void dispatch(const QString& command,const QVariant& value=QVariant());
    Q_INVOKABLE void openUrls(const QList<QUrl>& urls);
    Q_INVOKABLE void advanceFrame(){tick();}
    Q_INVOKABLE void resetFrameClock(){m_clock.restart();}
    Q_INVOKABLE bool petHitTest(qreal x,qreal y) const;
    bool desktopPetMode() const { return m_petMode; }
    void openPaths(const QStringList& paths,bool confirmed=false);
    void setWindow(QQuickWindow* window);
    void setViewport(QSizeF logicalSize,qreal dpr);
    std::shared_ptr<const SceneSnapshot> snapshot() const { return m_snapshot; }
    std::shared_ptr<SceneExportBatch> takeExportBatch() { return std::exchange(m_exportBatch, {}); }
    void pointerPress(QPointF,Qt::MouseButton,Qt::KeyboardModifiers);
    void pointerMove(QPointF,Qt::MouseButtons,Qt::KeyboardModifiers);
    void pointerRelease(QPointF,Qt::MouseButton,Qt::KeyboardModifiers);
    void wheel(QPointF,int,Qt::MouseButtons,Qt::KeyboardModifiers);
    void hover(QPointF);
signals:
    void stateChanged();
    void frameChanged();
    void errorOccurred(QString message);
    void languageRequested(QString language);
protected:
    bool eventFilter(QObject*,QEvent*) override;
private:
    SlPlaybackRuntime* runtime() const { return m_hub.CurrentRuntime(); }
    void tick();
    void refresh(bool notify=true);
    void publishState();
    QVariantList fileRows(const QVariantMap& live);
    void record();
    void fit();
    void scanFolder(const QString& folder,bool openAll=false,bool allowModeFallback=false);
    void addLayer(const QString& path);
    bool selectLayer(int index);
    void saveLayerControls();
    void restoreLayerControls();
    void updateSlotFilter();
    void playQueueItem();
    bool previewFile(int delta,bool commit=false);
    void setMode(bool live2d);
    void savePreferences();
    void beginExport(const QString& command,const QVariant& payload);
    QImage captureFrame(bool keepAlpha,QString* error=nullptr);
    bool renderExportBatch(const std::shared_ptr<SceneExportBatch>& batch,QList<QImage>& images,QString* error);
    bool windowCommand(const QString& command,const QVariant& value);
    void enterDesktopPet();
    void exitDesktopPet();
    void updateDesktopPet();
    void updateDesktopPetRegion();
    void updateDesktopPetCanvas(SceneSnapshot& frame);
    void applyDesktopPetMove();
    void showDesktopPetMenu(QPointF position);
    void recordSlotOverlay();
    bool live2dExportCommand(const QString& command,QVariantMap arguments,QString* error=nullptr);
    void fail(const QString& message);
    bool live2dMode() const { return m_state.value("mode").toString()=="live2d"; }
    bool inputBlocked() const;
    void syncPluginSuspension();
    void syncPluginSettings();
    void applyPluginSettings(const QString& action,const QVariant& value);
    bool modalOpen() const { return m_modal || !m_modalPanels.isEmpty(); }
    QStringList visibleFiles() const;
    SlRuntimeHub m_hub;
    std::shared_ptr<Live2DBridge> m_live2d;
    PluginHost* m_plugins=nullptr;
    bool m_pluginsWasOpen=false;
    SceneRecorder m_recorder;
    std::shared_ptr<const SceneSnapshot> m_snapshot;
    QPointer<QQuickWindow> m_window;
    QRect m_petReturnGeometry,m_fullscreenReturnGeometry;
    QRect m_pluginPortraitReturnGeometry;
    int m_pluginPortraitReturnVisibility=0;
    bool m_pluginPortrait=false;
    Qt::WindowFlags m_petReturnFlags;
    QRegion m_petReturnMask,m_petMask,m_petCurrentRegion;
    QPointF m_petCanvasOrigin;
    QPointF m_petStageOrigin;
    QRectF m_petCanvasBounds;
    bool m_petCanvasAdjusting=false;
    QTimer m_petMoveTimer;
    QPoint m_petMoveTarget;
    bool m_petMoveQueued=false;
    QPointer<QMenu> m_petMenu;
    QSize m_defaultWindowSize,m_petReturnMinimum;
    int m_fullscreenReturnVisibility=0;
    bool m_petMode=false,m_petRandom=true,m_resizeEnabled=true,m_nativeFrame=false,m_clickThrough=false;
    qint64 m_nextPetMotion=0;
    QElapsedTimer m_clock;
    QElapsedTimer m_petClock;
    QSettings m_settings;
    ExportService m_exportService;
    bool m_captureAlpha=false,m_exportActive=false,m_exportClosing=false;
    std::shared_ptr<SceneCaptureRequest> m_captureRequest;
    std::shared_ptr<SceneExportBatch> m_exportBatch;
    quint64 m_exportRequestId=0;
    QVariantMap m_state;
    QVariantMap m_publishedState;
    quint64 m_live2dSeenRevision=~quint64(0);
    QQmlPropertyMap* m_lists=nullptr;
    struct FileRowsCache {
        QStringList files,favorites,layers;
        QString current,livePath;
        bool valid=false,live=false,liveLoaded=false,favoritesOnly=false;
        QVariantList rows;
    } m_fileRows;
    QStringList m_files,m_layers,m_favorites,m_queue,m_pendingPaths;
    QSet<int> m_selectedSkins,m_tracks,m_hiddenSlots;
    QSet<QString> m_modalPanels;
    struct LayerControls {
        QSet<int> selectedSkins,tracks,hiddenSlots;
        QString slotQuery,pinnedSlot;
        int lastMixedSkin=-1;
    };
    QHash<QString,LayerControls> m_layerControls;
    QString m_currentPath,m_folder,m_version,m_slotQuery,m_hoveredSlot,m_listHoveredSlot,m_pinnedSlot;
    QString m_live2dPath;
    QStringList m_spineFiles,m_live2dFiles;
    QString m_lastLiveError;
    std::string m_lastAnimationName;
    QString m_cachedSkin;
    QStringList m_cachedMixSkins;
    int m_lastMixedSkin=-1;
    bool m_showLayers=false;
    bool m_cachedWasMix=false;
    int m_pointerMode=0;
    double m_animationTime=0;
    QSize m_viewport{1000,720};
    qreal m_dpr=1;
    bool m_viewportInitialized=false;
    bool m_viewportNotificationPending=false;
    QPointF m_pointerStart,m_pointerLast,m_bgOffset;
    QPoint m_petDragCursor,m_petDragWindow;
    QPointF m_hoverPosition{-1,-1};
    bool m_dragged=false,m_modal=false,m_favoritesOnly=false,m_skinMix=false,m_pma=true,m_resetOnLoad=false;
    bool m_queuePlaying=false,m_hoverEnabled=false,m_invertWheel=false;
    int m_queueIndex=0,m_wheelRemainder=0,m_wheelTarget=0,m_previewIndex=0;
    bool m_filePreviewPending=false;
    float m_mix=0,m_bgScale=1;
    SlTextureId m_background=0;
    quint64 m_titleImageRevision=0;
    QColor m_clearColor=Qt::black;
};
}
