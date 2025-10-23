/*
 * Copyright (C) 2014 Canonical, Ltd.
 * Copyright (C) 2017 Stefano Verzegnassi
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

#include "cachingnetworkmanagerfactory.h"
#include "apiconstants.h"

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QStandardPaths>

CachingNetworkAccessManager::CachingNetworkAccessManager(QObject* parent)
  : QNetworkAccessManager(parent)
{
}

QNetworkReply* CachingNetworkAccessManager::createRequest(Operation op, const QNetworkRequest& request, QIODevice* outgoingData)
{
  const QUrl& requestedUrl = request.url();

  // Use cache only for images
  if (requestedUrl.fileName().indexOf(".png") || requestedUrl.fileName().indexOf(".svg") || requestedUrl.fileName().indexOf(".jpg") ||
      requestedUrl.fileName().indexOf(".jpeg")) {
    QNetworkRequest req(request);
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    return QNetworkAccessManager::createRequest(op, req, outgoingData);
  }

  return QNetworkAccessManager::createRequest(op, request, outgoingData);
}

CachingNetworkManagerFactory::CachingNetworkManagerFactory() {}

QNetworkAccessManager* CachingNetworkManagerFactory::create(QObject* parent)
{
  QNetworkAccessManager* manager = new CachingNetworkAccessManager(parent);

  QString domain = qgetenv("OPENSTORE_DOMAIN");
  if (domain.isEmpty()) {
    domain = STORE_DOMAIN;
  }

  for (int i = 0; i < 6; ++i) {
    manager->connectToHostEncrypted(domain);
  }

  QNetworkDiskCache* cache = new QNetworkDiskCache(manager);
  cache->setCacheDirectory(QStringLiteral("%1/qml_cache").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));

  manager->setCache(cache);
  return manager;
}
