#include "reviewsmodel.h"
#include "review.h"

#include <QJsonDocument>
#include <QDebug>

ReviewsModel::ReviewsModel(const QString &appId, QObject *parent)
    : QAbstractListModel(parent), m_appId(appId)
{
    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::newReply,
            this, &ReviewsModel::parseReply);

    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getReviews(m_requestSignature, appId);
}


int ReviewsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_list.count();
}


QVariant ReviewsModel::data(const QModelIndex &index, int role) const
{
    const ReviewItem &item = m_list.at(index.row());

    switch (role) {
    case RoleVersion:
        return item.reviewedVersion();
    case RoleRating:
        return item.rating();
    }
    return QVariant();
}


QHash<int, QByteArray> ReviewsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleVersion, "version");
    roles.insert(RoleRating, "rating");
    return roles;
}


void ReviewsModel::loadMore()
{
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getReviews(m_requestSignature, m_appId, 50, m_list.constLast().id());
}


bool ReviewsModel::postReview(const QString &version, const QString &review, ReviewItem::Rating rating, const QString &apiKey)
{
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    return OpenStoreNetworkManager::instance()->postReview(m_requestSignature, m_appId, version, review, rating, apiKey);
}


void ReviewsModel::parseReply(OpenStoreReply reply)
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
        qWarning() << Q_FUNC_INFO << "Server replied with error";
        return;
    }

    QVariantMap data = replyMap.value("data").toMap();

    /*
     * TODO
     *
     * QVariantMap highlight = data.value("highlight").toMap();
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
    }*/

    Q_EMIT updated();
}
