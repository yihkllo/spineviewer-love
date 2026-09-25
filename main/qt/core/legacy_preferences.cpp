#include "legacy_preferences.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringConverter>
#include <algorithm>

namespace slqt {
QString readLegacyText(const QString& path){
    QFile file(path);if(!file.open(QIODevice::ReadOnly))return {};
    QTextStream text(&file);text.setEncoding(QStringConverter::Utf8);text.setAutoDetectUnicode(true);
    return text.readAll();
}
QStringList readLegacyFavorites(const QString& path){
    QStringList favorites;
    for(auto row:readLegacyText(path).split('\n')){
        if(row.endsWith('\r'))row.chop(1);
        if(row.isEmpty())continue;
        row=QDir::cleanPath(row);
        if(QFileInfo::exists(row)&&!favorites.contains(row))favorites.append(row);
    }
    std::sort(favorites.begin(),favorites.end());return favorites;
}
}
