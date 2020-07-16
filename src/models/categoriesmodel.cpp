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

CategoriesModel::CategoriesModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_ready(false)
{
    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::parsedReply, this, &CategoriesModel::parseReply);
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

int CategoriesModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return m_list.count();
}

QVariant CategoriesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > rowCount())
        return QVariant();

    const CategoryItem &cat = m_list.at(index.row());

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
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getCategories(m_requestSignature);
}

void CategoriesModel::parseReply(OpenStoreReply reply)
{
    if (reply.signature != m_requestSignature)
        return;

    QVariantList data = reply.data.toList();

    beginResetModel();
    m_list.clear();
    endResetModel();

    beginInsertRows(QModelIndex(), 0, data.count() - 1);
    Q_FOREACH(const QVariant & cat, data) {
        const QVariantMap &catMap = cat.toMap();

        CategoryItem catItem;
        catItem.id = catMap.value("category").toString();
        catItem.name = catMap.value("translation", catItem.id).toString();
        catItem.count = catMap.value("count").toInt();
        catItem.iconUrl = catMap.value("icon").toUrl();

        m_list.append(catItem);
    }
    endInsertRows();

    m_ready = true;
    Q_EMIT updated();
}
