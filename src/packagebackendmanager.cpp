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

#include "packagebackendmanager.h"

#include "packageitems/package.h"
#include "sources/remoteapisource.h"
#ifdef ENABLE_DEB_SUPPORT
#include "sources/packagekitsource.h"
#endif

PackageBackendManager* PackageBackendManager::m_instance = 0;

PackageBackendManager::PackageBackendManager()
  : m_busy(false)
{
#ifdef ENABLE_DEB_SUPPORT
  m_source = new PackageKitSource(this);
#else
  m_source = new RemoteApiSource(this);
#endif

  connect(m_source, &PackageSource::packageDetailsReady, this, &PackageBackendManager::packageDetailsReady);
  connect(m_source, &PackageSource::packageDetailsError, this, &PackageBackendManager::packageDetailsError);
  connect(m_source, &PackageSource::updated, this, &PackageBackendManager::updated);
  connect(m_source, &PackageSource::busyChanged, this, &PackageBackendManager::busyChanged);
}

PackageBackendManager* PackageBackendManager::instance()
{
  if (!m_instance) {
    m_instance = new PackageBackendManager();
  }
  return m_instance;
}

void PackageBackendManager::refreshCatalog()
{
  m_source->refresh();
}

QStringList PackageBackendManager::sources() const
{
  return m_source->sourceDescriptions();
}