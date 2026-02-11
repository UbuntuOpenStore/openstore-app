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
 *
 */

#ifndef PACKAGEBACKEND_H
#define PACKAGEBACKEND_H

#include <QObject>
#include <QVariant>

struct InstalledPackageInfo
{
  QString appId;
  QString version;
  QString packageType; // "click", "snap", etc.
  QVariantMap manifest;
};

class PackageBackend : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(bool isLocalInstall READ isLocalInstall NOTIFY isLocalInstallChanged)
  Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)

public:
  explicit PackageBackend(QObject* parent = nullptr)
    : QObject(parent)
  {
  }
  virtual ~PackageBackend() {}

  virtual bool busy() const = 0;
  virtual bool isLocalInstall() const = 0;
  virtual int downloadProgress() const = 0;

  // Check if this backend is available on the system
  virtual bool isAvailable() const = 0;

  // Get list of installed packages from this backend
  virtual QList<InstalledPackageInfo> getInstalledPackages() = 0;

public Q_SLOTS:
  virtual void installPackage(const QString& packageUrl, bool isLocalInstall = false) = 0;
  virtual void removePackage(const QString& appId, const QString& version) = 0;
  virtual bool abortInstallation() const = 0;

Q_SIGNALS:
  void busyChanged();
  void isLocalInstallChanged();
  void downloadProgressChanged();
  void packageInstalled();
  void packageInstallationFailed();
};

#endif // PACKAGEBACKEND_H
