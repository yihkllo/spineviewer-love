#include "archive_cache.h"
#include "asset_library.h"

#include <private/qzipreader_p.h>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSet>

#include <algorithm>

namespace slqt {
namespace {

const QString SourceMarker = QStringLiteral(".source");

QString& rootOverride()
{
    static QString value;
    return value;
}

struct Listing { QString key; QStringList models; QString error; };

QHash<QString, Listing>& listingCache()
{
    static QHash<QString, Listing> value;
    return value;
}

QHash<QString, QString>& sourceCache()
{
    static QHash<QString, QString> value;
    return value;
}

QString normalized(const QString& path)
{
    return path.isEmpty() ? QString{} : QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

bool wantedEntry(const QString& name)
{
    const QString lower = name.toLower();
    const QString file = lower.section(QLatin1Char('/'), -1);
    if (file.isEmpty() || file.startsWith(QLatin1String("._")) || file == QLatin1String(".ds_store") || file == QLatin1String("thumbs.db"))
        return false;
    if (lower.startsWith(QLatin1String("__macosx/")) || lower.contains(QLatin1String("/__macosx/")))
        return false;
    for (const char* suffix : {".json", ".skel", ".bytes", ".atlas", ".txt", ".png", ".jpg", ".jpeg", ".webp", ".bmp", ".tga"}) {
        if (file.endsWith(QLatin1String(suffix)))
            return true;
    }
    return false;
}

bool safeEntry(const QString& name)
{
    if (name.isEmpty() || name.startsWith(QLatin1Char('/')) || name.contains(QLatin1Char(':')))
        return false;
    const auto parts = name.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (const auto& part : parts) {
        if (part == QLatin1String(".."))
            return false;
    }
    return !parts.isEmpty();
}

QString archiveKey(const QFileInfo& info)
{
    const QByteArray seed = normalized(info.absoluteFilePath()).toLower().toUtf8() + '\n'
        + QByteArray::number(info.size()) + '\n'
        + QByteArray::number(info.lastModified().toMSecsSinceEpoch());
    return QString::fromLatin1(QCryptographicHash::hash(seed, QCryptographicHash::Sha1).toHex().left(16));
}

void touch(const QString& path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadWrite))
        file.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileModificationTime);
}

void prune(const QString& keep)
{
    QDir directory(ArchiveCache::root());
    struct Cached { QString path; QDateTime used; };
    QList<Cached> cached;
    for (const auto& entry : directory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (entry.fileName() == keep)
            continue;
        if (entry.fileName().endsWith(QLatin1String(".partial"))) {
            QDir(entry.absoluteFilePath()).removeRecursively();
            continue;
        }
        const QFileInfo marker(QDir(entry.absoluteFilePath()).filePath(SourceMarker));
        cached.append({entry.absoluteFilePath(), marker.exists() ? marker.lastModified() : QDateTime()});
    }
    std::sort(cached.begin(), cached.end(), [](const Cached& a, const Cached& b) { return a.used > b.used; });
    for (int i = ArchiveCache::MaximumArchives - 1; i < cached.size(); ++i) {
        sourceCache().remove(QFileInfo(cached[i].path).fileName());
        QDir(cached[i].path).removeRecursively();
    }
}

}

bool ArchiveCache::isArchive(const QString& path)
{
    return path.endsWith(QLatin1String(".zip"), Qt::CaseInsensitive);
}

QString ArchiveCache::root()
{
    if (!rootOverride().isEmpty())
        return rootOverride();
    QDir base(QCoreApplication::applicationDirPath());
    if (base.dirName() == QLatin1String("main"))
        base.cdUp();
    return QDir::cleanPath(base.filePath(QStringLiteral("cache/archives")));
}

void ArchiveCache::setRoot(const QString& root)
{
    rootOverride() = root.isEmpty() ? QString{} : QDir::cleanPath(root);
    sourceCache().clear();
}

