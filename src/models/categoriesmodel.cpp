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

#include "categoriesmodel.h"
#include "../openstorenetworkmanager.h"
#include "../packageSource.h"
#include "../packagebackendmanager.h"

CategoriesModel::CategoriesModel(QObject* parent)
  : QAbstractListModel(parent)
  , m_ready(false)
{
  m_source = PackageBackendManager::instance()->activeSource();
  connect(m_source, &PackageSource::categoriesReplied, this, &CategoriesModel::parseReply);
  connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::reloaded, this, &CategoriesModel::update);

  update();
}

QHash<int, QByteArray> CategoriesModel::roleNames() const
{
  QHash<int, QByteArray> roles;

  roles.insert(RoleName, "name");
  roles.insert(RoleId, "id");
  roles.insert(RoleCount, "count");
  roles.insert(RoleIconUrl, "iconUrl");

  return roles;
}

int CategoriesModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return m_list.count();
}

QVariant CategoriesModel::data(const QModelIndex& index, int role) const
{
  if (index.row() < 0 || index.row() > rowCount())
    return QVariant();

  const CategoryItem& cat = m_list.at(index.row());

  switch (role) {
    case RoleName:
      return cat.name;
    case RoleId:
      return cat.id;
    case RoleCount:
      return cat.count;
    case RoleIconUrl:
      return cat.iconUrl;

    default:
      return QVariant();
  }
}

void CategoriesModel::update()
{
  m_source->requestCategories();
}

void CategoriesModel::parseReply(const QList<CategoryItem>& categories)
{
  beginResetModel();
  m_list.clear();
  endResetModel();

  beginInsertRows(QModelIndex(), 0, categories.count() - 1);
  m_list.append(categories);
  endInsertRows();

  m_ready = true;
  Q_EMIT updated();
}
