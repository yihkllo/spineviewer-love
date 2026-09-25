#include "core/export_service.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QFile>
#include <QTemporaryDir>

#include <cmath>
#include <limits>
#include <iostream>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) {
        qCritical().noquote() << "FAIL:" << message;
        ++failures;
    }
}
void writeBytes(const QString& path,const QByteArray& bytes)
{
    QFile file(path);check(file.open(QIODevice::WriteOnly),"fixture output is writable");
    check(file.write(bytes)==bytes.size(),"fixture bytes are written completely");
}
QByteArray readBytes(const QString& path)
{ QFile file(path);if(!file.open(QIODevice::ReadOnly))return {};return file.readAll(); }
void removeRecovery(const QString& path)
{
    const QFileInfo directory(path);
    const bool owned=directory.fileName().startsWith("spinelove_frames_")
        &&QFileInfo(directory.absolutePath()).canonicalFilePath()==QFileInfo(QDir::tempPath()).canonicalFilePath();
    check(owned,"test recovery cleanup stays inside service-owned temporary directory");
    if(owned)QDir(path).removeRecursively();
}
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    using slqt::ExportService;
    check(ExportService::clampFps(0) == 1 && ExportService::clampFps(200) == 120, "fps clamp preserves limits");
    check(ExportService::frameCount(1.01, 30) == 31, "partial final interval rounds up");
    check(ExportService::frameCount(static_cast<double>(1.2f), 30) == 36,
          "runtime float duration preserves original float multiplication before ceil");
    check(ExportService::frameCount(0, 30) == 1 && ExportService::frameCount(-1, 30) == 1
              && ExportService::frameCount(std::numeric_limits<double>::quiet_NaN(), 30) == 1,
          "unknown and invalid durations still emit one frame");
    QImage source(2, 1, QImage::Format_ARGB32_Premultiplied);
    auto* pixels = reinterpret_cast<QRgb*>(source.scanLine(0));
    pixels[0] = qRgba(64, 32, 16, 128);
    pixels[1] = qRgba(50, 30, 10, 0);
    const auto alpha = ExportService::outputPixels(source, true, Qt::white);
    check(alpha.pixel(0, 0) == qRgba(128, 64, 32, 128), "PNG straight-alpha unpremultiply matches WIC predecessor rounding");
    check(alpha.pixel(1, 0) == qRgba(0, 0, 0, 0), "transparent PNG pixels clear hidden RGB");
    const auto matte = ExportService::outputPixels(source, false, QColor(200, 100, 0));
    check(matte.pixel(0, 0) == qRgb(164, 82, 16), "opaque matte compositing matches original integer equation");
    QTemporaryDir temporary;
    check(temporary.isValid(), "isolated export directory exists");
    if (!temporary.isValid())
        return 1;
    const QString png = temporary.filePath(QStringLiteral("半透明.png"));
    QString error;
    check(ExportService::saveImage(png, slqt::ImageFormat::Png, source, true, Qt::black, &error), "Unicode PNG writes successfully");
    const QImage decoded(png);
    check(!decoded.isNull() && decoded.pixel(0, 0) == alpha.pixel(0, 0), "PNG roundtrip keeps exact alpha pixels");
    check(ExportService::saveImage(temporary.filePath("opaque.jpg"), slqt::ImageFormat::Jpeg,
                                  source, true, Qt::white, &error), "JPEG encoder is available and forces opaque output");
    const auto mp4 = ExportService::movieArguments("C:/a b", "C:/out & quoted.mp4", slqt::MovieFormat::Mp4, true, 60);
    check(mp4.size() == 3 && mp4[0].contains("libx264") && mp4[1].contains("h264_mf") && mp4[2].contains("h264_nvenc"),
          "MP4 encoder fallback order preserved");
    check(mp4[0].contains("crop=trunc(iw/2)*2:trunc(ih/2)*2") && mp4[0].back() == "C:/out & quoted.mp4",
          "odd dimensions use original even crop; output path is a single process argument");
    const auto webm = ExportService::movieArguments("frames", "clip.webm", slqt::MovieFormat::Webm, true, 60);
    check(webm.front().contains("yuva420p") && webm.front().contains("-auto-alt-ref"), "transparent WebM flags are retained");
    const auto gif = ExportService::movieArguments("frames", "clip.gif", slqt::MovieFormat::Gif, false, 30);
    check(gif.front().contains("-loop") && gif.front().contains("split[s0][s1];[s0]palettegen=max_colors=256[p];[s1][p]paletteuse=dither=sierra2_4a"),
          "GIF palette and dithering recipe preserved");

    slqt::ExportRequest request;
    request.outputPath = temporary.filePath("sequence");
    request.fps = 30;
    request.motions = {{"first", 0.05}, {"still", 0.0}};
    QList<int> motionCalls;
    QList<int> frameCalls;
    QList<float> timeCalls;
    ExportService service;
    QEventLoop loop;
    bool done = false;
    QObject::connect(&service, &ExportService::finished, &loop, [&](bool success, const QString& message, const QString& recovery) {
        check(success && message.isEmpty() && recovery.isEmpty(), "sequence completes without a recovery error");
        done = true;
        loop.quit();
    });
    const auto renderer = [&](int motion, int frame, float advance, QString*) {
        motionCalls.append(motion);
        frameCalls.append(frame);
        timeCalls.append(advance);
        return source;
    };
    check(service.startFrames(request, renderer, &error), "incremental sequence job starts");
    check(!service.startFrames(request, renderer, &error), "second simultaneous export is refused");
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
    check(done && !service.isRunning(), "incremental rendering completes within event loop");
    check(motionCalls == QList<int>{0, 0, 1} && frameCalls == QList<int>{0, 1, 0}, "queue emits each complete motion in order");
    check(timeCalls.size() == 3 && timeCalls[0] == 0 && std::abs(timeCalls[1] - 1.0f / 30) < 0.00001f && timeCalls[2] == 0,
          "each motion starts at zero then advances exactly one frame period");
    check(QFileInfo::exists(QDir(request.outputPath).filePath("frame_000001.png"))
              && QFileInfo::exists(QDir(request.outputPath).filePath("frame_000003.png")),
          "frame filenames are one-based padded six digits");

    ExportService cancelled;
    bool cancelledSignal = false;
    QObject::connect(&cancelled, &ExportService::finished, [&](bool success, const QString&, const QString&) {
        cancelledSignal = !success;
    });
    check(cancelled.startFrames(request, renderer, &error), "cancellation fixture starts");
    cancelled.cancel();
    check(cancelledSignal && !cancelled.isRunning(), "cancellation reliably triggers restoration signal");
    check(QFileInfo::exists(request.outputPath), "cancellation never deletes user's frame output directory");
    cancelled.cancel();
    check(cancelled.cancellationRequested(),"cancel is observable even before a frame job starts");
    cancelled.resetCancellation();
    check(!cancelled.cancellationRequested(),"new export can reset a completed cancellation token");

    {
        ExportService reentrant;QEventLoop eventLoop;int calls=0,completions=0;bool nestedAccepted=true;
        slqt::ExportRequest abortRequest=request;abortRequest.outputPath=temporary.filePath("callback_cancel");
        QObject::connect(&reentrant,&ExportService::finished,&eventLoop,[&](bool success,const QString&,const QString&){
            ++completions;check(!success,"callback cancellation reports failure");
            nestedAccepted=reentrant.startFrames(abortRequest,renderer,&error);eventLoop.quit();
        });
        check(reentrant.startFrames(abortRequest,[&](int,int,float,QString*){++calls;reentrant.cancel();return source;},&error),"callback cancellation starts");
        QTimer::singleShot(5000,&eventLoop,&QEventLoop::quit);eventLoop.exec();
        check(calls==1&&completions==1&&!nestedAccepted&&!reentrant.isBusy(),"cancel cannot reenter a new export before the old render callback unwinds");
        check(!QFileInfo::exists(QDir(abortRequest.outputPath).filePath("frame_000001.png")),"cancelled callback never writes its late returned frame");
    }
    {
        ExportService writeFailure;QEventLoop eventLoop;int completions=0;
        slqt::ExportRequest blocked=request;blocked.outputPath=temporary.filePath("blocked_frame");
        QDir().mkpath(QDir(blocked.outputPath).filePath("frame_000001.png"));
        const QString marker=QDir(blocked.outputPath).filePath("user-note.txt");writeBytes(marker,"preserve me");
        QObject::connect(&writeFailure,&ExportService::finished,&eventLoop,[&](bool success,const QString& message,const QString&){
            ++completions;check(!success&&!message.isEmpty(),"frame write failure is surfaced");eventLoop.quit();
        });
        check(writeFailure.startFrames(blocked,[&](int,int,float,QString*){return source;},&error),"frame write failure fixture starts");
        QTimer::singleShot(5000,&eventLoop,&QEventLoop::quit);eventLoop.exec();
        check(completions==1&&!writeFailure.isRunning()&&readBytes(marker)=="preserve me","write failure completes exactly once and preserves user's other files");
    }
    {
        ExportService invalid;int rendered=0;slqt::ExportRequest absent=request;
        absent.outputPath=temporary.filePath("missing_parent/movie.mp4");
        check(!invalid.startMovie(absent,[&](int,int,float,QString*){++rendered;return source;},&error)&&rendered==0&&!invalid.isRunning(),
              "invalid movie destination fails before rendering or changing playback");
    }

    if (!ExportService::findFfmpeg().isEmpty()) {
        std::cout << "Real ffmpeg executable found; validating MP4, WebM and GIF.\n";
        for (const auto format : {slqt::MovieFormat::Mp4, slqt::MovieFormat::Webm, slqt::MovieFormat::Gif}) {
            const QString extension = format == slqt::MovieFormat::Mp4 ? ".mp4"
                : (format == slqt::MovieFormat::Webm ? ".webm" : ".gif");
            slqt::ExportRequest movie;
            movie.movieFormat = format;
            movie.outputPath = temporary.filePath(QStringLiteral("实际编码") + extension);
            writeBytes(movie.outputPath,"old movie contents");
            movie.motions = {{"sample", 0.1}};
            movie.keepAlpha = true;
            ExportService encoder;
            QEventLoop encodeLoop;
            bool movieDone = false;
            QObject::connect(&encoder, &ExportService::finished, &encodeLoop,
                             [&](bool success, const QString& message, const QString& recovery) {
                if (!success)
                    qCritical().noquote() << "Encoder failure:" << message << recovery;
                check(success && recovery.isEmpty(), "real ffmpeg completes and removes successful temporary frames");
                movieDone = true;
                encodeLoop.quit();
            });
            QImage clipFrame(16, 16, QImage::Format_ARGB32_Premultiplied);
            clipFrame.fill(QColor(200, 80, 40, 128));
            check(encoder.startMovie(movie, [&](int, int, float, QString*) { return clipFrame; }, &error),
                  "real ffmpeg movie job starts");
            QTimer::singleShot(30000, &encodeLoop, &QEventLoop::quit);
            encodeLoop.exec();
            check(movieDone && QFileInfo(movie.outputPath).size() > 0, "real codec writes nonempty video/GIF with Unicode output path");
            check(readBytes(movie.outputPath)!="old movie contents","successful movie atomically replaces the previous output");
            std::cout << "Encoded " << extension.toStdString() << ": " << QFileInfo(movie.outputPath).size() << " bytes\n";
            if (encoder.isRunning())
                encoder.cancel();
        }
        {
            ExportService failure;QEventLoop eventLoop;bool done=false;QString recovery;
            slqt::ExportRequest invalidMovie;invalidMovie.outputPath=temporary.filePath("protected_previous.mp4");invalidMovie.motions={{"one",0.0}};
            writeBytes(invalidMovie.outputPath,"previous finished movie");
            QObject::connect(&failure,&ExportService::finished,&eventLoop,[&](bool success,const QString& message,const QString& frames){
                done=true;recovery=frames;check(!success&&!message.isEmpty(),"all real MP4 codec failures report failure");eventLoop.quit();
            });
            QImage tooSmall(1,1,QImage::Format_ARGB32_Premultiplied);tooSmall.fill(Qt::white);
            check(failure.startMovie(invalidMovie,[&](int,int,float,QString*){return tooSmall;},&error),"MP4 invalid even crop fixture starts");
            QTimer::singleShot(15000,&eventLoop,&QEventLoop::quit);eventLoop.exec();
            check(done&&readBytes(invalidMovie.outputPath)=="previous finished movie","failed encoding preserves the complete previous final movie");
            check(QFileInfo::exists(QDir(recovery).filePath("frame_000001.png")),"encoder failure preserves rendered recovery frames");
            if(done&&!recovery.isEmpty())removeRecovery(recovery);else failure.cancel();
        }
        {
            ExportService abortEncoder;QEventLoop eventLoop;bool done=false;QString recovery;
            slqt::ExportRequest movie;movie.outputPath=temporary.filePath("cancel_previous.webm");movie.movieFormat=slqt::MovieFormat::Webm;movie.motions={{"one",0.0}};
            writeBytes(movie.outputPath,"previous movie before cancellation");
            QObject::connect(&abortEncoder,&ExportService::progressChanged,&eventLoop,[&](int,int,const QString& status){
                if(status=="Encoding video...")QTimer::singleShot(1,&abortEncoder,&ExportService::cancel);
            });
            QObject::connect(&abortEncoder,&ExportService::finished,&eventLoop,[&](bool success,const QString& message,const QString& frames){
                done=true;recovery=frames;check(!success&&message.contains("cancelled"),"encoding cancellation is explicit");eventLoop.quit();
            });
            QImage large(2048,2048,QImage::Format_ARGB32_Premultiplied);large.fill(QColor(240,40,10,128));
            check(abortEncoder.startMovie(movie,[&](int,int,float,QString*){return large;},&error),"encoding cancellation fixture starts");
            QTimer::singleShot(15000,&eventLoop,&QEventLoop::quit);eventLoop.exec();
            check(done&&readBytes(movie.outputPath)=="previous movie before cancellation","cancelled encoder cannot truncate the previous output");
            if(done&&!recovery.isEmpty())removeRecovery(recovery);else abortEncoder.cancel();
        }
        {
            ExportService commitFailure;QEventLoop eventLoop;bool done=false;QString recovery,reported;
            slqt::ExportRequest movie;movie.outputPath=temporary.filePath("late_destination_change.webm");movie.movieFormat=slqt::MovieFormat::Webm;movie.motions={{"one",0.0}};
            QObject::connect(&commitFailure,&ExportService::progressChanged,&eventLoop,[&](int,int,const QString& status){
                if(status=="Encoding video...")QDir().mkpath(movie.outputPath);
            });
            QObject::connect(&commitFailure,&ExportService::finished,&eventLoop,[&](bool success,const QString& message,const QString& frames){
                done=true;recovery=frames;reported=message;check(!success&&message.contains("encoded movie was preserved"),"final rename failure reports the usable encoded candidate");eventLoop.quit();
            });
            QImage frame(16,16,QImage::Format_ARGB32_Premultiplied);frame.fill(Qt::white);
            check(commitFailure.startMovie(movie,[&](int,int,float,QString*){return frame;},&error),"late destination change fixture starts");
            QTimer::singleShot(15000,&eventLoop,&QEventLoop::quit);eventLoop.exec();
            const auto candidates=QDir(temporary.path()).entryList({".spinelove-encode-*"},QDir::Files|QDir::Hidden);
            check(done&&candidates.size()==1,"commit failure keeps exactly its successful encoded candidate");
            if(candidates.size()==1){
                const QString preserved=QDir(temporary.path()).filePath(candidates.front());
                check(QFileInfo(preserved).size()>0&&reported.contains(preserved),"recovery message identifies a nonempty encoded movie");
                QFile::remove(preserved);
            }
            if(done&&!recovery.isEmpty())removeRecovery(recovery);else commitFailure.cancel();
        }
        check(QDir(temporary.path()).entryList({".spinelove-encode-*"},QDir::Files|QDir::Hidden).isEmpty(),
              "successful, failed and cancelled codecs leave no unowned staging movies");
    } else {
        std::cout << "ffmpeg not installed: executable codec integration checks skipped.\n";
    }
    qInfo() << (failures == 0 ? "Export-service compatibility checks passed." : "Export-service checks failed.") << failures;
    return failures == 0 ? 0 : 1;
}
