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

#include "localpackagesmodel.h"
#include "../openstorenetworkmanager.h"
#include "../packagebackendmanager.h"
#include "../packagescache.h"
#include "../platformintegration.h"

#include <algorithm>

#define MODEL_START_REFRESH()                                                                                                              \
  m_ready = false;                                                                                                                         \
  Q_EMIT readyChanged();
#define MODEL_END_REFRESH()                                                                                                                \
  m_ready = true;                                                                                                                          \
  Q_EMIT readyChanged();

/*
    Sort available updates first, then by name
*/
bool sortPackage(const LocalPackageItem& a, const LocalPackageItem& b)
{
  if (a.updateStatus == b.updateStatus) {
    return (a.name.compare(b.name, Qt::CaseInsensitive) < 0);
  }

  return (a.updateStatus.compare(b.updateStatus) < 0);
}

LocalPackagesModel::LocalPackagesModel(QAbstractListModel* parent)
  : QAbstractListModel(parent)
  , m_ready(false)
  , m_appStoreUpdateAvailable(false)
{
  m_source = PackageBackendManager::instance()->activeSource();

  connect(PlatformIntegration::instance(), &PlatformIntegration::updated, this, &LocalPackagesModel::refresh);
  connect(PackagesCache::instance(), &PackagesCache::updatingCacheChanged, this, &LocalPackagesModel::refresh);
  connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::snapSupportChanged, this, &LocalPackagesModel::refresh);
  connect(m_source, &PackageSource::installedChanged, this, &LocalPackagesModel::refresh);

  refresh();
}

QHash<int, QByteArray> LocalPackagesModel::roleNames() const
{
  QHash<int, QByteArray> roles;

  roles.insert(RoleName, "name");
  roles.insert(RoleAppId, "appId");
  roles.insert(RoleVersion, "version");
  roles.insert(RoleIcon, "icon");
  roles.insert(RoleUpdateAvailable, "updateAvailable");
  roles.insert(RoleUpdateStatus, "updateStatus");
  roles.insert(RolePackageUrl, "packageUrl");
  roles.insert(RoleAppLaunchUrl, "appLaunchUrl");
  roles.insert(RolePackageType, "packageType");

  return roles;
}

int LocalPackagesModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return m_list.count();
}

QVariant LocalPackagesModel::data(const QModelIndex& index, int role) const
{
  if (index.row() < 0 || index.row() > rowCount())
    return QVariant();

  auto pkg = m_list.at(index.row());

  switch (role) {
    case RoleName:
      return pkg.name;
    case RoleAppId:
      return pkg.appId;
    case RoleVersion:
      return pkg.version;
    case RoleIcon:
      return pkg.icon;
    case RoleUpdateAvailable:
      return pkg.updateAvailable;
    case RoleUpdateStatus:
      return pkg.updateStatus;
    case RolePackageUrl:
      return pkg.packageUrl;
    case RoleAppLaunchUrl:
      return pkg.appLaunchUrl;
    case RolePackageType:
      return pkg.packageType;

    default:
      return QVariant();
  }
}

int LocalPackagesModel::updatesAvailableCount() const
{
  int result = 0;

  Q_FOREACH (const LocalPackageItem& pkg, m_list) {
    if (pkg.updateStatus == QStringLiteral("available")) {
      ++result;
    }
  }

  return result;
}

int LocalPackagesModel::downgradesAvailableCount() const
{
  int result = 0;

  Q_FOREACH (const LocalPackageItem& pkg, m_list) {
    if (pkg.updateStatus == QStringLiteral("downgrade")) {
      ++result;
    }
  }

  return result;
}

int LocalPackagesModel::snapsCount() const
{
  int result = 0;

  Q_FOREACH (const LocalPackageItem& pkg, m_list) {
    if (pkg.updateStatus == QStringLiteral("snap")) {
      ++result;
    }
  }

  return result;
}

void LocalPackagesModel::refresh()
{
  MODEL_START_REFRESH();

  beginResetModel();
  m_list = m_source->requestInstalled();
  std::sort(m_list.begin(), m_list.end(), sortPackage);

  m_appStoreUpdateAvailable = false;
  Q_FOREACH (const LocalPackageItem& pkg, m_list) {
    if (pkg.packageType == QStringLiteral("click") && pkg.appId == m_appStoreAppId && pkg.updateStatus == QStringLiteral("available")) {
      m_appStoreUpdateAvailable = true;
      break;
    }
  }
  Q_EMIT appStoreUpdateAvailableChanged();

  endResetModel();

  Q_EMIT updated();
  MODEL_END_REFRESH();
}

QVariantMap LocalPackagesModel::get(int row)
{
  QHash<int, QByteArray> names = roleNames();
  QHashIterator<int, QByteArray> ittr(names);
  QVariantMap map;
  while (ittr.hasNext()) {
    ittr.next();

    QModelIndex idx = index(row, 0);
    QVariant data = idx.data(ittr.key());
    map[ittr.value()] = data;
  }

  return map;
}

QVariantMap LocalPackagesModel::getByAppId(const QString& appId)
{
  for (int i = 0; i < m_list.count(); i++) {
    if (m_list[i].appId == appId) {
      return get(i);
    }
  }

  return QVariantMap();
}
