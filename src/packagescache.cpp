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

#include "packagescache.h"
#include "package.h"

#include "openstorenetworkmanager.h"
#include "platformintegration.h"

PackagesCache* PackagesCache::m_instance = nullptr;

PackagesCache::PackagesCache()
{
  m_updatingCache = false;
  connect(PlatformIntegration::instance(), &PlatformIntegration::updated, this, &PackagesCache::updateCacheRevisions);
}

bool PackagesCache::contains(const QString& appId) const
{
  return m_cache.contains(appId);
}

PackageItem* PackagesCache::insert(const QString& appId, const QVariantMap& jsonMap)
{
  PackageItem* pkg = new PackageItem(jsonMap, this);
  pkg->updateLocalInfo(m_localAppRevision.value(pkg->appId()), PlatformIntegration::instance()->appVersion(pkg->appId()));

  m_cache.insert(appId, pkg);
  return pkg;
}

PackageItem* PackagesCache::get(const QString& appId) const
{
  return m_cache.value(appId, Q_NULLPTR);
}

void PackagesCache::getPackageDetails(const QString& appId, bool bust)
{
  if (contains(appId) && !bust) {
    Q_EMIT packageDetailsReady(get(appId));
  } else {
    const QString& requestSignature = OpenStoreNetworkManager::instance()->generateNewSignature();

    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::parsedReply, [=](const OpenStoreReply& reply) {
      if (reply.signature != requestSignature)
        return;

      QVariantMap pkg = reply.data.toMap();

      PackageItem* pkgItem = insert(appId, pkg);
      Q_EMIT packageDetailsReady(pkgItem);
    });
    connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::error, [=](const QString& signature, const QString& error) {
      if (signature != requestSignature)
        return;

      Q_EMIT packageFetchError(appId);
    });

    OpenStoreNetworkManager::instance()->getAppDetails(requestSignature, appId);
  }
}

void PackagesCache::updateCacheRevisions()
{
  m_updatingCache = true;
  Q_EMIT updatingCacheChanged();

  // qDebug() << Q_FUNC_INFO << "called";

  connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::parsedReply, [=](const OpenStoreReply& reply) {
    if (reply.signature != m_signature)
      return;

    m_localAppRevision.clear();
    m_remoteAppRevision.clear();
    m_packageUrls.clear();

    QVariantList data = reply.data.toList();
    Q_FOREACH (QVariant d, data) {
      QVariantMap map = d.toMap();
      const QString& appId = map.value("id").toString();
      m_localAppRevision.insert(appId, map.value("revision").toInt());
      m_remoteAppRevision.insert(appId, map.value("latest_revision").toInt());
      m_packageUrls.insert(appId, map.value("download_url").toString());
    }

    Q_FOREACH (PackageItem* pkg, m_cache) {
      int pkgRevNo = m_localAppRevision.value(pkg->appId(), -1);
      pkg->updateLocalInfo(pkgRevNo, PlatformIntegration::instance()->appVersion(pkg->appId()));
    }

    m_updatingCache = false;
    Q_EMIT updatingCacheChanged();
  });

  QStringList appIdsAtRevisionList;

  Q_FOREACH (const QString& appId, PlatformIntegration::instance()->installedAppIds()) {
    const QString& version = PlatformIntegration::instance()->appVersion(appId);
    appIdsAtRevisionList.append(QString("%1@%2").arg(appId, version));
  }

  m_signature = OpenStoreNetworkManager::instance()->generateNewSignature();
  OpenStoreNetworkManager::instance()->getRevisions(m_signature, appIdsAtRevisionList);
}
