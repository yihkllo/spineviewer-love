#pragma once

#include <QString>
#include <QStringList>

namespace slqt {

class ArchiveCache final {
public:
    static constexpr qint64 MaximumBytes = 2048ll * 1024 * 1024;
    static constexpr int MaximumEntries = 20000;
    static constexpr int MaximumArchives = 16;

    static bool isArchive(const QString& path);
    static QString extract(const QString& archivePath, QString* error = nullptr);
    static QStringList listSpine(const QString& archivePath, QString* error = nullptr);
    static QStringList listLive2D(const QString& archivePath);
    static QString sourceArchive(const QString& path);
    static QString archiveOf(const QString& path);
    static QString displayPath(const QString& path);
    static QString resolve(const QString& path, QString* error = nullptr);
    static bool exists(const QString& path);
    static QString root();
    static void setRoot(const QString& root);
};

}
