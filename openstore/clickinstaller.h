#ifndef CLICKINSTALLER_H
#define CLICKINSTALLER_H

#include <QObject>
#include <QProcess>
#include <QFile>

class QNetworkReply;
class QNetworkAccessManager;

class ClickInstaller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool isLocalInstall READ isLocalInstall NOTIFY isLocalInstallChanged)
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)

public:
    explicit ClickInstaller(QObject *parent = 0);

    bool busy() const;
    bool isLocalInstall() const;
    int downloadProgress() const;

Q_SIGNALS:
    void busyChanged();
    void isLocalInstallChanged();
    void downloadProgressChanged();
    void packageInstalled();
    void packageInstallationFailed();

public Q_SLOTS:
    Q_INVOKABLE void installPackage(const QString &packageUrl, const bool isLocalInstall = false);
    Q_INVOKABLE void removePackage(const QString &appId, const QString &version);
    bool abortInstallation() const;

private:
    void fetchPackage(const QString &packageUrl);
    void installLocalPackage(const QString &packageFilePath);

private Q_SLOTS:
    void installerFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processStatusChanged(QProcess::ProcessState state);

    void slotDownloadProgress();
    void downloadFinished();

private:
    QProcess* m_installerProcess;
    QNetworkReply *m_download;

    QNetworkAccessManager *m_nam;
    QFile m_file;

    bool m_isLocalInstall;
};

#endif // CLICKINSTALLER_H
