#pragma once
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include "spinelove/settings_storage.h"

namespace slqt {
class FavoriteSettings : public QSettings {
public:
    FavoriteSettings(Format format, Scope scope, const QString& organization, const QString& application)
        : QSettings(format, scope, organization, application), m_favorites(storagePath(), QSettings::IniFormat) {
        m_favorites.setFallbacksEnabled(false);

    }
    QVariant value(const QString& key, const QVariant& fallback = {}) const {
        if (!isFavorite(key)) return QSettings::value(key, fallback);
        const auto full = fullKey(key);
        return m_favorites.value(full, fallback);
    }
    bool contains(const QString& key) const {
        return isFavorite(key) ? m_favorites.contains(fullKey(key)) : QSettings::contains(key);
    }
    void setValue(const QString& key, const QVariant& value) {
        if (!isFavorite(key)) { QSettings::setValue(key, value); return; }
        m_favorites.setValue(fullKey(key), value);
        m_favorites.sync();
        if (m_favorites.status() != QSettings::NoError) qWarning() << "Could not save favorites to" << m_favorites.fileName();
    }
    void sync() { m_favorites.sync(); QSettings::sync(); }
    QString favoritesFileName() const { return m_favorites.fileName(); }
private:
    static bool isFavorite(const QString& key) { return key.section('/', -1).endsWith("favorites", Qt::CaseInsensitive); }
    QString fullKey(const QString& key) const { return group().isEmpty() ? key : group() + '/' + key; }
    QString storagePath() const {
        return QDir(settingsStorageRoot(this)).filePath("favorites/favorites.ini");
    }
    mutable QSettings m_favorites;
};
}
