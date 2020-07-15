#include "searchmodel.h"
#include "../platformintegration.h"
#include "../packagescache.h"

#include <QJsonDocument>
#include <QDebug>

#define REQUEST_LIMIT 30

SearchModel::SearchModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::parsedReply, this, &SearchModel::parseReply);
    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::reloaded, this, &SearchModel::update);
    connect(PlatformIntegration::instance(), &PlatformIntegration::updated, this, &SearchModel::refreshInstalledInfo);

    connect(this, &SearchModel::filterStringChanged, this, &SearchModel::update);
    connect(this, &SearchModel::categoryChanged, this, &SearchModel::update);
    connect(this, &SearchModel::sortModeChanged, this, &SearchModel::update);
    connect(this, &SearchModel::queryUrlChanged, this, &SearchModel::update);

    update();
}

int SearchModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_list.count();
}

QVariant SearchModel::data(const QModelIndex &index, int role) const
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
    return roles;
}

int SearchModel::find(const QString &appId) const
{
    for (int i=0; i<m_list.count(); ++i) {
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

    sendRequest();
}

bool SearchModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_queryUrl.isValid() ? false : bool(!m_fetchedAll);
}

void SearchModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    sendRequest(m_list.count());
}


void SearchModel::sendRequest(int skip)
{
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();

    if (!m_queryUrl.isEmpty()) {
        OpenStoreNetworkManager::instance()->getByUrl(m_requestSignature, m_queryUrl);
    } else {
        if (m_filterString.isEmpty() && m_category.isEmpty()) {
            // Show latest app
            OpenStoreNetworkManager::instance()->getSearch(m_requestSignature, skip, REQUEST_LIMIT, QString(), QString(), "-updated_date");
        } else {
            OpenStoreNetworkManager::instance()->getSearch(m_requestSignature, skip, REQUEST_LIMIT, m_filterString, m_category, m_sortMode);
        }
    }
}

void SearchModel::parseReply(OpenStoreReply reply)
{
    if (reply.signature != m_requestSignature)
        return;

    QVariantMap data = reply.data.toMap();
    QVariantList pkgList = data.value("packages").toList();

    beginInsertRows(QModelIndex(), m_list.count(), m_list.count() + pkgList.count() - 1);

    Q_FOREACH (const QVariant &pkg, pkgList) {
        SearchPackageItem item;
        const QVariantMap &pkgMap = pkg.toMap();

        item.appId = pkgMap.value("id").toString();
        item.name = pkgMap.value("name").toString();
        item.tagline = pkgMap.value("tagline").toString();
        item.icon = pkgMap.value("icon").toString();
        item.types = pkgMap.value("types").toStringList();
        item.ratings = new Ratings(pkgMap.value("ratings").toMap());

        item.updateAvailable = bool(PackagesCache::instance()->getRemoteAppRevision(item.appId) > PackagesCache::instance()->getLocalAppRevision(item.appId));
        item.installed = !PlatformIntegration::instance()->appVersion(item.appId).isNull();

        m_list.append(item);
    }

    endInsertRows();

    m_fetchedAll = !data.value("next").toUrl().isValid();

    Q_EMIT updated();
}

void SearchModel::refreshInstalledInfo()
{
    for (int i=0; i<m_list.count(); ++i) {
        m_list[i].updateAvailable = bool(PackagesCache::instance()->getRemoteAppRevision(m_list[i].appId) > PackagesCache::instance()->getLocalAppRevision(m_list[i].appId));
        m_list[i].installed = !PlatformIntegration::instance()->appVersion(m_list[i].appId).isNull();
        Q_EMIT dataChanged(index(i), index(i));
    }
}
