#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

#include <QObject>
#include <QFileSystemWatcher>

#include "clickinstaller.h"

class ServiceRegistry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ClickInstaller* clickInstaller READ clickInstaller WRITE setClickInstaller NOTIFY clickInstallerChanged)

public:
    explicit ServiceRegistry(QObject *parent = 0);

    ClickInstaller* clickInstaller() const;
    void setClickInstaller(ClickInstaller *clickInstaller);


Q_SIGNALS:
    void clickInstallerChanged();

private Q_SLOTS:
    void installerBusyChanged();
    void clickDirChanged();

private:
    bool upstartServiceExists(const QString &appId);

    void createServiceFile(const QString &appId, const QString &desktopFile);

private:
    ClickInstaller *m_clickInstaller;
    QFileSystemWatcher m_watcher;
};

#endif // SERVICEREGISTRY_H
