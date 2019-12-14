#include "reviewsmodel.h"
#include "review.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ReviewsModel::ReviewsModel(const QString &appId, QObject *parent)
    : QAbstractListModel(parent), m_appId(appId)
{
    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::newReply,
            this, &ReviewsModel::parseReply);
    connect(this, &ReviewsModel::refresh, this, &ReviewsModel::onRefresh);

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
    case RoleId:
        return item.id();
    case RoleBody:
        return item.body();
    case RoleComment:
        return QVariant::fromValue<ReviewItem::Comment>(item.comment());
    case RoleRedacted:
        return item.redacted();
    case RoleAuthor:
        return item.author();
    case RoleVersion:
        return item.version();
    case RoleRating:
        return item.rating();
    case RoleDate:
        return item.date();
    }
    return QVariant();
}


QHash<int, QByteArray> ReviewsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleId, "id");
    roles.insert(RoleBody, "body");
    roles.insert(RoleComment, "comment");
    roles.insert(RoleRedacted, "redacted");
    roles.insert(RoleAuthor, "author");
    roles.insert(RoleRating, "rating");
    roles.insert(RoleDate, "date");
    roles.insert(RoleVersion, "version");
    return roles;
}


void ReviewsModel::loadMore()
{
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getReviews(m_requestSignature, m_appId, 50, m_list.constLast().id());
}


unsigned int ReviewsModel::reviewCount() const
{
    return m_reviewCount;
}


bool ReviewsModel::sendReview(const QString &version, const QString &review, Ratings::Rating rating, const QString &apiKey, const bool &edit)
{
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    bool success = OpenStoreNetworkManager::instance()->postReview(m_requestSignature, m_appId, version, review, rating, apiKey, edit);
    if (!success) {
        qWarning() << Q_FUNC_INFO << "Posting review failed";
        return false;
    }
    return true;
}

void ReviewsModel::parseReply(OpenStoreReply reply)
{
    if (reply.signature != m_requestSignature)
        return;

    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << Q_FUNC_INFO << "Error parsing json";
        return;
    }

    if (!jsonDocument.isObject()) {
        qWarning() << Q_FUNC_INFO << "Error parsing json";
        return;
    }
    QJsonObject jsonObject = jsonDocument.object();

    QJsonObject data = jsonObject["data"].toObject();

    if (data.contains("reviews")) {
        m_reviewCount = data["count"].toInt();
        QJsonArray reviews = data["reviews"].toArray();

        beginResetModel();
        m_list.clear();
        Q_FOREACH(const QJsonValue &reviewJson, reviews) {
            qDebug() << reviewJson;
            ReviewItem review(reviewJson.toObject());
            m_list.append(review);
        }
        endResetModel();
    }
    else if (data.contains("review_id")) {
        Q_EMIT refresh();
        return;
    }
    else {
        qWarning() << Q_FUNC_INFO << "Invalid response to sendReview: " << data;
    }
    
    Q_EMIT updated();
}

void ReviewsModel::onRefresh()
{
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getReviews(m_requestSignature, m_appId);
}
