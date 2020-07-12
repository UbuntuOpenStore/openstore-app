#include "reviewsmodel.h"
#include "../review.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ReviewsModel::ReviewsModel(const QString &appId, QObject *parent)
    : QAbstractListModel(parent), m_replyType(ReplyType::AppendReviews), m_appId(appId), m_loadMorePending(false)
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
    if (m_loadMorePending) {
        return;
    }
    m_loadMorePending = true;
    m_replyType = ReplyType::AppendReviews;
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getReviews(m_requestSignature, m_appId, 10, m_list.constLast().date());
}

void ReviewsModel::getOwnReview(const QString &apiKey)
{
    m_replyType = ReplyType::HandleOwnReview;
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
    m_replyType = ReplyType::HandleReviewPosted;
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
        qWarning() << Q_FUNC_INFO << "Error parsing json" << error.errorString();
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

    switch (m_replyType) {
        case ReplyType::AppendReviews:
            handleAppendReviews(data);
            break;
        case ReplyType::HandleReviewPosted:
            handleReviewPosted(data);
            break;
        case ReplyType::ResetReviews:
            handleResetReviews(data);
            break;
        case ReplyType::HandleOwnReview:
            handleOwnReview(data);
            break;
    }
}

void ReviewsModel::onRefresh()
{
    m_replyType = ReplyType::ResetReviews;
    m_requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getReviews(m_requestSignature, m_appId);
}

void ReviewsModel::handleOwnReview(const QJsonObject &data)
{
    QJsonArray reviews = data["reviews"].toArray();
    if (reviews.count() > 0)
    {
        QJsonObject review(reviews.first().toObject());
        Q_EMIT ownReviewResponse(review, Ratings::ratingFromString(review["rating"].toString()));
    }
    else
    {
        Q_EMIT ownReviewResponse(QJsonObject(), Ratings::Rating::RatingNeutral);
    }
}

void ReviewsModel::handleResetReviews(const QJsonObject &data)
{
    m_reviewCount = data["count"].toInt();
    QJsonArray reviews = data["reviews"].toArray();

    beginResetModel();
    m_list.clear();
    Q_FOREACH (const QJsonValue &reviewJson, reviews)
    {
        ReviewItem review(reviewJson.toObject());
        m_list.append(review);
    }
    endResetModel();
    Q_EMIT updated();
}

void ReviewsModel::handleAppendReviews(const QJsonObject &data)
{
    m_reviewCount = data["count"].toInt();
    QJsonArray reviews = data["reviews"].toArray();

    beginInsertRows(QModelIndex(),
                    m_list.count(),
                    m_list.count()+reviews.count()-1);
    Q_FOREACH (const QJsonValue &reviewJson, reviews)
    {
        ReviewItem review(reviewJson.toObject());
        m_list.append(review);
    }
    endInsertRows();
    m_loadMorePending = false;
    Q_EMIT updated();
}

void ReviewsModel::handleReviewPosted(const QJsonObject &)
{
    Q_EMIT reviewPosted();
    Q_EMIT refresh();
}
