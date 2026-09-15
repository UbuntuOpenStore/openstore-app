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

#include "searchmodel.h"
#include "../packagebackendmanager.h"
#include "../platformintegration.h"

SearchModel::SearchModel(QObject* parent)
  : QAbstractListModel(parent)
{
  m_source = PackageBackendManager::instance()->activeSource();

  connect(m_source, &PackageSource::searchReplied, this, &SearchModel::searchReplied);
  connect(PlatformIntegration::instance(), &PlatformIntegration::updated, m_source, &PackageSource::refreshInstalledState);

  connect(this, &SearchModel::filterStringChanged, this, &SearchModel::update);
  connect(this, &SearchModel::categoryChanged, this, &SearchModel::update);
  connect(this, &SearchModel::sortModeChanged, this, &SearchModel::update);
  connect(this, &SearchModel::queryUrlChanged, this, &SearchModel::update);
  connect(this, &SearchModel::filterTypeChanged, this, &SearchModel::update);
  connect(this, &SearchModel::filterPackageTypeChanged, this, &SearchModel::update);

  update();
}

int SearchModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return m_list.count();
}

QVariant SearchModel::data(const QModelIndex& index, int role) const
{
  if (index.row() < 0 || index.row() > rowCount())
    return QVariant();

  auto item = m_list.at(index.row());

  switch (role) {
    case RoleName:
      return item.name;
    case RoleAppId:
      return item.appId;
    case RoleIcon:
      return item.icon;
    case RoleRatings:
      return QVariant::fromValue<Ratings*>(item.ratings);
    case RoleTagline:
      return item.tagline;
    case RoleInstalled:
      return item.installed;
    case RoleUpdateAvailable:
      return item.updateAvailable;
    case RoleTypes:
      return item.types;
    case RolePackageType:
      return item.packageType;
  }
  return QVariant();
}

QHash<int, QByteArray> SearchModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles.insert(RoleName, "name");
  roles.insert(RoleAppId, "appId");
  roles.insert(RoleIcon, "icon");
  roles.insert(RoleRatings, "ratings");
  roles.insert(RoleTagline, "tagline");
  roles.insert(RoleInstalled, "installed");
  roles.insert(RoleUpdateAvailable, "updateAvailable");
  roles.insert(RoleTypes, "types");
  roles.insert(RolePackageType, "packageType");
  return roles;
}

int SearchModel::find(const QString& appId) const
{
  for (int i = 0; i < m_list.count(); ++i) {
    if (m_list.at(i).appId == appId)
      return i;
  }

  return -1;
}

void SearchModel::update()
{
  beginResetModel();
  m_list.clear();
  endResetModel();

  SearchRequest request;
  request.filterString = m_filterString;
  request.category = m_category;
  request.sortMode = m_sortMode;
  request.filterType = m_filterType;
  request.filterPackageType = m_filterPackageType;
  request.queryUrl = m_queryUrl;
  request.offset = 0;
  request.limit = 30;
  m_source->requestSearch(request);
}

bool SearchModel::canFetchMore(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return m_queryUrl.isValid() ? false : bool(!m_fetchedAll);
}

void SearchModel::fetchMore(const QModelIndex& parent)
{
  Q_UNUSED(parent)
  SearchRequest request;
  request.filterString = m_filterString;
  request.category = m_category;
  request.sortMode = m_sortMode;
  request.filterType = m_filterType;
  request.filterPackageType = m_filterPackageType;
  request.queryUrl = m_queryUrl;
  request.offset = m_list.count();
  request.limit = 30;
  m_source->requestSearch(request);
}

void SearchModel::searchReplied(const SearchReply& reply)
{
  if (reply.refresh) {
    // Merge the new flags into the current rows by appId; never append or reset.
    for (int i = 0; i < reply.packages.count(); ++i) {
      const SearchPackageItem& refreshed = reply.packages.at(i);
      const int row = find(refreshed.appId);
      if (row < 0)
        continue;
      m_list[row].installed = refreshed.installed;
      m_list[row].updateAvailable = refreshed.updateAvailable;
      Q_EMIT dataChanged(index(row), index(row));
    }
    Q_EMIT updated();
    return;
  }

  beginInsertRows(QModelIndex(), m_list.count(), m_list.count() + reply.packages.count() - 1);
  m_list.append(reply.packages);
  endInsertRows();

  m_fetchedAll = reply.fetchedAll;
  Q_EMIT updated();
}
