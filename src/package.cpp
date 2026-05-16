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

#include "package.h"

PackageItem::PackageItem(const QVariantMap& json, QObject* parent)
  : QObject(parent), m_downloadProgress(0)
{
}

PackageItem::~PackageItem()
{
  // qDebug() << Q_FUNC_INFO << m_appId;
}

void PackageItem::updateLocalInfo(int localRevision, const QString& localVersion) const
{
  m_installedVersion = localVersion;
  m_installedRevision = localRevision;

  Q_EMIT updated();
  Q_EMIT installedChanged();
}

bool PackageItem::review(const QString& review, Rating rating, const QString& apiKey) const
{
  return m_reviews->sendReview(m_version, review, rating, apiKey, false);
}

bool PackageItem::editReview(const QString& review, Rating rating, const QString& apiKey) const
{
  return m_reviews->sendReview(m_version, review, rating, apiKey, true);
}
