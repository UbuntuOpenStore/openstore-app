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

#ifndef PACKAGEBACKENDMANAGER_H
#define PACKAGEBACKENDMANAGER_H

#include <QHash>
#include <QObject>

#include "packagebackend.h"

class ClickBackend;
class SnapBackend;

class PackageBackendManager : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool anyBackendBusy READ anyBackendBusy NOTIFY anyBackendBusyChanged)

public:
  explicit PackageBackendManager(QObject* parent = nullptr);
  ~PackageBackendManager();

  static PackageBackendManager* instance();

  // Get backend by package type (e.g., "click", "snap")
  Q_INVOKABLE PackageBackend* getBackend(const QString& packageType);

  // Get backend for a specific installed package by appId
  Q_INVOKABLE PackageBackend* getBackendForPackage(const QString& appId);

  // Aggregate installed packages from all backends
  QList<InstalledPackageInfo> getAllInstalledPackages();

  // Check if any backend is busy
  bool anyBackendBusy() const;

Q_SIGNALS:
  void anyBackendBusyChanged();
  void packageInstalled(QString packageType);
  void packageInstallationFailed(QString packageType);

private:
  void setupBackends();

private:
  QHash<QString, PackageBackend*> m_backends;
  ClickBackend* m_clickBackend;
  SnapBackend* m_snapBackend;

  static PackageBackendManager* m_instance;
};

#endif // PACKAGEBACKENDMANAGER_H
