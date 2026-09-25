#include "asset_library.h"
#include "spine_json_preflight.h"

#include "spinelove/sl_skeleton_probe.h"

#include <QCollator>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>

#include <algorithm>

namespace slqt {
namespace {

QString cleanLocalPath(const QString& path)
{
    return path.isEmpty() ? QString{} : QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

bool readBytes(const QString& path, QByteArray& bytes, QString& error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QStringLiteral("Could not read %1: %2").arg(path, file.errorString());
        return false;
    }
    bytes = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        error = QStringLiteral("Could not read %1: %2").arg(path, file.errorString());
        bytes.clear();
        return false;
    }
    return true;
}

AssetEntry inspectSpine(const QString& path, const QByteArray& bytes)
{
    AssetEntry entry;
    entry.path = cleanLocalPath(path);
    const QFileInfo file(entry.path);
    entry.displayName = file.completeBaseName();
    entry.textureDirectory = file.absolutePath() + QLatin1Char('/');
    entry.atlasPath = AssetLibrary::matchingAtlas(entry.path);
    if (entry.atlasPath.isEmpty()) {
        entry.error = QStringLiteral("Atlas file not found for: %1").arg(entry.displayName);
        return entry;
    }
    const auto probe = sl_skeleton_probe::Inspect(
        reinterpret_cast<const unsigned char*>(bytes.constData()),
        static_cast<size_t>(bytes.size()));
    if (!probe.IsSpineSkeleton()) {
        entry.error = QStringLiteral("This seems not to be valid Spine skeleton file.");
        return entry;
    }
    entry.spineVersion = QString::fromUtf8(probe.version.data(), static_cast<qsizetype>(probe.version.size()));
    entry.kind = probe.kind == sl_skeleton_probe::FileKind::Binary
        ? AssetKind::SpineBinary : AssetKind::SpineJson;
    return entry;
}

QStringList scan(const QString& folder, bool live2d)
{
    if (folder.isEmpty() || !QFileInfo(folder).isDir())
        return {};
    struct PendingDirectory { QString path; int depth; };
    QList<PendingDirectory> pending{{cleanLocalPath(folder), 0}};
    QStringList result;
    const int maximumDepth = live2d ? AssetLibrary::Live2DMaximumDepth : AssetLibrary::SpineMaximumDepth;
    while (!pending.isEmpty()) {
        const auto current = pending.takeLast();
        const auto entries = QDir(current.path).entryInfoList(
            QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot,
            QDir::NoSort);
        for (const auto& entry : entries) {
            if (entry.isDir()) {
                if (current.depth < maximumDepth && !entry.isSymLink())
                    pending.append({entry.absoluteFilePath(), current.depth + 1});
                continue;
            }
            if (!entry.isFile())
                continue;
            const QString path = entry.absoluteFilePath();
            if (live2d ? AssetLibrary::isLive2DFileName(path)
                       : AssetLibrary::isSpineFileName(path) && !AssetLibrary::matchingAtlas(path).isEmpty())
                result.append(path);
        }
    }
    AssetLibrary::naturalSort(result);
    return result;
}

}

QString AssetLibrary::localPath(const QUrl& url, QString* error)
{
    if (error)
        error->clear();
    if (url.isLocalFile())
        return cleanLocalPath(url.toLocalFile());
    if (url.isRelative() && !url.toString().isEmpty())
        return cleanLocalPath(url.toString(QUrl::FullyDecoded));
    if (error)
        *error = QStringLiteral("This document URL requires the platform import service: %1").arg(url.scheme());
    return {};
}

bool AssetLibrary::isSpineFileName(const QString& path)
{
    return path.endsWith(QLatin1String(".json"), Qt::CaseInsensitive)
        || path.endsWith(QLatin1String(".skel"), Qt::CaseInsensitive);
}

bool AssetLibrary::isLive2DFileName(const QString& path)
{
    return path.endsWith(QLatin1String(".model3.json"), Qt::CaseInsensitive);
}

QString AssetLibrary::matchingAtlas(const QString& skeletonPath)
{
    if (skeletonPath.isEmpty())
        return {};
    const QFileInfo skeleton(skeletonPath);
    const QDir directory(skeleton.absolutePath());
    const QString stem = skeleton.completeBaseName();
    QStringList names;
    for (const auto& suffix : {QStringLiteral(".atlas"), QStringLiteral(".atlas.txt")}) {
        const QString wanted = stem + suffix;
        const QString exact = directory.filePath(wanted);
        if (QFileInfo(exact).isFile())
            return QDir::cleanPath(exact);
        if (names.isEmpty())
            names = directory.entryList(QDir::Files | QDir::Hidden | QDir::System, QDir::Name);
        for (const QString& name : names) {
            if (QString::compare(name, wanted, Qt::CaseInsensitive) == 0)
                return QDir::cleanPath(directory.filePath(name));
        }
    }
    return {};
}

