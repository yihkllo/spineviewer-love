#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace slqt {

enum class AssetKind { Unknown, SpineJson, SpineBinary, Live2D };

struct AssetEntry {
    QString path;
    QString atlasPath;
    QString textureDirectory;
    QString displayName;
    QString spineVersion;
    AssetKind kind = AssetKind::Unknown;
    QString error;

    bool isSpine() const noexcept
    { return kind == AssetKind::SpineJson || kind == AssetKind::SpineBinary; }
    bool isValid() const noexcept { return kind != AssetKind::Unknown && error.isEmpty(); }
};

struct AssetBundle {
    QList<AssetEntry> items;
    QList<QByteArray> atlasBytes;
    QList<QByteArray> skeletonBytes;
    QStringList textureDirectories;
    bool binarySkeleton = false;
    QString error;

    bool isValid() const noexcept { return !items.isEmpty() && error.isEmpty(); }
};

class AssetLibrary final {
public:
    static constexpr int SpineMaximumDepth = 7;
    static constexpr int Live2DMaximumDepth = 12;

    static QString localPath(const QUrl& url, QString* error = nullptr);
    static bool isSpineFileName(const QString& path);
    static bool isJsonSkeletonName(const QString& path);
    static bool isUnityComponentFile(const QString& path, qint64 size);
    static bool isLive2DFileName(const QString& path);
    static QString skeletonStem(const QString& path);
    static QString chooseAtlas(const QString& stem, const QStringList& names);
    static QString matchingAtlas(const QString& skeletonPath);
    static AssetEntry inspect(const QString& path);
    static AssetBundle readSpineBundle(const QStringList& paths);

    static QStringList scanSpine(const QString& folder);
    static QStringList scanLive2D(const QString& folder);
    static QStringList siblingFolders(const QString& folder);
    static QStringList filesInDirectory(const QString& folder, const QStringList& filters);
    static void naturalSort(QStringList& paths);
};

}
