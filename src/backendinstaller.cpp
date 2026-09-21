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

#include "backendinstaller.h"

#include "platformintegration.h"
#ifdef ENABLE_CLICK_SUPPORT
#include "installers/clickinstaller.h"
#endif
#ifdef ENABLE_DEB_SUPPORT
#include "indexstatus.h"
#include "installers/packagekitinstaller.h"
#include "packageindex.h"
#endif

BackendInstaller* BackendInstaller::m_instance = 0;

BackendInstaller::BackendInstaller()
  : m_installer(0)
  , m_indexStatus(0)
{
#ifdef ENABLE_CLICK_SUPPORT
  m_installer = PlatformIntegration::instance()->clickInstaller();
#endif
#ifdef ENABLE_DEB_SUPPORT
  m_indexStatus = PackageIndex::instance()->status();
  m_installer = PlatformIntegration::instance()->packageKitInstaller();
#endif
}

BackendInstaller* BackendInstaller::instance()
{
  if (!m_instance) {
    m_instance = new BackendInstaller();
  }
  return m_instance;
}