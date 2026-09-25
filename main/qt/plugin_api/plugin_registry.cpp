#include "spinelove/plugin_registry.h"
#include "spinelove/plugin_module.h"
#include <algorithm>

namespace slqt::PluginRegistry {
namespace {
std::vector<PluginInfo>& moduleList(){static std::vector<PluginInfo> list;return list;}
std::vector<std::function<void()>>& startupHooks(){static std::vector<std::function<void()>> hooks;return hooks;}
std::vector<std::function<std::optional<int>(const QStringList&)>>& commandHooks(){
    static std::vector<std::function<std::optional<int>(const QStringList&)>> hooks;return hooks;
}
}
void addModule(PluginInfo info){
    auto& list=moduleList();
    list.erase(std::remove_if(list.begin(),list.end(),[&](const PluginInfo& row){return row.id==info.id;}),list.end());
    list.push_back(std::move(info));
    std::stable_sort(list.begin(),list.end(),[](const PluginInfo& a,const PluginInfo& b){return a.order<b.order;});
}
const std::vector<PluginInfo>& modules(){return moduleList();}
const PluginInfo* find(const QString& id){
    for(const auto& info:moduleList())if(info.id==id)return &info;
    return nullptr;
}
void addStartup(std::function<void()> hook){startupHooks().push_back(std::move(hook));}
void runStartup(){for(const auto& hook:startupHooks())hook();}
void addCommand(std::function<std::optional<int>(const QStringList&)> hook){commandHooks().push_back(std::move(hook));}
std::optional<int> runCommand(const QStringList& arguments){
    for(const auto& hook:commandHooks())if(const auto code=hook(arguments))return code;
    return std::nullopt;
}
}
