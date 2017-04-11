#include "serviceregistry.h"

#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QSettings>
#include <QDir>

const QString clickRoot = "/opt/click.ubuntu.com/";

ServiceRegistry::ServiceRegistry(QObject *parent) :
    QObject(parent),
    m_clickInstaller(0)
{
    //m_watcher.addPath(clickRoot);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &ServiceRegistry::clickDirChanged);
}

ClickInstaller *ServiceRegistry::clickInstaller() const
{
    return m_clickInstaller;
}

void ServiceRegistry::setClickInstaller(ClickInstaller *clickInstaller)
{
    if (m_clickInstaller != clickInstaller) {
        if (m_clickInstaller) {
            disconnect(m_clickInstaller, &ClickInstaller::busyChanged, this, &ServiceRegistry::installerBusyChanged);
        }
        m_clickInstaller = clickInstaller;
        Q_EMIT clickInstallerChanged();
        connect(m_clickInstaller, &ClickInstaller::packageInstalled, this, &ServiceRegistry::installerBusyChanged);
    }
}

void ServiceRegistry::installerBusyChanged()
{
    clickDirChanged();
}

void ServiceRegistry::clickDirChanged()
{
    QDir clickDir(clickRoot);
    Q_FOREACH (const QString &dir, clickDir.entryList()) {
        QString appId = dir;
        QString infoDirPath = clickRoot + dir + "/current/.click/info/";
        QDir infoDir(infoDirPath);

        if (infoDir.entryInfoList(QStringList() << "*.manifest").count() == 0) {
            qDebug() << "No manifest file in" << infoDirPath;
            continue;
        }

        QString manifestFile = infoDir.entryList(QStringList() << "*.manifest").first();

        QFile f(infoDirPath + "/" + manifestFile);
        if (!f.open(QFile::ReadOnly)) {
            qDebug() << "failed to open manifest file" << manifestFile;
            continue;
        }
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(f.readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qDebug() << "failed to parse manifest file" << error.errorString();
            continue;
        }
        QVariantMap manifest = jsonDoc.toVariant().toMap();
        if (!manifest.contains("hooks") || !manifest.value("hooks").toMap().contains("service")) {
            continue;
        }
        QString desktopFile = manifest.value("hooks").toMap().value("service").toMap().value("desktop").toString();
        if (desktopFile.isEmpty()) {
            qDebug() << "No desktop entry in service hook";
            continue;
        }

        if (upstartServiceExists(appId)) {
            qDebug() << "already have service installed";
            continue;
        }

        qDebug() << "should install service file for" << appId;
        createServiceFile(appId, desktopFile);
    }
}

bool ServiceRegistry::upstartServiceExists(const QString &appId)
{
    QString homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    QFile f(homeDir + "/.config/upstart/" + appId + ".conf");
    return f.exists();
}

void ServiceRegistry::createServiceFile(const QString &appId, const QString &desktopFile)
{
    QSettings d(clickRoot + appId + "/current/" + desktopFile, QSettings::IniFormat);
    qDebug() << "desktopFile" << clickRoot + appId + "/" + desktopFile;
    d.beginGroup("Desktop Entry");
    QString exec = d.value("Exec").toString();
    qDebug() << "have exec" << exec;

    QString homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    QFile serviceFile(homeDir + "/.config/upstart/" + appId + ".conf");

    if (!serviceFile.open(QFile::WriteOnly)) {
        qDebug() << "Error creating service file";
        return;
    }
    serviceFile.write("#openstore\n");
    serviceFile.write("start on started unity8\n");
    serviceFile.write("pre-start script\n");
    serviceFile.write(QString("  initctl set-env LD_LIBRARY_PATH=%1\n").arg(clickRoot + appId + "/current/lib/arm-linux-gnueabihf").toLatin1());
    serviceFile.write("end script\n");
    serviceFile.write("task\n");
    serviceFile.write(QString("exec %1\n").arg(clickRoot + appId + "/current/lib/arm-linux-gnueabihf/bin/" + exec).toLatin1());
    serviceFile.close();
}
