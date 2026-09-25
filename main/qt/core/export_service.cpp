#include "export_service.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImageWriter>
#include <QMutexLocker>
#include <QSaveFile>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QThread>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <utility>

namespace slqt {
namespace {
bool failWith(QString* target, const QString& error)
{
    if (target)
        *target = error;
    return false;
}
int straight(int channel, int alpha)
{
    return alpha == 0 ? 0 : std::min(255, (channel * 255 + alpha / 2) / alpha);
}
const std::array<std::uint8_t, 256 * 256>& straightTable()
{
    static const auto table = [] {
        std::array<std::uint8_t, 256 * 256> values{};
        for (int alpha = 0; alpha < 256; ++alpha)
            for (int channel = 0; channel < 256; ++channel)
                values[alpha * 256 + channel] = std::uint8_t(straight(channel, alpha));
        return values;
    }();
    return table;
}
constexpr qint64 BatchBudgetBytes = qint64(160) << 20;
constexpr qint64 WriteBudgetBytes = qint64(256) << 20;
constexpr int MaxBatchFrames = 16;
constexpr int MaxPendingWrites = 48;
constexpr int MovieFramePngQuality = 80;
constexpr int SequenceFramePngQuality = 50;
}

ExportService::ExportService(QObject* parent) : QObject(parent)
{
    m_writers.setMaxThreadCount(std::clamp(QThread::idealThreadCount() - 1, 1, 8));
    m_frameTimer.setSingleShot(true);
    connect(&m_frameTimer, &QTimer::timeout, this, &ExportService::renderNext);
    connect(&m_encoder, &QProcess::readyReadStandardError, this, [this] {
        m_encoderError += QString::fromUtf8(m_encoder.readAllStandardError());
        if (m_encoderError.size() > 8192)
            m_encoderError = m_encoderError.right(8192);
    });
    connect(&m_encoder, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (!m_running)
            return;
        if (error == QProcess::FailedToStart) {
            m_encoderError = m_encoder.errorString();
            ++m_encodeAttempt;
            const auto generation = m_generation;
            QTimer::singleShot(0, this, [this, generation] {
                if (m_generation == generation)
                    encodeNext();
            });
        }
    });
    connect(&m_encoder, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (!m_running)
            return;
        if (m_cancelled) {
            finish(false, QStringLiteral("Export cancelled."));
            return;
        }
        if (exitStatus == QProcess::NormalExit && exitCode == 0
            && QFileInfo(m_stagedMoviePath).size() > 0) {
            QString error;
            const bool committed = commitMovie(&error);
            finish(committed, error);
            return;
        }
        ++m_encodeAttempt;
        const auto generation = m_generation;
        QTimer::singleShot(0, this, [this, generation] {
            if (m_generation == generation)
                encodeNext();
        });
    });
}

ExportService::~ExportService()
{
    m_writers.waitForDone();
    m_running = false;
    m_frameTimer.stop();
    if (m_encoder.state() != QProcess::NotRunning) {
        m_encoder.kill();
        m_encoder.waitForFinished(1000);
    }
    removeStagedMovie();
}

int ExportService::clampFps(int fps) noexcept { return std::clamp(fps, 1, 120); }

int ExportService::frameCount(double durationSeconds, int fps) noexcept
{
    if (!std::isfinite(durationSeconds) || durationSeconds <= 0.0)
        return 1;
    const double count = std::ceil(static_cast<float>(durationSeconds) * static_cast<float>(clampFps(fps)));
    if (!std::isfinite(count) || count > static_cast<double>(std::numeric_limits<int>::max()))
        return 0;
    return std::max(1, static_cast<int>(count));
}

