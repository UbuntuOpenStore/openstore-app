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
#include "../package.h"
#include "../platformintegration.h"
#include "../packagescache.h"

#include <QJsonDocument>
#include <QDebug>

    DiscoverModel::DiscoverModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::reloaded, this, &DiscoverModel::refresh);
    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::parsedReply, this, &DiscoverModel::parseReply);

    refresh();
}

int DiscoverModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_list.count();
}

QVariant DiscoverModel::data(const QModelIndex &index, int role) const
{
    const DiscoverCategoryItem &item = m_list.at(index.row());

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

PackageItem* DiscoverModel::getPackage(const QString &appId)
{
    return PackagesCache::instance()->get(appId);
}

void DiscoverModel::refresh()
{
    // Safety is the number one priority
    beginResetModel();
    m_list.clear();
    endResetModel();

    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getDiscover(m_requestSignature);
}

void DiscoverModel::parseReply(OpenStoreReply reply)
{
    if (reply.signature != m_requestSignature)
        return;

    QVariantMap data = reply.data.toMap();

    // Highlighted app data
    QVariantMap highlight = data.value("highlight").toMap();
    m_highlightBannerUrl = highlight.value("image").toUrl();
    m_highlightAppId = highlight.value("id").toString();

    if (!PackagesCache::instance()->contains(m_highlightAppId)) {
        PackagesCache::instance()->insert(m_highlightAppId, highlight.value("app").toMap());
    }

    // Categories parsing
    QVariantList categories = data.value("categories").toList();
    Q_FOREACH(const QVariant &categoryVariant, categories) {
        const QVariantMap &category = categoryVariant.toMap();

        beginInsertRows(QModelIndex(), m_list.count(), m_list.count());
        DiscoverCategoryItem item;
        item.name = category.value("name").toString();
        item.tagline = category.value("tagline").toString();
        item.queryUrl = category.value("query_url").toString();
        item.appIds = category.value("ids").toStringList();
        m_list.append(item);
        endInsertRows();

        QVariantList catAppsList = category.value("apps").toList();
        Q_FOREACH(const QVariant &appVariant, catAppsList) {
            const QVariantMap &app = appVariant.toMap();
            const QString &appId = app.value("id").toString();

            if (!PackagesCache::instance()->contains(appId)) {
                PackagesCache::instance()->insert(appId, app);
            }
        }
    }

    Q_EMIT updated();
}
