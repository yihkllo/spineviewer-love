#include "plugin_host.h"
#include "spinelove/plugin_registry.h"
#include "spinelove/plugin_module.h"
#include <QCoreApplication>
#include <QFileDialog>

namespace slqt {
PluginHost::PluginHost(QObject* parent) : QObject(parent) {}
PluginHost::~PluginHost() { if (auto* module=activeModule())module->deactivate(); }
QVariantMap PluginHost::state() const {
    QVariantList modules;
    for (const auto& info : PluginRegistry::modules())
        modules.append(QVariantMap{{"id", info.id}, {"name", info.name}, {"available", true}});
    const auto* active = PluginRegistry::find(m_moduleKey);
    return {{"selectorOpen", m_selectorOpen}, {"moduleKey", m_moduleKey}, {"modules", modules},
            {"view", active ? active->view.toString() : QString()}, {"portrait", active && active->portrait}};
}
bool PluginHost::hasModules() const { return !PluginRegistry::modules().empty(); }
PluginModule* PluginHost::activeModule()const{
    const auto found=m_modules.find(m_moduleKey);
    return found==m_modules.end()?nullptr:found->second.get();
}
QObject* PluginHost::module() const { return activeModule(); }
PluginModule* PluginHost::ensureModule(const QString& id) {
    auto& owned=m_modules[id];
    if (!owned) {
        owned=PluginRegistry::find(id)->create();
        connect(owned.get(), &PluginModule::failure, this, [this](const QString& message) {
            if (!m_resolvingSavedRoot) emit failure(message);
        });
        connect(owned.get(), &PluginModule::exited, this, &PluginHost::close);
        auto* source=owned.get();
        connect(source,&PluginModule::settingsRequested,this,[this,source](const QString& action,const QVariant& value){
            if(source==activeModule())emit settingsRequested(action,value);
        });
        source->setHostSettings(m_hostSettings);
    }
    return owned.get();
}
void PluginHost::setHostSettings(const QVariantMap& settings){
    if(m_hostSettings==settings)return;
    m_hostSettings=settings;
    for(const auto& [id,module]:m_modules)if(module)module->setHostSettings(settings);
}
void PluginHost::setSuspended(bool suspended) {
    m_suspended = suspended;
    if (auto* module=activeModule()) module->setSuspended(suspended || m_selectorOpen);
}
void PluginHost::close() {
    if (m_closing) return;
    m_closing = true;
    const bool wasActive = isActive(), wasOpen = isOpen();
    if (auto* module=activeModule()) module->deactivate();
    m_moduleKey.clear(); m_selectorOpen = false;
    m_closing = false;
    if (wasActive) { emit moduleChanged(); emit activeChanged(); }
    if (wasOpen) emit stateChanged();
}
void PluginHost::dispatch(const QString& action, const QVariant& value) {
    if (action == "open") {
        if (!m_selectorOpen) {
            m_selectorOpen = true;
            if (auto* module=activeModule()) module->setSuspended(true);
            emit stateChanged();
        }
        return;
    }
    if (action == "dismiss") {
        if (m_selectorOpen) {
            m_selectorOpen = false;
            if (auto* module=activeModule()) module->setSuspended(m_suspended);
            emit stateChanged();
        }
        return;
    }
    if (action == "close") { close(); return; }
    if (action != "activate") return;
    const auto request = value.toMap();
    const QString id = request.value("id").toString();
    const auto* info = PluginRegistry::find(id);
    if (!info) { emit failure(tr("Unknown plugin module.")); return; }
    auto* selected = ensureModule(id);
    QString root = request.value("root").toString();
    const bool fromSettings = root.isEmpty();
    if (fromSettings) root = selected->preferredRoot();
    const bool hasSavedRoot = fromSettings && !root.isEmpty();
    const QString prompt = QCoreApplication::translate("PluginHost", info->rootPrompt.toUtf8().constData());
    const auto chooseRoot = [&] { return QFileDialog::getExistingDirectory(nullptr, prompt); };
    if (root.isEmpty()) root = chooseRoot();
    if (root.isEmpty()) return;
    const bool same=selected==activeModule();
    selected->setSuspended(true);
    m_resolvingSavedRoot = hasSavedRoot;
    bool activated = selected->activate(root);
    m_resolvingSavedRoot = false;
    if (!activated && hasSavedRoot) {
        root = chooseRoot();
        if (!root.isEmpty()) activated = selected->activate(root);
    }
    if (!activated){if(same)selected->setSuspended(m_suspended||m_selectorOpen);return;}
    const bool wasActive = isActive();
    if(auto* previous=activeModule();previous&&previous!=selected)previous->deactivate();
    m_moduleKey = id; m_selectorOpen = false;
    selected->setSuspended(m_suspended);
    emit moduleChanged();
    if (!wasActive) emit activeChanged();
    emit stateChanged();
}
}