QImage ExportService::outputPixels(const QImage& image, bool keepAlpha, const QColor& matteColor)
{
    if (image.isNull())
        return {};
    const QImage pma = image.format() == QImage::Format_RGBA8888_Premultiplied
        ? image : image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    QImage output(pma.size(), keepAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
    if (output.isNull())
        return {};
    const auto& table = straightTable();
    std::array<std::uint8_t, 256> matte[3];
    for (int inverse = 0; inverse < 256; ++inverse) {
        matte[0][inverse] = std::uint8_t((matteColor.red() * inverse + 127) / 255);
        matte[1][inverse] = std::uint8_t((matteColor.green() * inverse + 127) / 255);
        matte[2][inverse] = std::uint8_t((matteColor.blue() * inverse + 127) / 255);
    }
    for (int y = 0; y < pma.height(); ++y) {
        const uchar* source = pma.constScanLine(y);
        uchar* target = output.scanLine(y);
        for (int x = 0; x < pma.width(); ++x, source += 4) {
            const int alpha = source[3];
            if (keepAlpha) {
                const auto* row = table.data() + alpha * 256;
                target[0] = row[source[0]];
                target[1] = row[source[1]];
                target[2] = row[source[2]];
                target[3] = uchar(alpha);
                target += 4;
            } else {
                const int inverse = 255 - alpha;
                target[0] = uchar(std::min(255, source[0] + matte[0][inverse]));
                target[1] = uchar(std::min(255, source[1] + matte[1][inverse]));
                target[2] = uchar(std::min(255, source[2] + matte[2][inverse]));
                target += 3;
            }
        }
    }
    return output;
}

bool ExportService::saveImage(const QString& path, ImageFormat format, const QImage& image,
                             bool keepAlpha, const QColor& matteColor, QString* error)
{
    return writeImage(path, format, image, keepAlpha, matteColor, -1, error);
}

bool ExportService::writeImage(const QString& path, ImageFormat format, const QImage& image, bool keepAlpha,
                               const QColor& matteColor, int pngQuality, QString* error)
{
    if (error)
        error->clear();
    if (path.isEmpty())
        return failWith(error, QStringLiteral("Export path is empty."));
    if (image.isNull())
        return failWith(error, QStringLiteral("Nothing is available to export."));
    const QImage pixels = outputPixels(image, format == ImageFormat::Png && keepAlpha, matteColor);
    if (pixels.isNull())
        return failWith(error, QStringLiteral("Could not allocate export pixels."));
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return failWith(error, file.errorString());
    QImageWriter writer(&file, format == ImageFormat::Png ? QByteArray("png") : QByteArray("jpg"));
    if (format == ImageFormat::Png && pngQuality >= 0)
        writer.setQuality(pngQuality);
    if (!writer.write(pixels))
        return failWith(error, writer.errorString());
    if (!file.commit())
        return failWith(error, file.errorString());
    return true;
}

QString ExportService::findFfmpeg()
{
    const QString name = QStringLiteral("ffmpeg")
#ifdef Q_OS_WIN
        + QStringLiteral(".exe")
#endif
        ;
    const QDir application(QCoreApplication::applicationDirPath());
    const QStringList candidates{application.filePath(name), application.filePath("tools/" + name)};
    for (const auto& path : candidates) {
        const QFileInfo info(path);
        if (info.isFile() && info.isExecutable())
            return info.absoluteFilePath();
    }
    return QStandardPaths::findExecutable(name);
}

QList<QStringList> ExportService::movieArguments(const QString& frameFolder, const QString& outputPath,
                                                MovieFormat format, bool keepAlpha, int fps)
{
    const QStringList base{"-hide_banner", "-loglevel", "error", "-y", "-framerate",
                           QString::number(clampFps(fps)), "-start_number", "1", "-i",
                           QDir(frameFolder).filePath("frame_%06d.png")};
    if (format == MovieFormat::Mp4) {
        const QStringList prefix = base + QStringList{"-vf", "crop=trunc(iw/2)*2:trunc(ih/2)*2"};
        const QStringList tail{"-pix_fmt", "yuv420p", "-movflags", "+faststart", outputPath};
        return {prefix + QStringList{"-c:v", "libx264", "-crf", "17"} + tail,
                prefix + QStringList{"-c:v", "h264_mf", "-b:v", "12M"} + tail,
                prefix + QStringList{"-c:v", "h264_nvenc", "-cq", "18"} + tail};
    }
    if (format == MovieFormat::Gif)
        return {base + QStringList{"-vf", "split[s0][s1];[s0]palettegen=max_colors=256[p];[s1][p]paletteuse=dither=sierra2_4a",
                                   "-loop", "0", outputPath}};
    if (keepAlpha)
        return {base + QStringList{"-c:v", "libvpx-vp9", "-crf", "17", "-b:v", "0", "-pix_fmt", "yuva420p",
                                   "-auto-alt-ref", "0", outputPath}};
    return {base + QStringList{"-c:v", "libvpx-vp9", "-crf", "24", "-b:v", "0", "-pix_fmt", "yuv420p", outputPath}};
}

bool ExportService::startFrames(const ExportRequest& request, RenderFrame render, QString* error)
{ return start(request, perFrame(std::move(render)), false, error); }

bool ExportService::startMovie(const ExportRequest& request, RenderFrame render, QString* error)
{ return start(request, perFrame(std::move(render)), true, error); }

bool ExportService::startFrames(const ExportRequest& request, RenderFrames render, QString* error)
{ return start(request, std::move(render), false, error); }

bool ExportService::startMovie(const ExportRequest& request, RenderFrames render, QString* error)
{ return start(request, std::move(render), true, error); }

ExportService::RenderFrames ExportService::perFrame(RenderFrame render)
{
    if (!render)
        return {};
    return [this, render = std::move(render)](const QList<ExportFrameStep>& steps, QString* error) {
        QList<QImage> images;
        for (const auto& step : steps) {
            QImage image = render(step.motionIndex, step.frameInMotion, step.advanceSeconds, error);
            if (image.isNull())
                break;
            images.append(std::move(image));
            if (m_cancelled || !m_running)
                break;
        }
        return images;
    };
}

bool ExportService::start(const ExportRequest& request, RenderFrames render, bool movie, QString* error)
{
    if (error)
        error->clear();
    if (isBusy())
        return failWith(error, QStringLiteral("Another export is already running."));
    if (!render || request.outputPath.isEmpty())
        return failWith(error, QStringLiteral("Export output and renderer are required."));
    if (request.motions.isEmpty())
        return failWith(error, QStringLiteral("No animation is available to export."));
    QList<int> counts;
    int total = 0;
    for (const auto& motion : request.motions) {
        const int count = frameCount(motion.durationSeconds, request.fps);
        if (count == 0 || total > std::numeric_limits<int>::max() - count)
            return failWith(error, QStringLiteral("The export contains too many frames."));
        counts.append(count);
        total += count;
    }
    QString folder, stagedMovie;
    if (movie) {
        const QFileInfo output(request.outputPath);
        if (!QFileInfo(output.absolutePath()).isDir())
            return failWith(error, QStringLiteral("The export directory does not exist."));
        if (output.exists() && !output.isFile())
            return failWith(error, QStringLiteral("The movie export path is not a file."));
        const QString suffix = request.movieFormat == MovieFormat::Mp4 ? ".mp4"
            : request.movieFormat == MovieFormat::Gif ? ".gif" : ".webm";
        QTemporaryFile encoded(QDir(output.absolutePath()).filePath(".spinelove-encode-XXXXXX" + suffix));
        if (!encoded.open())
            return failWith(error, encoded.errorString());
        QTemporaryDir temporary(QDir::tempPath() + "/spinelove_frames_XXXXXX");
        if (!temporary.isValid())
            return failWith(error, QStringLiteral("Could not create temporary export directory."));
        temporary.setAutoRemove(false);
        encoded.setAutoRemove(false);
        stagedMovie = encoded.fileName();
        folder = temporary.path();
    } else {
        if (!QDir().mkpath(request.outputPath))
            return failWith(error, QStringLiteral("Could not create export directory."));
        folder = QFileInfo(request.outputPath).absoluteFilePath();
    }
    m_request = request;
    ++m_generation;
    m_request.outputPath = QFileInfo(request.outputPath).absoluteFilePath();
    m_request.fps = clampFps(request.fps);
    if (movie) {
        m_request.imageFormat = ImageFormat::Png;
        m_request.keepAlpha = request.movieFormat == MovieFormat::Webm && request.keepAlpha;
    }
    m_render = std::move(render);
    m_frameCounts = counts;
    m_frameFolder = folder;
    m_stagedMoviePath = stagedMovie;
    m_keepStagedMovie = false;
    m_ownedTemporaryDirectory = movie;
    m_movie = movie;
    m_running = true;
    m_cancelled = false;
    m_totalFrames = total;
    m_completedFrames = m_motionIndex = m_frameInMotion = m_encodeAttempt = 0;
    m_frameSize = {};
    m_writeLimit = 2;
    m_encoderError.clear();
    {
        QMutexLocker lock(&m_writeMutex);
        m_writeError.clear();
    }
    emit progressChanged(0, total, renderStatus(0));
    m_frameTimer.start(0);
    return true;
}

int ExportService::batchFrames() const
{
    if (m_frameSize.isEmpty())
        return 1;
    const qint64 bytes = std::max<qint64>(1, qint64(m_frameSize.width()) * m_frameSize.height() * 4);
    return int(std::clamp<qint64>(BatchBudgetBytes / bytes, 1, MaxBatchFrames));
}

QString ExportService::renderStatus(int motionIndex) const
{
    if (m_request.queue && motionIndex >= 0 && motionIndex < m_request.motions.size())
        return (m_movie ? tr("Rendering queue video (%1/%2) %3...") : tr("Rendering queue frames (%1/%2) %3..."))
            .arg(motionIndex + 1).arg(m_request.motions.size()).arg(m_request.motions[motionIndex].name);
    return m_movie ? tr("Rendering video frames...") : tr("Rendering frames...");
}

void ExportService::writeFrame(const QString& path, const QImage& frame)
{
    m_pendingWrites.fetch_add(1, std::memory_order_acq_rel);
    const int quality = m_movie ? MovieFramePngQuality : SequenceFramePngQuality;
    m_writers.start([this, path, frame, format = m_request.imageFormat, keepAlpha = m_request.keepAlpha,
                     matte = m_request.matteColor, quality] {
        QString error;
        if (!writeImage(path, format, frame, keepAlpha, matte, quality, &error)) {
            QMutexLocker lock(&m_writeMutex);
            if (m_writeError.isEmpty())
                m_writeError = error.isEmpty() ? QStringLiteral("Could not save animation frame.") : error;
        }
        m_pendingWrites.fetch_sub(1, std::memory_order_acq_rel);
    });
}

QString ExportService::writeError() const
{
    QMutexLocker lock(&m_writeMutex);
    return m_writeError;
}

void ExportService::renderNext()
{
    if (!m_running)
        return;
    if (m_cancelled) {
        finish(false, QStringLiteral("Export cancelled."));
        return;
    }
    if (const QString failed = writeError(); !failed.isEmpty()) {
        finish(false, failed);
        return;
    }
    if (m_pendingWrites.load(std::memory_order_acquire) >= m_writeLimit) {
        m_frameTimer.start(2);
        return;
    }
    QList<ExportFrameStep> steps;
    int motion = m_motionIndex, frame = m_frameInMotion;
    const int count = std::min(batchFrames(), m_totalFrames - m_completedFrames);
    for (int i = 0; i < count; ++i) {
        while (frame >= m_frameCounts[motion]) {
            ++motion;
            frame = 0;
        }
        steps.append({motion, frame, frame == 0 ? 0.0f : 1.0f / m_request.fps});
        ++frame;
    }
    QString error;
    const auto generation = m_generation;
    const RenderFrames renderer = m_render;
    QList<QImage> frames;
    {
        QScopedValueRollback<bool> rendering(m_rendering, true);
        frames = renderer(steps, &error);
    }
    if (!m_running || m_generation != generation)
        return;
    const QString suffix = m_request.imageFormat == ImageFormat::Png ? ".png" : ".jpg";
    int accepted = 0;
    for (; accepted < frames.size() && accepted < steps.size(); ++accepted) {
        const QImage& image = frames[accepted];
        if (image.isNull())
            break;
        if (m_frameSize.isEmpty()) {
            m_frameSize = image.size();
            const qint64 bytes = std::max<qint64>(1, qint64(m_frameSize.width()) * m_frameSize.height() * 4);
            m_writeLimit = int(std::clamp<qint64>(WriteBudgetBytes / bytes, 2, MaxPendingWrites));
        }
        if (image.size() != m_frameSize) {
            finish(false, QStringLiteral("Export frame dimensions changed during rendering."));
            return;
        }
        const QString path = QDir(m_frameFolder).filePath(
            QStringLiteral("frame_%1").arg(m_completedFrames + 1, 6, 10, QLatin1Char('0')) + suffix);
        writeFrame(path, image);
        ++m_completedFrames;
        m_motionIndex = steps[accepted].motionIndex;
        m_frameInMotion = steps[accepted].frameInMotion + 1;
    }
    if (accepted < steps.size()) {
        finish(false, !error.isEmpty() ? error
            : m_cancelled ? QStringLiteral("Export cancelled.") : QStringLiteral("Could not render animation frame."));
        return;
    }
    emit progressChanged(m_completedFrames, m_totalFrames, renderStatus(m_motionIndex));
    if (!m_running || m_generation != generation)
        return;
    if (m_completedFrames == m_totalFrames) {
        m_writers.waitForDone();
        if (const QString failed = writeError(); !failed.isEmpty()) {
            finish(false, failed);
            return;
        }
        if (!m_movie) {
            finish(true, {});
            return;
        }
        m_ffmpeg = findFfmpeg();
        m_encodeArguments = movieArguments(m_frameFolder, m_stagedMoviePath,
                                           m_request.movieFormat, m_request.keepAlpha, m_request.fps);
        encodeNext();
        return;
    }
    m_frameTimer.start(0);
}

void ExportService::encodeNext()
{
    if (!m_running)
        return;
    if (m_cancelled) {
        finish(false, QStringLiteral("Export cancelled."));
        return;
    }
    if (m_ffmpeg.isEmpty()) {
        finish(false, QStringLiteral("ffmpeg was not found. Put it beside the application, in its tools folder, or on PATH."));
        return;
    }
    if (m_encodeAttempt >= m_encodeArguments.size()) {
        finish(false, QStringLiteral("Video encoding failed.\n%1").arg(m_encoderError));
        return;
    }
    const auto generation = m_generation;
    emit progressChanged(m_completedFrames, m_totalFrames, tr("Encoding video..."));
    if (!m_running || m_generation != generation)
        return;
    if (QFileInfo::exists(m_stagedMoviePath) && !QFile::remove(m_stagedMoviePath)) {
        finish(false, QStringLiteral("Could not prepare the temporary movie output."));
        return;
    }
    m_encoder.setProgram(m_ffmpeg);
    m_encoder.setArguments(m_encodeArguments[m_encodeAttempt]);
    m_encoder.start();
}

void ExportService::cancel()
{
    m_cancelled = true;
    if (!m_running)
        return;
    m_frameTimer.stop();
    if (m_encoder.state() != QProcess::NotRunning)
        m_encoder.kill();
    else
        finish(false, QStringLiteral("Export cancelled."));
}

void ExportService::finish(bool success, const QString& error)
{
    if (!m_running)
        return;
    m_frameTimer.stop();
    m_running = false;
    m_writers.waitForDone();
    {
        QMutexLocker lock(&m_writeMutex);
        m_writeError.clear();
    }
    const QString recovery = !success && m_movie ? m_frameFolder : QString{};
    if (success && m_ownedTemporaryDirectory)
        QDir(m_frameFolder).removeRecursively();
    m_ownedTemporaryDirectory = false;
    removeStagedMovie();
    m_render = {};
    emit finished(success, error, recovery);
}

bool ExportService::commitMovie(QString* error)
{
    const auto path = [](const QString& value) {
#ifdef Q_OS_WIN
        return std::filesystem::path(value.toStdWString());
#else
        const QByteArray bytes = value.toUtf8();
        return std::filesystem::u8path(bytes.constData(), bytes.constData() + bytes.size());
#endif
    };
    std::error_code failure;
    std::filesystem::rename(path(m_stagedMoviePath), path(m_request.outputPath), failure);
    if (failure) {
        m_keepStagedMovie = true;
        const std::string nativeError = failure.message();
        return failWith(error, QStringLiteral("Could not replace the final movie: %1\nThe encoded movie was preserved in:\n%2")
            .arg(QString::fromLocal8Bit(nativeError.c_str()), m_stagedMoviePath));
    }
    m_stagedMoviePath.clear();
    return true;
}

void ExportService::removeStagedMovie()
{
    if (!m_keepStagedMovie && !m_stagedMoviePath.isEmpty())
        QFile::remove(m_stagedMoviePath);
    m_stagedMoviePath.clear();
}

}
