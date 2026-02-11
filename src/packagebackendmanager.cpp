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

#include "packagebackendmanager.h"
#include "clickbackend.h"
#include "snapbackend.h"

#include <QDebug>

PackageBackendManager* PackageBackendManager::m_instance = nullptr;

PackageBackendManager::PackageBackendManager(QObject* parent)
  : QObject(parent)
  , m_clickBackend(nullptr)
  , m_snapBackend(nullptr)
{
  setupBackends();
}

PackageBackendManager::~PackageBackendManager()
{
  // Backends are children, will be deleted automatically
}

PackageBackendManager* PackageBackendManager::instance()
{
  if (!m_instance) {
    m_instance = new PackageBackendManager();
  }
  return m_instance;
}

void PackageBackendManager::setupBackends()
{
  // Always create Click backend (it's the primary for Ubuntu Touch)
  m_clickBackend = new ClickBackend(this);
  m_backends["click"] = m_clickBackend;

  connect(m_clickBackend, &PackageBackend::busyChanged, this, &PackageBackendManager::anyBackendBusyChanged);
  connect(m_clickBackend, &PackageBackend::packageInstalled, this, [=]() { Q_EMIT packageInstalled("click"); });
  connect(m_clickBackend, &PackageBackend::packageInstallationFailed, this, [=]() { Q_EMIT packageInstallationFailed("click"); });

#ifdef ENABLE_SNAP_SUPPORT
  // Create Snap backend if enabled at compile time
  m_snapBackend = new SnapBackend(this);

  // Only register if snap is available on the system
  if (m_snapBackend->isAvailable()) {
    m_backends["snap"] = m_snapBackend;
    qDebug() << "Snap backend registered and available";

    connect(m_snapBackend, &PackageBackend::busyChanged, this, &PackageBackendManager::anyBackendBusyChanged);
    connect(m_snapBackend, &PackageBackend::packageInstalled, this, [=]() { Q_EMIT packageInstalled("snap"); });
    connect(m_snapBackend, &PackageBackend::packageInstallationFailed, this, [=]() { Q_EMIT packageInstallationFailed("snap"); });
  } else {
    qDebug() << "Snap backend not available on this system";
    delete m_snapBackend;
    m_snapBackend = nullptr;
  }
#endif
}

PackageBackend* PackageBackendManager::getBackend(const QString& packageType)
{
  return m_backends.value(packageType.toLower(), nullptr);
}

PackageBackend* PackageBackendManager::getBackendForPackage(const QString& appId)
{
  // Check each backend for this package
  QHashIterator<QString, PackageBackend*> i(m_backends);
  while (i.hasNext()) {
    i.next();
    PackageBackend* backend = i.value();
    QList<InstalledPackageInfo> packages = backend->getInstalledPackages();

    for (const InstalledPackageInfo& pkg : packages) {
      if (pkg.appId == appId) {
        return backend;
      }
    }
  }

  return nullptr;
}

QList<InstalledPackageInfo> PackageBackendManager::getAllInstalledPackages()
{
  QList<InstalledPackageInfo> allPackages;

  QHashIterator<QString, PackageBackend*> i(m_backends);
  while (i.hasNext()) {
    i.next();
    PackageBackend* backend = i.value();
    allPackages.append(backend->getInstalledPackages());
  }

  return allPackages;
}

bool PackageBackendManager::anyBackendBusy() const
{
  QHashIterator<QString, PackageBackend*> i(m_backends);
  while (i.hasNext()) {
    i.next();
    if (i.value()->busy()) {
      return true;
    }
  }

  return false;
}
