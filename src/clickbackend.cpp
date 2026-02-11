/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
 * Copyright (C) 2020 Brian Douglass
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

#include "clickbackend.h"

#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QUrl>

#include <click.h>
#include <gio/gio.h>
#include <glib.h>

ClickBackend::ClickBackend(QObject* parent)
  : PackageBackend(parent)
  , m_installerProcess(nullptr)
  , m_download(nullptr)
  , m_isLocalInstall(false)
  , m_bus(QDBusConnection::systemBus())
  , m_iface("com.lomiri.click", "/com/lomiri/click", "com.lomiri.click", m_bus)
{
  m_nam = new QNetworkAccessManager(this);
}

bool ClickBackend::busy() const
{
  return m_installerProcess != nullptr || m_download != nullptr;
}

bool ClickBackend::isLocalInstall() const
{
  return m_isLocalInstall;
}

int ClickBackend::downloadProgress() const
{
  if (m_file.isOpen()) {
    return m_file.size();
  }
  return 0;
}

bool ClickBackend::isAvailable() const
{
  // Click is always available on Ubuntu Touch
  return true;
}

QList<InstalledPackageInfo> ClickBackend::getInstalledPackages()
{
  QList<InstalledPackageInfo> packages;

  ClickDB* clickdb;
  GError* err = nullptr;
  gchar* clickmanifest = nullptr;

  clickdb = click_db_new();
  click_db_read(clickdb, nullptr, &err);

  if (err != nullptr) {
    g_warning("Unable to read Click database: %s", err->message);
    g_error_free(err);
    g_object_unref(clickdb);
    return packages;
  }

  ClickUser* clickUser = click_user_new_for_user(clickdb, "phablet", &err);

  if (err != nullptr) {
    g_error_free(err);
    g_object_unref(clickdb);
    g_object_unref(clickUser);
    return packages;
  }

  clickmanifest = click_user_get_manifests_as_string(clickUser, &err);
  g_object_unref(clickdb);
  g_object_unref(clickUser);

  if (err != nullptr) {
    g_warning("Unable to get the manifests: %s", err->message);
    g_error_free(err);
    return packages;
  }

  QJsonParseError error;
  QJsonDocument jsond = QJsonDocument::fromJson(clickmanifest, &error);
  g_free(clickmanifest);

  if (error.error != QJsonParseError::NoError) {
    qWarning() << "Click manifest parse error:" << error.errorString();
    return packages;
  }

  QVariantList clickDb = jsond.toVariant().toList();

  for (const QVariant& appJson : clickDb) {
    QVariantMap appMap = appJson.toMap();
    InstalledPackageInfo info;
    info.appId = appMap.value("name").toString();
    info.version = appMap.value("version").toString();
    info.packageType = "click";
    info.manifest = appMap;
    packages.append(info);
  }

  return packages;
}

void ClickBackend::installPackage(const QString& packageUrl, const bool isLocalInstall)
{
  m_isLocalInstall = isLocalInstall;
  Q_EMIT isLocalInstallChanged();

  if (busy()) {
    return;
  }

  if (packageUrl.startsWith("http://") || packageUrl.startsWith("https://")) {
    fetchPackage(packageUrl);
    return;
  }

  QString localPath = packageUrl;
  localPath.remove(QRegExp("^file://"));
  installLocalPackage(localPath);
}

void ClickBackend::removePackage(const QString& appId, const QString& version)
{
  if (m_installerProcess) {
    return;
  }
  qDebug() << "starting package removal:" << appId << version << m_iface.isValid();

  if (m_iface.isValid()) {
    auto reply = m_iface.call("Remove", appId);
    if (reply.type() == QDBusMessage::ErrorMessage) {
      qCritical() << "Error removing package" << appId;
      Q_EMIT packageInstallationFailed();
    } else {
      Q_EMIT packageInstalled();
    }
  } else {
    m_installerProcess = new QProcess(this);
    connect(m_installerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(installerFinished(int, QProcess::ExitStatus)));
    connect(m_installerProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStatusChanged(QProcess::ProcessState)));
    m_installerProcess->start("pkcon", QStringList() << "remove" << appId + ";" + version + ";all;local:click");
  }
  Q_EMIT busyChanged();
}

bool ClickBackend::abortInstallation() const
{
  if (busy() && m_download) {
    m_download->abort();
    return true;
  }

  return false;
}

void ClickBackend::fetchPackage(const QString& packageUrl)
{
  QUrl url(packageUrl);
  qDebug() << "fetching package" << url.url();

  m_file.setFileName("/tmp/" + url.fileName());
  if (!m_file.open(QFile::WriteOnly | QFile::Truncate)) {
    qDebug() << "Cannot open temp file" << m_file.fileName();
    return;
  }

  QNetworkRequest request;
  request.setUrl(url);

  m_download = m_nam->get(request);
  connect(m_download, &QNetworkReply::downloadProgress, this, &ClickBackend::slotDownloadProgress);
  connect(m_download, &QNetworkReply::finished, this, &ClickBackend::downloadFinished);

  Q_EMIT busyChanged();
  Q_EMIT downloadProgressChanged();
}

void ClickBackend::installLocalPackage(const QString& packageFilePath)
{
  if (m_installerProcess) {
    return;
  }
  qDebug() << "starting installer:" << packageFilePath << m_iface.isValid();

  if (m_iface.isValid()) {
    auto reply = m_iface.call("Install", packageFilePath);
    if (reply.type() == QDBusMessage::ErrorMessage) {
      qCritical() << "Error Installing package" << packageFilePath;
      Q_EMIT packageInstallationFailed();
    } else {
      Q_EMIT packageInstalled();
    }
  } else {
    m_installerProcess = new QProcess(this);
    connect(m_installerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(installerFinished(int, QProcess::ExitStatus)));
    connect(m_installerProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStatusChanged(QProcess::ProcessState)));
    m_installerProcess->start("pkcon",
                              QStringList() << "install-local"
                                            << "--allow-untrusted" << packageFilePath);
  }
  Q_EMIT busyChanged();
}

void ClickBackend::installerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  qDebug() << "installing finished" << exitCode << exitStatus;
  qDebug() << "stdout:" << QString::fromUtf8(m_installerProcess->readAll());
  m_installerProcess->deleteLater();
  m_installerProcess = nullptr;
  Q_EMIT busyChanged();
  if (exitCode == 0 && exitStatus == 0) {
    Q_EMIT packageInstalled();
  } else {
    Q_EMIT packageInstallationFailed();
  }
}

void ClickBackend::processStatusChanged(QProcess::ProcessState state)
{
  Q_UNUSED(state);
}

void ClickBackend::slotDownloadProgress()
{
  m_file.write(m_download->readAll());
  Q_EMIT downloadProgressChanged();
}

void ClickBackend::downloadFinished()
{
  qDebug() << "finished" << m_download->error();
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

  if (!m_download->attribute(QNetworkRequest::RedirectionTargetAttribute).toString().isEmpty()) {
    qDebug() << "fetching new url:" << m_download->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    fetchPackage(m_download->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl().toString());
  } else {
    qDebug() << "Package fetched. Starting installation";
    m_download = nullptr;
    installLocalPackage(m_file.fileName());
    Q_EMIT downloadProgressChanged();
    Q_EMIT busyChanged();
  }
}
