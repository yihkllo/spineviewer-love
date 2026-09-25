#pragma once
#include <QString>
#include <QStringList>
#include <QUrl>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include "spinelove/sdk_api.h"

namespace slqt {
class PluginModule;

struct PluginInfo {
    QString id;
    QString name;
    QString rootPrompt;
    QUrl view;
    int order=0;
    bool portrait=false;
    std::function<std::unique_ptr<PluginModule>()> create;
};

namespace PluginRegistry {
SL_SDK_API void addModule(PluginInfo info);
SL_SDK_API const std::vector<PluginInfo>& modules();
SL_SDK_API const PluginInfo* find(const QString& id);
SL_SDK_API void addStartup(std::function<void()> hook);
SL_SDK_API void runStartup();
SL_SDK_API void addCommand(std::function<std::optional<int>(const QStringList&)> hook);
SL_SDK_API std::optional<int> runCommand(const QStringList& arguments);
}
}
