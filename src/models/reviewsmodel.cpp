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

#include "reviewsmodel.h"
#include "../review.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ReviewsModel::ReviewsModel(const QString& appId, QObject* parent)
  : QAbstractListModel(parent)
  , m_appId(appId)
  , m_loadMorePending(false)
{
  connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::parsedReply, this, &ReviewsModel::parseReply);
  connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::error, this, &ReviewsModel::parseError);
  connect(this, &ReviewsModel::refresh, this, &ReviewsModel::onRefresh);

  m_appendSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
  OpenStoreNetworkManager::instance()->getReviews(m_appendSignature, appId);
}

int ReviewsModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return m_list.count();
}

QVariant ReviewsModel::data(const QModelIndex& index, int role) const
{
  const ReviewItem& item = m_list.at(index.row());

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
  if (m_list.count() == m_reviewCount) {
    return;
  }
  if (m_loadMorePending) {
    return;
  }
  m_loadMorePending = true;
  m_appendSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
  OpenStoreNetworkManager::instance()->getReviews(m_appendSignature, m_appId, 10, m_list.constLast().date());
}

void ReviewsModel::getOwnReview(const QString& apiKey)
{
  m_ownSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
  OpenStoreNetworkManager::instance()->getReviews(m_ownSignature, m_appId, apiKey);
}

unsigned int ReviewsModel::reviewCount() const
{
  return m_reviewCount;
}

bool ReviewsModel::sendReview(const QString& version,
                              const QString& review,
                              Ratings::Rating rating,
                              const QString& apiKey,
                              const bool& edit)
{
  m_postedSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
  OpenStoreNetworkManager::instance()->postReview(m_postedSignature, m_appId, version, review, rating, apiKey, edit);

  return true;
}

void ReviewsModel::parseReply(OpenStoreReply reply)
{
  QJsonObject data = QJsonObject::fromVariantMap(reply.data.toMap());
  if (reply.signature == m_appendSignature) {
    handleAppendReviews(data);
  } else if (reply.signature == m_postedSignature) {
    handleReviewPosted(data);
  } else if (reply.signature == m_resetSignature) {
    handleResetReviews(data);
  } else if (reply.signature == m_ownSignature) {
    handleOwnReview(data);
  }
}

void ReviewsModel::parseError(const QString& signature, const QString& error)
{
  if (signature == m_appendSignature || signature == m_postedSignature || signature == m_resetSignature || signature == m_ownSignature) {
    Q_EMIT ReviewsModel::error(error);
  }
}

void ReviewsModel::onRefresh()
{
  m_resetSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
  OpenStoreNetworkManager::instance()->getReviews(m_resetSignature, m_appId);
}

void ReviewsModel::handleOwnReview(const QJsonObject& data)
{
  QJsonArray reviews = data["reviews"].toArray();
  if (reviews.count() > 0) {
    QJsonObject review(reviews.first().toObject());
    Q_EMIT ownReviewResponse(review, Ratings::ratingFromString(review["rating"].toString()));
  } else {
    Q_EMIT ownReviewResponse(QJsonObject(), Ratings::Rating::RatingNone);
  }
}

void ReviewsModel::handleResetReviews(const QJsonObject& data)
{
  m_reviewCount = data["count"].toInt();
  QJsonArray reviews = data["reviews"].toArray();

  beginResetModel();
  m_list.clear();
  Q_FOREACH (const QJsonValue& reviewJson, reviews) {
    ReviewItem review(reviewJson.toObject());
    m_list.append(review);
  }
  endResetModel();
  Q_EMIT updated();
}

void ReviewsModel::handleAppendReviews(const QJsonObject& data)
{
  m_reviewCount = data["count"].toInt();
  QJsonArray reviews = data["reviews"].toArray();

  beginInsertRows(QModelIndex(), m_list.count(), m_list.count() + reviews.count() - 1);
  Q_FOREACH (const QJsonValue& reviewJson, reviews) {
    ReviewItem review(reviewJson.toObject());
    m_list.append(review);
  }
  endInsertRows();
  m_loadMorePending = false;
  Q_EMIT updated();
}

void ReviewsModel::handleReviewPosted(const QJsonObject&)
{
  Q_EMIT reviewPosted();
  Q_EMIT refresh();
}
