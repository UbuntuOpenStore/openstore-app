#ifndef PLATFORMINTEGRATION_H
#define PLATFORMINTEGRATION_H

#include <QObject>
#include <QHash>
#include <QVariantList>

class ClickInstaller;
class ServiceRegistry;

class PlatformIntegration : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ClickInstaller* clickInstaller READ clickInstaller CONSTANT)

public:
    PlatformIntegration();
    ~PlatformIntegration();

    static PlatformIntegration* instance();

    ClickInstaller* clickInstaller() const { return m_installer; }

    QStringList supportedFrameworks() const { return m_supportedFrameworks; }
    QString supportedArchitecture() const { return m_supportedArchitecture; }
    QString systemLocale() const { return m_systemLocale; }
    QString systemCodename() const { return m_systemCodename; }

    QString appVersion(const QString &appId) const { return m_installedAppIds.value(appId, QString()); }
    QStringList installedAppIds() const { return m_installedAppIds.keys(); }

    QVariantList clickDb() const { return m_clickDb; }

Q_SIGNALS:
    void updated();

public Q_SLOTS:
    void update();

private:
    QStringList getSupportedFrameworks();
    QString getSupportedArchitecture();
    QString getSystemLocale();
    QString getSystemCodename();

private:
    QStringList m_supportedFrameworks;
    QString m_supportedArchitecture;
    QString m_systemLocale;
    QString m_systemCodename;

    QHash<QString, QString> m_installedAppIds; // appid, version
    QVariantList m_clickDb;

    ClickInstaller* m_installer;
    ServiceRegistry* m_serviceRegistry;
};

#endif // PLATFORMINTEGRATION_H
