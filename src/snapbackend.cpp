/*
 * Copyright (C) 2026 Brian Douglass
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "snapbackend.h"

#include <QDebug>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>

SnapBackend::SnapBackend(QObject* parent)
  : PackageBackend(parent)
  , m_installerProcess(nullptr)
  , m_download(nullptr)
  , m_isLocalInstall(false)
{
  m_nam = new QNetworkAccessManager(this);
}

bool SnapBackend::busy() const
{
  return m_installerProcess != nullptr || m_download != nullptr;
}

bool SnapBackend::isLocalInstall() const
{
  return m_isLocalInstall;
}

int SnapBackend::downloadProgress() const
{
  if (m_file.isOpen()) {
    return m_file.size();
  }
  return 0;
}

bool SnapBackend::isAvailable() const
{
  // Check if snap command is available
  QProcess process;
  process.start("which", QStringList() << "snap");
  process.waitForFinished(1000);
  return process.exitCode() == 0;
}

QList<InstalledPackageInfo> SnapBackend::getInstalledPackages()
{
  QList<InstalledPackageInfo> packages;

  if (!isAvailable()) {
    return packages;
  }

  QProcess process;
  process.start("snap",
                QStringList() << "list"
                              << "--color=never");
  process.waitForFinished(5000);

  if (process.exitCode() != 0) {
    qWarning() << "Failed to list snaps:" << process.errorString();
    return packages;
  }

  QString output = QString::fromUtf8(process.readAllStandardOutput());
  QStringList lines = output.split('\n', QString::SkipEmptyParts);

  // Skip header line
  for (int i = 1; i < lines.size(); i++) {
    QString line = lines[i];
    // Format: Name Version Rev Tracking Publisher Notes
    QStringList parts = line.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);

    if (parts.size() >= 2) {
      InstalledPackageInfo info;
      info.appId = parts[0];
      info.version = parts[1];
      info.packageType = "snap";

      // Build a simple manifest
      QVariantMap manifest;
      manifest["name"] = info.appId;
      manifest["version"] = info.version;
      if (parts.size() >= 5) {
        manifest["publisher"] = parts[4];
      }
      info.manifest = manifest;

      packages.append(info);
    }
  }

  return packages;
}

void SnapBackend::installPackage(const QString& packageUrl, const bool isLocalInstall)
{
  m_isLocalInstall = isLocalInstall;
  Q_EMIT isLocalInstallChanged();

  if (busy()) {
    return;
  }

  if (packageUrl.startsWith("http://") || packageUrl.startsWith("https://")) {
    // Check if it's a URL to download a snap file or a snap name from the store
    if (packageUrl.endsWith(".snap")) {
      fetchPackage(packageUrl);
    } else {
      // Assume it's a snap name from the store
      installFromStore(packageUrl);
    }
    return;
  }

  QString localPath = packageUrl;
  localPath.remove(QRegExp("^file://"));
  installLocalPackage(localPath);
}

void SnapBackend::removePackage(const QString& appId, const QString& version)
{
  Q_UNUSED(version);

  if (m_installerProcess) {
    return;
  }

  if (!isAvailable()) {
    qWarning() << "Snap command not available";
    Q_EMIT packageInstallationFailed();
    return;
  }

  qDebug() << "starting snap removal:" << appId;

  m_installerProcess = new QProcess(this);
  connect(m_installerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(installerFinished(int, QProcess::ExitStatus)));
  connect(m_installerProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStatusChanged(QProcess::ProcessState)));
  m_installerProcess->start("snap", QStringList() << "remove" << appId);

  Q_EMIT busyChanged();
}

bool SnapBackend::abortInstallation() const
{
  if (busy() && m_download) {
    m_download->abort();
    return true;
  }

  // Can't abort snap process easily, would need to kill it
  return false;
}

void SnapBackend::fetchPackage(const QString& packageUrl)
{
  QUrl url(packageUrl);
  qDebug() << "fetching snap package" << url.url();

  QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  m_file.setFileName(cacheDir + "/" + url.fileName());

  if (!m_file.open(QFile::WriteOnly | QFile::Truncate)) {
    qDebug() << "Cannot open temp file" << m_file.fileName();
    Q_EMIT packageInstallationFailed();
    return;
  }

  QNetworkRequest request;
  request.setUrl(url);

  m_download = m_nam->get(request);
  connect(m_download, &QNetworkReply::downloadProgress, this, &SnapBackend::slotDownloadProgress);
  connect(m_download, &QNetworkReply::finished, this, &SnapBackend::downloadFinished);

  Q_EMIT busyChanged();
  Q_EMIT downloadProgressChanged();
}

void SnapBackend::installLocalPackage(const QString& packageFilePath)
{
  if (m_installerProcess) {
    return;
  }

  if (!isAvailable()) {
    qWarning() << "Snap command not available";
    Q_EMIT packageInstallationFailed();
    return;
  }

  qDebug() << "starting snap installer:" << packageFilePath;

  m_installerProcess = new QProcess(this);
  connect(m_installerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(installerFinished(int, QProcess::ExitStatus)));
  connect(m_installerProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStatusChanged(QProcess::ProcessState)));
  m_installerProcess->start("snap",
                            QStringList() << "install"
                                          << "--dangerous" << packageFilePath);

  Q_EMIT busyChanged();
}

void SnapBackend::installFromStore(const QString& snapName)
{
  if (m_installerProcess) {
    return;
  }

  if (!isAvailable()) {
    qWarning() << "Snap command not available";
    Q_EMIT packageInstallationFailed();
    return;
  }

  qDebug() << "installing snap from store:" << snapName;

  m_installerProcess = new QProcess(this);
  connect(m_installerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(installerFinished(int, QProcess::ExitStatus)));
  connect(m_installerProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStatusChanged(QProcess::ProcessState)));
  m_installerProcess->start("snap", QStringList() << "install" << snapName);

  Q_EMIT busyChanged();
}

void SnapBackend::installerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  qDebug() << "snap installing finished" << exitCode << exitStatus;
  qDebug() << "stdout:" << QString::fromUtf8(m_installerProcess->readAllStandardOutput());
  qDebug() << "stderr:" << QString::fromUtf8(m_installerProcess->readAllStandardError());

  m_installerProcess->deleteLater();
  m_installerProcess = nullptr;
  Q_EMIT busyChanged();

  if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
    Q_EMIT packageInstalled();
  } else {
    Q_EMIT packageInstallationFailed();
  }
}

void SnapBackend::processStatusChanged(QProcess::ProcessState state)
{
  Q_UNUSED(state);
}

void SnapBackend::slotDownloadProgress()
{
  m_file.write(m_download->readAll());
  Q_EMIT downloadProgressChanged();
}

void SnapBackend::downloadFinished()
{
  qDebug() << "snap download finished" << m_download->error();
  if (m_download->error() != QNetworkReply::NoError) {
    qDebug() << m_download->errorString() << m_download->attribute(QNetworkRequest::RedirectionTargetAttribute);
  }

  m_file.write(m_download->readAll());
  m_file.close();

  m_download->deleteLater();

  m_isLocalInstall = false;
  Q_EMIT isLocalInstallChanged();

  if (m_download->error() == QNetworkReply::OperationCanceledError) {
    Q_EMIT downloadProgressChanged();
    m_download = nullptr;
    Q_EMIT busyChanged();
    return;
  }

  if (m_download->error() != QNetworkReply::NoError) {
    m_download = nullptr;
    Q_EMIT downloadProgressChanged();
    Q_EMIT busyChanged();
    Q_EMIT packageInstallationFailed();
    return;
  }

  if (!m_download->attribute(QNetworkRequest::RedirectionTargetAttribute).toString().isEmpty()) {
    qDebug() << "fetching new url:" << m_download->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    fetchPackage(m_download->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl().toString());
  } else {
    qDebug() << "Snap package fetched. Starting installation";
    m_download = nullptr;
    installLocalPackage(m_file.fileName());
    Q_EMIT downloadProgressChanged();
    Q_EMIT busyChanged();
  }
}
