#pragma once
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

namespace slqt {
inline QString packagedAssetPath(const QString& relative)
{
    const QDir app(QCoreApplication::applicationDirPath());
    const QString packaged=app.filePath("ttf/"+relative);
    return QFileInfo::exists(packaged)?packaged:app.filePath(relative);
}
}
