#include "review.h"

#include <QDebug>

ReviewItem::ReviewItem(const QJsonObject &json, QObject * parent)
{
    m_author = json["author"].toString();
    m_body = json["body"].toString();
    m_rating = ratingFromString(json["rating"].toString());
    m_reviewedVersion = json["version"].toString();
    QJsonObject jsonComment = json["comment"].toObject();
    m_comment = Comment(jsonComment["body"].toString(), jsonComment["date"].toInt());
    m_isRedacted = json["redacted"].toBool();
    m_date = json["date"].toInt();
}

ReviewItem::ReviewItem(const ReviewItem &review)
    : m_author(review.m_author), m_body(review.m_body), m_rating(review.m_rating), m_reviewedVersion(review.m_reviewedVersion), m_comment(review.m_comment), m_isRedacted(review.m_isRedacted), m_date(review.m_date)
{
}

QString ReviewItem::ratingToString(enum Rating rating)
{
    return stringToRatingMap().key(rating);
}

Rating ReviewItem::ratingFromString(const QString &rating)
{
    return stringToRatingMap()[rating];
}

QString ReviewItem::id() const
{
    return m_reviewId;
}

QString ReviewItem::version() const
{
    return m_reviewedVersion;
}

Rating ReviewItem::rating() const
{
    return m_rating;
}

QString ReviewItem::body() const
{
    return m_body;
}

ReviewItem::Comment ReviewItem::comment() const
{
    return m_comment;
}

bool ReviewItem::redacted() const
{
    return m_isRedacted;
}

QString ReviewItem::author() const
{
    return m_author;
}

unsigned int ReviewItem::date() const
{
    return m_date;
}
