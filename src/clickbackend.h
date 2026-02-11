/*
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
 *
 */

#ifndef CLICKBACKEND_H
#define CLICKBACKEND_H

#include "packagebackend.h"

#include <QDBusInterface>
#include <QFile>
#include <QProcess>

class QNetworkReply;
class QNetworkAccessManager;
class QProcess;

class ClickBackend : public PackageBackend
{
  Q_OBJECT

public:
  explicit ClickBackend(QObject* parent = nullptr);

  bool busy() const override;
  bool isLocalInstall() const override;
  int downloadProgress() const override;
  bool isAvailable() const override;

  QList<InstalledPackageInfo> getInstalledPackages() override;

public Q_SLOTS:
  void installPackage(const QString& packageUrl, bool isLocalInstall = false) override;
  void removePackage(const QString& appId, const QString& version) override;
  bool abortInstallation() const override;

private:
  void fetchPackage(const QString& packageUrl);
  void installLocalPackage(const QString& packageFilePath);

private Q_SLOTS:
  void installerFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void processStatusChanged(QProcess::ProcessState state);

  void slotDownloadProgress();
  void downloadFinished();

private:
  QProcess* m_installerProcess;
  QNetworkReply* m_download;

  QNetworkAccessManager* m_nam;
  QFile m_file;

  bool m_isLocalInstall;

  QDBusConnection m_bus;
  QDBusInterface m_iface;
};

#endif // CLICKBACKEND_H