QString ArchiveCache::extract(const QString& archivePath, QString* error)
{
    const auto fail = [error](const QString& message) {
        if (error)
            *error = message;
        return QString{};
    };
    const QFileInfo info(archivePath);
    if (!info.isFile())
        return fail(QStringLiteral("File does not exist: %1").arg(archivePath));
    const QString name = info.fileName();
    const QString base = root();
    if (!QDir().mkpath(base))
        return fail(QStringLiteral("Could not create the archive cache: %1").arg(base));
    const QString key = archiveKey(info);
    const QString target = QDir(base).filePath(key);
    const QString marker = QDir(target).filePath(SourceMarker);
    if (QFileInfo(marker).isFile()) {
        touch(marker);
        sourceCache().insert(key, normalized(info.absoluteFilePath()));
        return target;
    }
    QZipReader zip(info.absoluteFilePath());
    if (!zip.isReadable() || zip.status() != QZipReader::NoError)
        return fail(QStringLiteral("The archive is damaged or cannot be read: %1").arg(name));
    const auto entries = zip.fileInfoList();
    if (zip.status() != QZipReader::NoError)
        return fail(QStringLiteral("The archive is damaged or cannot be read: %1").arg(name));
    if (entries.size() > MaximumEntries)
        return fail(QStringLiteral("The archive contains too many files: %1").arg(name));
    QList<QZipReader::FileInfo> selected;
    qint64 total = 0;
    for (const auto& entry : entries) {
        if (!entry.isFile)
            continue;
        const QString entryName = QString(entry.filePath).replace(QLatin1Char('\\'), QLatin1Char('/'));
        if (!wantedEntry(entryName))
            continue;
        if (!safeEntry(entryName))
            return fail(QStringLiteral("The archive contains an unsafe file path: %1").arg(name));
        total += entry.size;
        if (total > MaximumBytes)
            return fail(QStringLiteral("The archive is too large to open: %1").arg(name));
        selected.append(entry);
    }
    if (selected.isEmpty())
        return fail(QStringLiteral("No Spine data was found in %1. Export JSON or binary skeleton data from Spine first.").arg(name));
    const QString partial = target + QStringLiteral(".partial");
    QDir(partial).removeRecursively();
    if (!QDir().mkpath(partial))
        return fail(QStringLiteral("Could not create the archive cache: %1").arg(base));
    const auto abandon = [&](const QString& message) {
        QDir(partial).removeRecursively();
        return fail(message);
    };
    for (const auto& entry : selected) {
        const QByteArray data = zip.fileData(entry.filePath);
        if (data.size() != entry.size)
            return abandon(QStringLiteral("The archive is password-protected or damaged: %1").arg(name));
        const QString entryName = QString(entry.filePath).replace(QLatin1Char('\\'), QLatin1Char('/'));
        const QString path = QDir(partial).filePath(entryName);
        if (!QDir().mkpath(QFileInfo(path).absolutePath()))
            return abandon(QStringLiteral("Could not create the archive cache: %1").arg(base));
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size())
            return abandon(QStringLiteral("Could not write the archive cache: %1").arg(base));
    }
    QFile source(QDir(partial).filePath(SourceMarker));
    if (!source.open(QIODevice::WriteOnly) || source.write(normalized(info.absoluteFilePath()).toUtf8()) < 0)
        return abandon(QStringLiteral("Could not write the archive cache: %1").arg(base));
    source.close();
    QDir(target).removeRecursively();
    if (!QDir().rename(partial, target))
        return abandon(QStringLiteral("Could not write the archive cache: %1").arg(base));
    sourceCache().insert(key, normalized(info.absoluteFilePath()));
    prune(key);
    return target;
}

