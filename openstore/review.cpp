#include "review.h"

#include <QDebug>

ReviewItem::ReviewItem(const QJsonObject &json, QObject * parent)
{
    m_author = json["author"].toString();
    m_body = json["body"].toString();
    m_date = json["date"].toInt();
    m_rating = ratingFromString(json["rating"].toString());
    m_reviewedVersion = json["version"].toString();
    m_isRedacted = json["redacted"].toBool();
}

ReviewItem::ReviewItem(const ReviewItem &review)
{
    m_author = review.m_author;
    m_body = review.m_body;
    m_date = review.m_date;
    m_rating = review.m_rating;
    m_reviewedVersion = review.m_reviewedVersion;
    m_isRedacted = review.m_isRedacted;
    m_comment = review.m_comment;
}

QString ReviewItem::ratingToString(enum Rating rating)
{
    switch (rating)
    {
        case RatingThumbsUp:
            return "THUMBS_UP";
        case RatingThumbsDown:
            return "THUMBS_DOWN";
        case RatingNeutral:
            return "NEUTRAL";
        case RatingHappy:
            return "HAPPY";
        case RatingBuggy:
            return "BUGGY";
    }
    qDebug() << "Error: Unkown rating enum value: " << rating;
    return "";
}

Rating ReviewItem::ratingFromString(const QString &rating)
{
    return RatingNeutral;
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

QString ReviewItem::comment() const
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
