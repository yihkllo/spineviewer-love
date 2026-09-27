#pragma once
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QVariant>

namespace slqt {
inline QString settingsStorageRoot(const QSettings* settings = nullptr) {
    const auto configured = QCoreApplication::instance()->property("settingsStorageRoot").toString();
    if (!configured.isEmpty()) return configured;
    if (settings && settings->format() == QSettings::IniFormat) return QFileInfo(settings->fileName()).absolutePath();
    QDir directory(QCoreApplication::applicationDirPath());
    if (directory.dirName().compare("main", Qt::CaseInsensitive) == 0) directory.cdUp();
    return directory.absolutePath();
}
inline void configureSettingsStorage(const QString& root = {}) {
    const auto resolved = root.isEmpty() ? settingsStorageRoot() : QDir(root).absolutePath();
    QCoreApplication::instance()->setProperty("settingsStorageRoot", resolved);
    const auto directory = QDir(resolved).filePath("settings");
    QDir().mkpath(directory);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory);
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, directory);
}
}
