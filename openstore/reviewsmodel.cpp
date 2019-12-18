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
    m_replyHandler = &m_getReviewsAppendHandler;
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

    switch (role)
    {
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
    if (m_list.count() == m_reviewCount)
    {
        return;
    }
    m_replyHandler = &m_getReviewsAppendHandler;
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getReviews(m_requestSignature, m_appId, 10, m_list.constLast().id());
}

void ReviewsModel::getOwnReview(const QString &apiKey)
{
    m_replyHandler = &m_getOwnReviewReplyHandler;
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getReviews(m_requestSignature, m_appId, apiKey);
}

unsigned int ReviewsModel::reviewCount() const
{
    return m_reviewCount;
}

bool ReviewsModel::sendReview(const QString &version, const QString &review, Ratings::Rating rating, const QString &apiKey, const bool &edit)
{
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    m_replyHandler = &m_reviewPostedHandler;
    bool success = OpenStoreNetworkManager::instance()->postReview(m_requestSignature, m_appId, version, review, rating, apiKey, edit);
    if (!success)
    {
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

    if (error.error != QJsonParseError::NoError)
    {
        qWarning() << Q_FUNC_INFO << "Error parsing json";
        ReviewsModel::error(QString("Invalid response"));
        return;
    }

    if (!jsonDocument.isObject())
    {
        qWarning() << Q_FUNC_INFO << "Error parsing json";
        ReviewsModel::error(QString("Invalid response"));
        return;
    }
    QJsonObject jsonObject = jsonDocument.object();

    QJsonObject data = jsonObject["data"].toObject();

    bool success = jsonObject["success"].toBool();

    if (!success)
    {
        QString message = jsonObject["message"].toString();
        ReviewsModel::error(message);
        return;
    }

    m_replyHandler->handle(data, *this);
}

void ReviewsModel::onRefresh()
{
    m_replyHandler = &m_getReviewsResetHandler;
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getReviews(m_requestSignature, m_appId);
}

void ReviewsModel::GetOwnReviewReplyHandler::handle(const QJsonObject &data, ReviewsModel &model)
{
    QJsonArray reviews = data["reviews"].toArray();
    if (reviews.count() > 0)
    {
        Q_EMIT model.ownReviewResponse(reviews.first().toObject());
    }
    else
    {
        Q_EMIT model.ownReviewResponse(QJsonObject());
    }
}

void ReviewsModel::GetReviewsResetHandler::handle(const QJsonObject &data, ReviewsModel &model)
{
    model.m_reviewCount = data["count"].toInt();
    QJsonArray reviews = data["reviews"].toArray();

    model.beginResetModel();
    model.m_list.clear();
    Q_FOREACH (const QJsonValue &reviewJson, reviews)
    {
        ReviewItem review(reviewJson.toObject());
        model.m_list.append(review);
    }
    model.endResetModel();
    Q_EMIT model.updated();
}

void ReviewsModel::GetReviewsAppendHandler::handle(const QJsonObject &data, ReviewsModel &model)
{
    model.m_reviewCount = data["count"].toInt();
    QJsonArray reviews = data["reviews"].toArray();

    model.beginInsertRows(QModelIndex(), model.m_list.count(), model.m_list.count());
    Q_FOREACH (const QJsonValue &reviewJson, reviews)
    {
        ReviewItem review(reviewJson.toObject());
        model.m_list.append(review);
    }
    model.endInsertRows();
    Q_EMIT model.updated();
}

void ReviewsModel::ReviewPostedHandler::handle(const QJsonObject &data, ReviewsModel &model)
{
    Q_EMIT model.reviewPosted();
    Q_EMIT model.refresh();
}
