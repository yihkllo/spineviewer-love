#include "spinelove/favorite_settings.h"
#include <QTemporaryDir>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QTemporaryDir temporary;
    if (!temporary.isValid()) return 1;
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, temporary.filePath("old"));
    QSettings legacy(QSettings::IniFormat, QSettings::UserScope, "FavoriteTest", "Preferences");
    legacy.setValue("language", "old");
    legacy.setValue("Library/one/favorites", QStringList{"stale"});
    legacy.sync();
    slqt::configureSettingsStorage(temporary.path());
    if (QSettings::defaultFormat() != QSettings::IniFormat) return 2;
    QString path;
    {
        slqt::FavoriteSettings settings(QSettings::defaultFormat(), QSettings::UserScope, "FavoriteTest", "Preferences");
        if (settings.contains("language") || settings.contains("Library/one/favorites")) return 3;
        if (!settings.fileName().startsWith(temporary.path()+"/settings/")) return 4;
        settings.setValue("language", "new");
        settings.setValue("window/width", 1280);
        settings.beginGroup("Library");
        settings.setValue("one/favorites", QStringList{"first", "second"});
        settings.setValue("two/privateFavorites", QStringList{"third"});
        path = settings.favoritesFileName();
        settings.sync();
    }
    if (path != temporary.filePath("favorites/favorites.ini")) return 5;
    {
        slqt::FavoriteSettings settings(QSettings::defaultFormat(), QSettings::UserScope, "FavoriteTest", "Preferences");
        if (settings.value("language").toString() != "new" || settings.value("window/width").toInt() != 1280) return 6;
        if (settings.value("Library/one/favorites").toStringList() != QStringList{"first", "second"}) return 7;
        settings.setValue("Library/one/favorites", QStringList{});
    }
    {
        slqt::FavoriteSettings settings(QSettings::defaultFormat(), QSettings::UserScope, "FavoriteTest", "Preferences");
        if (!settings.contains("Library/one/favorites") || !settings.value("Library/one/favorites").toStringList().isEmpty()) return 8;
        if (settings.value("Library/two/privateFavorites").toStringList() != QStringList{"third"}) return 9;
    }
    {
        QSettings progress(QSettings::defaultFormat(), QSettings::UserScope, "FavoriteTest", "Progress");
        progress.setValue("read/chapter", 42);progress.sync();
        if (!progress.fileName().startsWith(temporary.path()+"/settings/")) return 10;
    }
    QSettings progress(QSettings::defaultFormat(), QSettings::UserScope, "FavoriteTest", "Progress");
    if (progress.value("read/chapter").toInt() != 42) return 11;
    QSettings stored(path, QSettings::IniFormat);
    if (stored.contains("language") || stored.contains("window/width")) return 12;
    QTemporaryDir other;
    slqt::configureSettingsStorage(other.path());
    QSettings independent(QSettings::defaultFormat(), QSettings::UserScope, "FavoriteTest", "Preferences");
    if (independent.contains("language")) return 13;
    qInfo() << "Portable preferences, progress, favorites, restart and isolation checks passed";
    return 0;
}