AssetEntry AssetLibrary::inspect(const QString& path)
{
    AssetEntry entry;
    entry.path = cleanLocalPath(path);
    const QFileInfo file(entry.path);
    entry.displayName = file.completeBaseName();
    entry.textureDirectory = file.absolutePath() + QLatin1Char('/');
    if (path.isEmpty() || !file.isFile()) {
        entry.error = QStringLiteral("File does not exist: %1").arg(path);
        return entry;
    }
    if (isLive2DFileName(entry.path)) {
        entry.kind = AssetKind::Live2D;
        return entry;
    }
    QByteArray bytes;
    if (!readBytes(entry.path, bytes, entry.error))
        return entry;
    return inspectSpine(entry.path, bytes);
}

AssetBundle AssetLibrary::readSpineBundle(const QStringList& paths)
{
    AssetBundle bundle;
    if (paths.isEmpty()) {
        bundle.error = QStringLiteral("No Spine file pair was provided.");
        return bundle;
    }
    const auto fail = [](const QString& message) {
        AssetBundle result;
        result.error = message;
        return result;
    };
    for (const auto& path : paths) {
        if (isLive2DFileName(path))
            return fail(QStringLiteral("Please select Spine skeleton files for a Spine bundle."));
        QByteArray skeletonBytes;
        QString error;
        if (!readBytes(path, skeletonBytes, error))
            return fail(error);
        const AssetEntry entry = inspectSpine(path, skeletonBytes);
        if (!entry.isValid())
            return fail(entry.error);
        const bool binary = entry.kind == AssetKind::SpineBinary;
        if (!binary) {
            if (!SpineJsonPreflight::valid(skeletonBytes))
                return fail(QStringLiteral("Spine JSON has no bone data: %1").arg(path));
        }
        if (!bundle.items.isEmpty() && bundle.binarySkeleton != binary)
            return fail(QStringLiteral("Please open either binary .skel files or JSON files together, not both."));
        if (!bundle.items.isEmpty() && bundle.items.front().spineVersion != entry.spineVersion)
            return fail(QStringLiteral("Please open Spine files from the same runtime version together."));
        QByteArray atlasBytes;
        if (!readBytes(entry.atlasPath, atlasBytes, error))
            return fail(error);
        bundle.binarySkeleton = binary;
        bundle.items.append(entry);
        bundle.atlasBytes.append(atlasBytes);
        bundle.skeletonBytes.append(skeletonBytes);
        bundle.textureDirectories.append(entry.textureDirectory);
    }
    return bundle;
}

QStringList AssetLibrary::scanSpine(const QString& folder) { return scan(folder, false); }
QStringList AssetLibrary::scanLive2D(const QString& folder) { return scan(folder, true); }

QStringList AssetLibrary::siblingFolders(const QString& folder)
{
    if (folder.isEmpty())
        return {};
    const QDir parent(QFileInfo(QDir::cleanPath(folder)).absolutePath());
    QStringList result;
    for (const auto& entry : parent.entryInfoList(QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDir::NoSort))
        result.append(entry.absoluteFilePath());
    naturalSort(result);
    return result;
}

QStringList AssetLibrary::filesInDirectory(const QString& folder, const QStringList& filters)
{
    if (folder.isEmpty() || !QFileInfo(folder).isDir())
        return {};
    QStringList result;
    for (const auto& entry : QDir(folder).entryInfoList(QDir::Files | QDir::Hidden | QDir::System, QDir::NoSort)) {
        if (filters.isEmpty() || QDir::match(filters, entry.fileName()))
            result.append(entry.absoluteFilePath());
    }
    naturalSort(result);
    return result;
}

void AssetLibrary::naturalSort(QStringList& paths)
{
    QCollator collator(QLocale::system());
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(paths.begin(), paths.end(), [&collator](const QString& left, const QString& right) {
        int compared = collator.compare(QFileInfo(left).fileName(), QFileInfo(right).fileName());
        if (compared == 0)
            compared = collator.compare(left, right);
        if (compared == 0)
            compared = QString::compare(left, right, Qt::CaseSensitive);
        return compared < 0;
    });
}

}
