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

#include "review.h"

#include <QDebug>

    Ratings::Ratings(const QMap <QString, QVariant> &map, QObject *parent)
    : QObject(parent)
{
    m_thumbsUpCount = map[ratingToString(RatingThumbsUp)].toInt();
    m_thumbsDownCount = map[ratingToString(RatingThumbsDown)].toInt();
    m_neutralCount = map[ratingToString(RatingNeutral)].toInt();
    m_happyCount = map[ratingToString(RatingHappy)].toInt();
    m_buggyCount = map[ratingToString(RatingBuggy)].toInt();
}

Ratings::Ratings(QObject * parent)
    : QObject(parent)
{
}

Ratings::Ratings(const Ratings &ratings)
    : QObject(Q_NULLPTR),
      m_thumbsUpCount(ratings.thumbsUpCount()),
      m_thumbsDownCount(ratings.thumbsDownCount()),
      m_neutralCount(ratings.neutralCount()),
      m_happyCount(ratings.happyCount()),
      m_buggyCount(ratings.buggyCount())
{
}

unsigned int Ratings::thumbsUpCount() const
{
    return m_thumbsUpCount;
}

unsigned int Ratings::thumbsDownCount() const
{
    return m_thumbsDownCount;
}

unsigned int Ratings::neutralCount() const
{
    return m_neutralCount;
}

unsigned int Ratings::happyCount() const
{
    return m_happyCount;
}

unsigned int Ratings::buggyCount() const
{
    return m_buggyCount;
}

unsigned int Ratings::totalCount() const
{
    return m_thumbsUpCount + m_thumbsDownCount + m_neutralCount + m_happyCount + m_buggyCount;
}

QString Ratings::ratingToString(enum Rating rating)
{
    return stringToRatingMap().key(rating);
}

Rating Ratings::ratingFromString(const QString &rating)
{
    return stringToRatingMap()[rating];
}

QMap<QString, Rating> & Ratings::stringToRatingMap()
{
    static QMap<QString, Rating> map
    {
        {"THUMBS_UP", RatingThumbsUp},
        {"THUMBS_DOWN", RatingThumbsDown},
        {"NEUTRAL", RatingNeutral},
        {"HAPPY", RatingHappy},
        {"BUGGY", RatingBuggy}
    };
    return map;
}


ReviewItem::ReviewItem(const QJsonObject &json, QObject * parent)
    : QObject(parent)
{
    m_reviewId = json["id"].toString();
    m_author = json["author"].toString();
    m_body = json["body"].toString();
    m_rating = Ratings::ratingFromString(json["rating"].toString());
    m_reviewedVersion = json["version"].toString();
    QJsonObject jsonComment = json["comment"].toObject();
    m_comment = Comment(jsonComment["body"].toString(), jsonComment["date"].toVariant().toLongLong());
    m_isRedacted = json["redacted"].toBool();
    m_date = json["date"].toVariant().toLongLong();
}

ReviewItem::ReviewItem(const ReviewItem &review)
    : QObject(Q_NULLPTR), m_author(review.m_author), m_body(review.m_body), m_rating(review.m_rating), m_reviewedVersion(review.m_reviewedVersion), m_comment(review.m_comment), m_isRedacted(review.m_isRedacted), m_date(review.m_date)
{
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

qlonglong ReviewItem::date() const
{
    return m_date;
}
