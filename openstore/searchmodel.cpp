#include "searchmodel.h"
#include "platformintegration.h"
#include "packagescache.h"

#include <QJsonDocument>
#include <QDebug>

#define REQUEST_LIMIT 30

SearchModel::SearchModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::newReply,
            this, &SearchModel::parseReply);

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
    PackageItem* item = PackagesCache::instance()->get(m_list.at(index.row()));

    if (item == Q_NULLPTR) {
        return QVariant();
    }

    switch (role) {
    case RoleName:
        return item->name();
    case RoleIcon:
        return item->icon();
    case RoleTagline:
        return item->tagline();
    case RoleInstalled:
        return item->installed();
    case RoleUpdateAvailable:
        return item->updateAvailable();
    }
    return QVariant();
}

QHash<int, QByteArray> SearchModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleName, "name");
    roles.insert(RoleIcon, "icon");
    roles.insert(RoleTagline, "tagline");
    roles.insert(RoleInstalled, "installed");
    roles.insert(RoleUpdateAvailable, "updateAvailable");
    return roles;
}

int SearchModel::find(const QString &appId) const
{
    return m_list.indexOf(appId);   // Returns -1 if not found
}

PackageItem *SearchModel::getPackage(int index) const
{
    if (index < 0 || index + 1 > m_list.count()) {
        qWarning() << Q_FUNC_INFO << "Index is out of range. Returning nullptr...";
        return Q_NULLPTR;
    }

    return PackagesCache::instance()->get(m_list.at(index));
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
        OpenStoreNetworkManager::instance()->getUrl(m_requestSignature, m_queryUrl);
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

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply.data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << Q_FUNC_INFO << "Error parsing json";
        return;
    }

    QVariantMap replyMap = jsonDoc.toVariant().toMap();

    if (!replyMap.value("success").toBool() || !replyMap.contains("data")) {
        qWarning() << Q_FUNC_INFO << "Error retriving info from" << reply.url;
        return;
    }

    QVariantMap data = replyMap.value("data").toMap();

    QVariantList pkgList = data.value("packages").toList();

    beginInsertRows(QModelIndex(), m_list.count(), m_list.count() + pkgList.count() - 1);

    Q_FOREACH (const QVariant &pkg, pkgList) {
        const QVariantMap &pkgMap = pkg.toMap();

        const QString &appId = pkgMap.value("id").toString();

        if (!PackagesCache::instance()->contains(appId)) {
            PackagesCache::instance()->insert(appId, pkgMap);
        }

        m_list.append(appId);

        connect(PackagesCache::instance()->get(appId), &PackageItem::installedChanged, [=]() {
            int idx = m_list.indexOf(appId);
            Q_EMIT dataChanged(index(idx), index(idx));
        });
    }

    endInsertRows();

    m_fetchedAll = !data.value("next").toUrl().isValid();

    Q_EMIT updated();
}
