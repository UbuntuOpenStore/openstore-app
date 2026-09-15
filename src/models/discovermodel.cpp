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

#include "discovermodel.h"
#include "../openstorenetworkmanager.h"
#include "../packageSource.h"
#include "../packagebackendmanager.h"

DiscoverModel::DiscoverModel(QObject* parent)
  : QAbstractListModel(parent)
{
  m_source = PackageBackendManager::instance()->activeSource();
  connect(m_source, &PackageSource::discoverReplied, this, &DiscoverModel::parseReply);
  connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::reloaded, this, &DiscoverModel::refresh);

  refresh();
}

int DiscoverModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return m_list.count();
}

QVariant DiscoverModel::data(const QModelIndex& index, int role) const
{
  const DiscoverCategoryItem& item = m_list.at(index.row());

  switch (role) {
    case RoleName:
      return item.name;
    case RoleTagline:
      return item.tagline;
    case RoleQueryUrl:
      return item.queryUrl;
    case RoleAppIds:
      return item.appIds;
  }
  return QVariant();
}

QHash<int, QByteArray> DiscoverModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles.insert(RoleName, "name");
  roles.insert(RoleTagline, "tagline");
  roles.insert(RoleQueryUrl, "queryUrl");
  roles.insert(RoleAppIds, "appIds");
  return roles;
}

PackageItem* DiscoverModel::getPackage(const QString& appId)
{
  return m_source->requestPackageDetails(appId);
}

void DiscoverModel::refresh()
{
  m_ready = false;

  // Safety is the number one priority
  beginResetModel();
  m_list.clear();
  endResetModel();

  m_source->requestDiscover();
}

void DiscoverModel::parseReply(const DiscoverReply& reply)
{
  m_highlightBannerUrl = reply.highlightBannerUrl;
  m_highlightAppId = reply.highlightAppId;

  // Replace the rows; the source re-emits the full discover state.
  beginResetModel();
  m_list.clear();
  m_list = reply.categories;
  endResetModel();

  m_ready = true;
  Q_EMIT updated();
}
