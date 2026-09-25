#pragma once
#include <QObject>
#include <QVariantMap>
#include <map>
#include <memory>

namespace slqt {
class PluginModule;
class PluginHost final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap state READ state NOTIFY stateChanged)
    Q_PROPERTY(QObject* module READ module NOTIFY moduleChanged)
public:
    explicit PluginHost(QObject* parent = nullptr);
    ~PluginHost() override;
    QVariantMap state() const;
    QObject* module() const;
    bool hasModules() const;
    bool isOpen() const { return m_selectorOpen || isActive(); }
    bool isActive() const { return !m_moduleKey.isEmpty(); }
    void setSuspended(bool suspended);
    void setHostSettings(const QVariantMap& settings);
    Q_INVOKABLE void dispatch(const QString& action, const QVariant& value = {});
signals:
    void stateChanged();
    void moduleChanged();
    void activeChanged();
    void failure(const QString& message);
    void settingsRequested(const QString& action,const QVariant& value);
private:
    PluginModule* ensureModule(const QString& id);
    PluginModule* activeModule()const;
    void close();
    std::map<QString, std::unique_ptr<PluginModule>> m_modules;
    QString m_moduleKey;
    QVariantMap m_hostSettings;
    bool m_selectorOpen = false;
    bool m_suspended = false;
    bool m_closing = false;
    bool m_resolvingSavedRoot = false;
};
}