QStringList ArchiveCache::listSpine(const QString& archivePath, QString* error)
{
    const QFileInfo info(archivePath);
    const QString archive = normalized(info.absoluteFilePath());
    const QString key = archiveKey(info);
    const auto cached = listingCache().constFind(archive);
    if (cached != listingCache().constEnd() && cached->key == key) {
        if (error)
            *error = cached->error;
        return cached->models;
    }
    Listing listing{key, {}, {}};
    QZipReader zip(archive);
    if (!info.isFile() || !zip.isReadable() || zip.status() != QZipReader::NoError) {
        listing.error = QStringLiteral("The archive is damaged or cannot be read: %1").arg(info.fileName());
    } else {
        const auto entries = zip.fileInfoList();
        if (zip.status() != QZipReader::NoError || entries.size() > MaximumEntries) {
            listing.error = QStringLiteral("The archive is damaged or cannot be read: %1").arg(info.fileName());
        } else {
            QHash<QString, QStringList> folders;
            for (const auto& entry : entries) {
                if (!entry.isFile)
                    continue;
                const QString name = QString(entry.filePath).replace(QLatin1Char('\\'), QLatin1Char('/'));
                if (!wantedEntry(name) || !safeEntry(name))
                    continue;
                const int slash = name.lastIndexOf(QLatin1Char('/'));
                folders[slash < 0 ? QString{} : name.left(slash)].append(name.mid(slash + 1));
            }
            for (auto folder = folders.cbegin(); folder != folders.cend(); ++folder) {
                QSet<QString> binaries;
                QStringList candidates;
                for (const QString& name : folder.value()) {
                    if (!AssetLibrary::isSpineFileName(name))
                        continue;
                    const QString stem = AssetLibrary::skeletonStem(name);
                    if (AssetLibrary::chooseAtlas(stem, folder.value()).isEmpty())
                        continue;
                    candidates.append(name);
                    if (!name.endsWith(QLatin1String(".json"), Qt::CaseInsensitive))
                        binaries.insert(stem.toLower());
                }
                for (const QString& name : candidates) {
                    if (name.endsWith(QLatin1String(".json"), Qt::CaseInsensitive) && binaries.contains(AssetLibrary::skeletonStem(name).toLower()))
                        continue;
                    listing.models.append(archive + QLatin1Char('/') + (folder.key().isEmpty() ? name : folder.key() + QLatin1Char('/') + name));
                }
            }
            AssetLibrary::naturalSort(listing.models);
            if (listing.models.isEmpty())
                listing.error = QStringLiteral("No Spine data was found in %1. Export JSON or binary skeleton data from Spine first.").arg(info.fileName());
        }
    }
    listingCache().insert(archive, listing);
    if (error)
        *error = listing.error;
    return listing.models;
}

QString ArchiveCache::archiveOf(const QString& path)
{
    const QString source = sourceArchive(path);
    if (!source.isEmpty())
        return source;
    const QString clean = QDir::cleanPath(path);
    int cursor = 0;
    while ((cursor = clean.indexOf(QLatin1Char('/'), cursor + 1)) > 0) {
        const QString prefix = clean.left(cursor);
        if (isArchive(prefix))
            return QFileInfo(prefix).isFile() ? prefix : QString{};
    }
    return isArchive(clean) && QFileInfo(clean).isFile() ? clean : QString{};
}

QString ArchiveCache::sourceArchive(const QString& path)
{
    const QString base = root() + QLatin1Char('/');
    const QString clean = normalized(path);
    if (!clean.startsWith(base, Qt::CaseInsensitive))
        return {};
    const QString key = clean.mid(base.size()).section(QLatin1Char('/'), 0, 0);
    if (key.isEmpty())
        return {};
    const auto cached = sourceCache().constFind(key);
    if (cached != sourceCache().constEnd())
        return cached.value();
    QFile marker(QDir(base + key).filePath(SourceMarker));
    if (!marker.open(QIODevice::ReadOnly))
        return {};
    const QString source = QString::fromUtf8(marker.readAll()).trimmed();
    if (!source.isEmpty())
        sourceCache().insert(key, source);
    return source;
}

QString ArchiveCache::displayPath(const QString& path)
{
    const QString source = sourceArchive(path);
    if (source.isEmpty())
        return path;
    const QString base = root() + QLatin1Char('/');
    const QString inner = normalized(path).mid(base.size()).section(QLatin1Char('/'), 1);
    return inner.isEmpty() ? source : source + QLatin1Char('/') + inner;
}

QString ArchiveCache::resolve(const QString& path, QString* error)
{
    if (path.isEmpty() || QFileInfo::exists(path))
        return path;
    const QString clean = QDir::cleanPath(path);
    int cursor = 0;
    while ((cursor = clean.indexOf(QLatin1Char('/'), cursor + 1)) > 0) {
        const QString prefix = clean.left(cursor);
        if (!isArchive(prefix))
            continue;
        if (!QFileInfo(prefix).isFile())
            break;
        const QString folder = extract(prefix, error);
        if (folder.isEmpty())
            return {};
        return QDir(folder).filePath(clean.mid(cursor + 1));
    }
    return path;
}

bool ArchiveCache::exists(const QString& path)
{
    if (QFileInfo::exists(path))
        return true;
    const QString clean = QDir::cleanPath(path);
    int cursor = 0;
    while ((cursor = clean.indexOf(QLatin1Char('/'), cursor + 1)) > 0) {
        const QString prefix = clean.left(cursor);
        if (isArchive(prefix))
            return QFileInfo(prefix).isFile();
    }
    return false;
}

}
