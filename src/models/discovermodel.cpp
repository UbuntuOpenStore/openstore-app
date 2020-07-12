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

    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::newReply,
            this, &DiscoverModel::parseReply);

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

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply.data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << Q_FUNC_INFO << "Error parsing json" << error.errorString();
        return;
    }

    QVariantMap replyMap = jsonDoc.toVariant().toMap();

    if (!replyMap.value("success").toBool() || !replyMap.contains("data")) {
        qWarning() << Q_FUNC_INFO << "Server replied with error";
        return;
    }

    QVariantMap data = replyMap.value("data").toMap();

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
