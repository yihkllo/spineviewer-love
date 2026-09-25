#include "viewer_controller.h"

#include <QEventLoop>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScopeGuard>

#include <algorithm>

namespace slqt {
namespace {
enum class RenderWait { Ready, Cancelled, Closed, TimedOut };
RenderWait waitForRender(QPointer<QQuickWindow> window,const std::function<bool()>& ready,
    const std::function<bool()>& cancelled,const std::function<bool()>& closing)
{
    QEventLoop loop;QTimer poll;poll.setInterval(8);QElapsedTimer elapsed;elapsed.start();
    qint64 activeMilliseconds=0;RenderWait result=RenderWait::Closed;
    if(window)QObject::connect(window,&QQuickWindow::afterFrameEnd,&loop,[&]{
        if(result==RenderWait::Closed&&ready()){result=RenderWait::Ready;loop.quit();}
    },Qt::QueuedConnection);
    QObject::connect(&poll,&QTimer::timeout,&loop,[&]{
        if(cancelled()){result=RenderWait::Cancelled;loop.quit();return;}
        if(ready()){result=RenderWait::Ready;loop.quit();return;}
        if(!window){result=RenderWait::Closed;loop.quit();return;}
        const bool visible=window->isVisible()&&window->visibility()!=QWindow::Minimized;
        if(closing()&&!visible){result=RenderWait::Closed;loop.quit();return;}
        const qint64 interval=elapsed.restart();
        if(visible){
            activeMilliseconds+=interval;poll.setInterval(8);
            if(auto* scene=window->findChild<QQuickItem*>("spineScene"))scene->update();
            window->update();
        }else poll.setInterval(100);
        if(activeMilliseconds>=10000){result=RenderWait::TimedOut;loop.quit();}
    });
    if(window)QObject::connect(window,&QObject::destroyed,&loop,&QEventLoop::quit);
    QObject::connect(QCoreApplication::instance(),&QCoreApplication::aboutToQuit,&loop,&QEventLoop::quit);
    if(ready())return RenderWait::Ready;
    poll.start();loop.exec(QEventLoop::ExcludeUserInputEvents);return result;
}
}

bool ViewerController::live2dExportCommand(const QString& command,QVariantMap arguments,QString* error)
{
    if(error)error->clear();
    if(!m_window||!m_live2d){if(error)*error=tr("Live2D render context is not available.");return false;}
    const quint64 requestId=++m_exportRequestId;
    arguments["requestId"]=QVariant::fromValue(requestId);
    m_live2d->command(command,arguments);record();m_window->update();
    bool acknowledged=false,success=false;
    const auto waited=waitForRender(m_window,[&]{
        const auto state=m_live2d->state();
        if(state.value("exportAckId").toULongLong()==requestId){
            acknowledged=true;success=state.value("exportRequestSuccess").toBool();
            if(!success&&error){*error=state.value("exportError").toString();if(error->isEmpty())*error=tr("Live2D export command failed: %1").arg(command);}
        }
        return acknowledged;
    },[&]{return command!="export.end"&&m_exportService.cancellationRequested();},[&]{return m_exportClosing;});
    if(!acknowledged){
        if(error)*error=waited==RenderWait::Cancelled?tr("Export cancelled."):
            waited==RenderWait::Closed?tr("The render window closed during export."):
            tr("Live2D export command timed out: %1").arg(command);
        return false;
    }
    return success;
}

QImage ViewerController::captureFrame(bool keepAlpha,QString* error)
{
    if(error)error->clear();
    if(!m_window){
        if(error)*error=tr("The viewer window is not available to capture its stage.");
        return {};
    }
    const QPointer<QQuickItem> scene=m_window->findChild<QQuickItem*>("spineScene");
    if(!scene||scene->width()<=0||scene->height()<=0){
        if(error)*error=tr("The render stage is not ready for capture.");
        return {};
    }
    const bool previousAlpha=m_captureAlpha;
    const auto previousRequest=m_captureRequest;
    const auto capture=std::make_shared<SceneCaptureRequest>();
    m_captureAlpha=keepAlpha;
    m_captureRequest=capture;
    const auto restore=qScopeGuard([this,previousAlpha,previousRequest]{m_captureAlpha=previousAlpha;m_captureRequest=previousRequest;m_clock.restart();record();});
    record();scene->update();m_window->update();
    const QSize expected=m_viewport;
    const auto waited=waitForRender(m_window,[&]{return capture->completed.load(std::memory_order_acquire);},
        [&]{return m_exportService.cancellationRequested()||!m_exportActive;},[&]{return m_exportClosing;});
    if(waited!=RenderWait::Ready){
        if(error)*error=waited==RenderWait::Cancelled?tr("Export cancelled."):
            waited==RenderWait::Closed?tr("The render window closed during export."):
            tr("Stage capture timed out before the renderer returned pixels.");
        return {};
    }
    if(!capture->error.isEmpty()){if(error)*error=capture->error;return {};}
    const QImage image=capture->image;
    if(image.isNull()||image.size()!=expected){
        if(error)*error=tr("The captured stage dimensions (%1 x %2) do not match the export viewport (%3 x %4; DPR %5).")
            .arg(image.width()).arg(image.height()).arg(expected.width()).arg(expected.height()).arg(m_dpr);
        return {};
    }
    return image;
}

bool ViewerController::renderExportBatch(const std::shared_ptr<SceneExportBatch>& batch,QList<QImage>& images,QString* error)
{
    if(error)error->clear();
    if(!m_window){
        if(error)*error=tr("The viewer window is not available to capture its stage.");
        return false;
    }
    auto* scene=m_window->findChild<QQuickItem*>("spineScene");
    if(!scene||scene->width()<=0||scene->height()<=0){
        if(error)*error=tr("The render stage is not ready for capture.");
        return false;
    }
    m_exportBatch=batch;scene->update();m_window->update();
    const auto waited=waitForRender(m_window,[&]{return batch->completed.load(std::memory_order_acquire);},
        [&]{return m_exportService.cancellationRequested()||!m_exportActive;},[&]{return m_exportClosing;});
    if(m_exportBatch==batch)m_exportBatch.reset();
    if(waited!=RenderWait::Ready){
        if(error)*error=waited==RenderWait::Cancelled?tr("Export cancelled."):
            waited==RenderWait::Closed?tr("The render window closed during export."):
            tr("Stage capture timed out before the renderer returned pixels.");
        return false;
    }
    if(!batch->error.isEmpty()){if(error)*error=batch->error;return false;}
    const QSize expected=m_viewport;
    for(auto& image:batch->images){
        if(image.isNull()||image.size()!=expected){
            if(error)*error=tr("The captured stage dimensions (%1 x %2) do not match the export viewport (%3 x %4; DPR %5).")
                .arg(image.width()).arg(image.height()).arg(expected.width()).arg(expected.height()).arg(m_dpr);
            return false;
        }
        images.append(std::move(image));
    }
    return true;
}

void ViewerController::beginExport(const QString& command,const QVariant& payload)
{
    if(m_exportClosing)return;
    if(m_exportActive||m_exportService.isBusy()){fail(tr("Another export is already running."));return;}
    const bool snapshot=command=="export.png"||command=="export.jpg";
    const bool frames=command=="export.pngFrames"||command=="export.jpgFrames";
    const bool movie=command=="export.mp4"||command=="export.webm"||command=="export.gif";
    if(!snapshot&&!frames&&!movie){fail(tr("Unknown export command."));return;}
    m_exportService.resetCancellation();m_exportActive=true;
    bool asynchronous=false;
    const auto prepareGuard=qScopeGuard([&]{
        if(!asynchronous){m_exportActive=false;m_captureAlpha=false;m_captureRequest.reset();m_state["exportRunning"]=false;m_clock.restart();refresh();record();}
    });
    m_state["exportRunning"]=true;m_state["exportFailed"]=false;m_state["exportDone"]=0;m_state["exportTotal"]=0;
    m_state["exportQueueActive"]=false;m_state["exportQueueIndex"]=0;m_state["exportRecoveryFolder"]=QString{};
    m_state["lastExportPath"]=QString{};m_state["lastError"]=QString{};m_state["exportStatus"]=tr("Preparing export...");
    const auto reject=[&](const QString& error){m_state["exportFailed"]=true;m_state["exportStatus"]=tr("Export failed.");if(!m_exportClosing)fail(error);};
    refresh();
    const bool live=live2dMode();
    if(live){
        QString syncError;
        if(!live2dExportCommand("export.sync",{},&syncError)){reject(syncError);return;}
        refresh();
    }
    if(!m_state.value("loaded").toBool()){reject(tr("Nothing is loaded to export."));return;}
    const QVariantMap options=payload.toMap();
    if(movie&&ExportService::findFfmpeg().isEmpty()){
        const QString folder=QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
        const QString message=tr("Video export needs ffmpeg.exe, which was not found.")+"\n\n"
            +tr("Put ffmpeg.exe in this folder, then export again:")+"\n"+folder;
        if(!options.value("path").toString().isEmpty()||qApp->property("qaSilent").toBool()){reject(message);return;}
        m_modal=true;
        QMessageBox box(QMessageBox::Warning,tr("ffmpeg not found"),message);
        const auto* download=box.addButton(tr("Download ffmpeg"),QMessageBox::ActionRole);
        const auto* openFolder=box.addButton(tr("Open folder"),QMessageBox::ActionRole);
        box.addButton(QMessageBox::Ok);
        box.exec();
        m_modal=false;m_clock.restart();
        if(box.clickedButton()==download)QDesktopServices::openUrl(QUrl("https://github.com/BtbN/FFmpeg-Builds/releases"));
        else if(box.clickedButton()==openFolder)QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
        m_state["exportFailed"]=true;m_state["exportStatus"]=tr("Video export needs ffmpeg.exe, which was not found.");
        return;
    }
    const bool jpeg=command=="export.jpg"||command=="export.jpgFrames";
    const bool keepAlpha=options.value("alpha",m_state.value("exportAlpha",true)).toBool()
        &&!jpeg&&(!movie||command=="export.webm");
    const ImageFormat imageFormat=jpeg?ImageFormat::Jpeg:ImageFormat::Png;
    const QString extension=movie?(command=="export.mp4"?".mp4":command=="export.gif"?".gif":".webm"):(jpeg?".jpg":".png");
    QString path=options.value("path").toString();
    if(path.isEmpty()){
        m_modal=true;
        if(frames)path=QFileDialog::getExistingDirectory(nullptr,tr("Export animation frames"));
        else{
            QString base=m_state.value("currentFileName").toString();
            if(base.isEmpty())base="spinelove_export";
            path=QFileDialog::getSaveFileName(nullptr,tr("Export"),base+extension,
                                             tr("Export files (*%1);;All files (*)").arg(extension));
        }
        m_modal=false;m_clock.restart();
        if(path.isEmpty()){m_state["exportStatus"]=QString{};return;}
    }
    if(m_exportService.cancellationRequested()){reject(tr("Export cancelled."));return;}
    if(!frames&&!path.endsWith(extension,Qt::CaseInsensitive)
        &&!(jpeg&&path.endsWith(".jpeg",Qt::CaseInsensitive)))path+=extension;
    path=QFileInfo(path).absoluteFilePath();
    const QColor matte=m_clearColor;
    if(snapshot){
        m_exportActive=true;
        m_state["exportRunning"]=true;m_state["exportFailed"]=false;m_state["exportTotal"]=1;m_state["exportDone"]=0;
        m_state["exportStatus"]=tr("Capturing stage...");refresh();
        QString error;
        const QImage image=captureFrame(keepAlpha,&error);
        const bool ok=!image.isNull()&&ExportService::saveImage(path,imageFormat,image,keepAlpha,matte,&error);
        m_exportActive=false;m_captureAlpha=false;
        m_state["exportRunning"]=false;m_state["exportFailed"]=!ok;m_state["exportDone"]=ok?1:0;
        m_state["exportStatus"]=ok?tr("Snapshot export complete."):tr("Snapshot export failed.");
        m_state["lastExportPath"]=ok?path:QString{};
        m_clock.restart();refresh();record();if(!ok&&!m_exportClosing)fail(error);
        return;
    }

    auto* r=runtime();
    ExportRequest request;
    request.outputPath=path;request.imageFormat=imageFormat;request.keepAlpha=keepAlpha;request.matteColor=matte;
    request.movieFormat=command=="export.webm"?MovieFormat::Webm:command=="export.gif"?MovieFormat::Gif:MovieFormat::Mp4;
    request.fps=ExportService::clampFps(m_state.value(movie?"exportVideoFps":"exportImageFps",movie?60:30).toInt());
    const bool queue=options.value("queue",m_state.value("exportQueue",false)).toBool();
    request.queue=queue;
    QStringList motions;
    QList<int> liveMotionIndices;
    if(live){
        const auto liveState=m_live2d->state();
        const auto catalog=liveState.value("animations").toList();
        if(catalog.isEmpty()){reject(tr("No animation is available to export."));return;}
        if(queue){
            const auto queued=liveState.value("queue").toList();
            if(queued.isEmpty()){reject(tr("Animation queue is empty."));return;}
            for(const auto& row:queued){
                const QString name=row.toMap().value("name").toString();int found=-1;
                for(int i=0;i<catalog.size();++i)if(catalog[i].toMap().value("name").toString()==name){found=i;break;}
                if(found<0){reject(tr("The animation queue references a motion this model does not have."));return;}
                liveMotionIndices.append(found);
            }
        }else{
            int index=liveState.value("currentAnimation").toInt();if(index<0||index>=catalog.size())index=0;
            liveMotionIndices.append(index);
        }
        for(int index:liveMotionIndices){const auto row=catalog[index].toMap();const QString name=row.value("name").toString();motions.append(name);request.motions.append({name,row.value("duration").toDouble()});}
    }else if(queue){for(const auto& name:m_queue)if(!name.isEmpty())motions.append(name);}
    else{
        std::string name=r->ActiveMotionName();if(name.empty()&&!r->MotionNames().empty())name=r->MotionNames().front();
        if(!name.empty())motions.append(QString::fromUtf8(name.data(),qsizetype(name.size())));
    }
    if(motions.isEmpty()){reject(queue?tr("Animation queue is empty."):tr("No animation is available to export."));return;}
    if(!live)for(const auto& name:motions)request.motions.append({name,r->MotionDuration(name.toUtf8().constData())});
    const std::string restoreMotion=r->ActiveMotionName();
    const float restoreSpeed=r->TimeScale();
    float restoreLast=0;r->ReadMotionClock(nullptr,&restoreLast,nullptr,nullptr);
    m_exportActive=true;
    m_state["exportRunning"]=true;m_state["exportFailed"]=false;m_state["exportDone"]=0;
    m_state["exportQueueActive"]=queue;m_state["exportQueueIndex"]=0;m_state["exportRecoveryFolder"]=QString{};
    m_clock.restart();
    struct PlaybackChange { bool changed=false;bool beginAttempted=false; };
    const auto playback=std::make_shared<PlaybackChange>();
    QObject::disconnect(&m_exportService,nullptr,this,nullptr);
    QObject::connect(&m_exportService,&ExportService::progressChanged,this,[this](int done,int total,const QString& status){
        m_state["exportDone"]=done;m_state["exportTotal"]=total;m_state["exportStatus"]=status;publishState();
    });
    const auto finish=[this,r,restoreMotion,restoreSpeed,restoreLast,path,movie,live,playback](bool success,const QString& originalError,const QString& recovery){
        QString error=originalError;
        if(live&&playback->beginAttempted){
            QString restoreError;
            if(!live2dExportCommand("export.end",{},&restoreError)){
                success=false;error+=(error.isEmpty()?QString{}:QStringLiteral("\n"))+restoreError;
            }
        }else if(!live&&playback->changed){
            r->SetTimeScale(1.f);
            if(!restoreMotion.empty()){r->PlayMotionByName(restoreMotion.c_str());if(restoreLast>0)r->TickPlayback(restoreLast);}
            r->SetTimeScale(restoreSpeed);
        }
        m_captureAlpha=false;m_exportActive=false;
        m_state["exportRunning"]=false;m_state["exportFailed"]=!success;
        m_state["exportQueueActive"]=false;m_state["exportQueueIndex"]=0;
        m_state["exportStatus"]=success?(movie?tr("Video export complete."):tr("Frame export complete.")):tr("Export failed.");
        m_state["exportRecoveryFolder"]=recovery;m_state["lastExportPath"]=success?path:QString{};
        m_clock.restart();refresh();record();
        if(!success&&!m_exportClosing)fail(error+(recovery.isEmpty()?QString{}:tr("\nRendered frames were preserved in:\n")+recovery));
    };
    QObject::connect(&m_exportService,&ExportService::finished,this,finish);
    const auto render=[this,r,motions,keepAlpha,live,liveMotionIndices,playback,fps=request.fps](const QList<ExportFrameStep>& steps,QString* error){
        if(!m_exportActive||(!live&&!r->ContainsDrawableContent())){if(error)*error=tr("The model became unavailable during export.");return QList<QImage>{};}
        if(live&&!playback->beginAttempted){
            playback->beginAttempted=true;
            if(!live2dExportCommand("export.begin",{{"motionIndex",liveMotionIndices.front()},{"fps",fps}},error))return QList<QImage>{};
            playback->changed=true;
        }
        auto batch=std::make_shared<SceneExportBatch>();
        const bool previousAlpha=m_captureAlpha;m_captureAlpha=keepAlpha;
        for(const auto& step:steps){
            SceneExportBatch::Frame frame;
            if(live){
                if(step.frameInMotion==0&&step.motionIndex>0)frame.live2dMotion=liveMotionIndices[step.motionIndex];
                else if(step.frameInMotion>0){frame.live2dStep=true;frame.live2dAdvance=step.advanceSeconds;}
            }else if(step.frameInMotion==0){
                playback->changed=true;r->SetTimeScale(1.f);r->PlayMotionByName(motions[step.motionIndex].toUtf8().constData());r->TickPlayback(0.f);
            }else r->TickPlayback(step.advanceSeconds);
            record();
            frame.snapshot=m_snapshot;
            batch->frames.push_back(std::move(frame));
        }
        m_state["exportQueueIndex"]=steps.back().motionIndex;
        QList<QImage> images;
        const bool rendered=renderExportBatch(batch,images,error);
        m_captureAlpha=previousAlpha;m_clock.restart();record();
        if(!rendered)return QList<QImage>{};
        if(m_exportService.cancellationRequested()||!m_exportActive){if(error)*error=tr("Export cancelled.");return QList<QImage>{};}
        return images;
    };
    QString error;
    const bool started=movie?m_exportService.startMovie(request,ExportService::RenderFrames(render),&error)
        :m_exportService.startFrames(request,ExportService::RenderFrames(render),&error);
    if(!started)finish(false,error,{});
    asynchronous=started&&m_exportService.isRunning();
}

}
