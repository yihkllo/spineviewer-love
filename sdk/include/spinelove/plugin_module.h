#pragma once
#include <QObject>
#include <QVariantMap>
#include "spinelove/sdk_api.h"

namespace slqt {
class SL_SDK_API PluginModule : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap state READ state NOTIFY stateChanged)
    Q_PROPERTY(QObject* media READ media CONSTANT)
    Q_PROPERTY(QVariantMap hostSettings READ hostSettings NOTIFY hostSettingsChanged)
    Q_PROPERTY(QVariantMap titleState READ titleState NOTIFY titleStateChanged)
public:
    using QObject::QObject;
    virtual QVariantMap state() const=0;
    virtual QVariantMap titleState() const{return {};}
    virtual QObject* media() const=0;
    virtual QString preferredRoot() const=0;
    virtual bool activate(const QString& root)=0;
    virtual void deactivate()=0;
    virtual void setSuspended(bool suspended)=0;
    QVariantMap hostSettings()const{return m_hostSettings;}
    virtual void setHostSettings(const QVariantMap& settings){
        if(m_hostSettings==settings)return;
        m_hostSettings=settings;emit hostSettingsChanged();
    }
    Q_INVOKABLE virtual void dispatch(const QString& action,const QVariant& value={})=0;
signals:
    void stateChanged();
    void titleStateChanged();
    void exited();
    void failure(const QString& message);
    void hostSettingsChanged();
    void settingsRequested(const QString& action,const QVariant& value);
private:
    QVariantMap m_hostSettings;
};
}
