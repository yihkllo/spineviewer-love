#include "plugin_loader.h"
#include "spinelove/sdk_api.h"
#include <QDebug>
#include <QDir>
#include <QLibrary>

namespace slqt {
QStringList loadPlugins(const QString& directory){
    QStringList loaded;
    const QDir dir(directory);
    for(const auto& file:dir.entryInfoList({QStringLiteral("*.dll")},QDir::Files,QDir::Name)){
        auto* library=new QLibrary(file.absoluteFilePath());
        if(!library->load()){
            qWarning().noquote()<<"Plugin not loaded:"<<file.fileName()<<library->errorString();
            delete library;continue;
        }
        using Version=int(*)();using Register=void(*)();
        const auto version=reinterpret_cast<Version>(library->resolve(SL_PLUGIN_API_SYMBOL));
        const auto registerModules=reinterpret_cast<Register>(library->resolve(SL_PLUGIN_REGISTER_SYMBOL));
        if(!version||!registerModules||version()!=SL_PLUGIN_API_VERSION){
            qWarning().noquote()<<"Plugin skipped (not a SpineLove plugin or interface version mismatch):"<<file.fileName();
            library->unload();delete library;continue;
        }
        registerModules();
        qInfo().noquote()<<"Plugin loaded:"<<file.fileName();
        loaded.append(file.fileName());
    }
    return loaded;
}
}
