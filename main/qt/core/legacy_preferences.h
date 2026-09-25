#pragma once
#include <QStringList>
namespace slqt {
QString readLegacyText(const QString& path);
QStringList readLegacyFavorites(const QString& path);
}
