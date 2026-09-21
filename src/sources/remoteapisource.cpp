/*
 * Copyright (C) 2026 Brian Douglass
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

#include "remoteapisource.h"

#include "../openstorenetworkmanager.h"
#include "../packageitems/package.h"
#include "../packagescache.h"
#include "../platformintegration.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSettings>
#include <algorithm>

#ifdef ENABLE_SNAP_SUPPORT
#include <Snapd/Client>
#endif

#define REQUEST_LIMIT 30

RemoteApiSource::RemoteApiSource(QObject* parent)
  : PackageSource(parent)
{
  connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::parsedReply, this, &RemoteApiSource::parseReply);
  // Forward cache-miss detail fetches; the guards route the reply
  // to the pending request only.
  connect(PackagesCache::instance(), &PackagesCache::packageDetailsReady, this, [this](PackageItem* pkg) {
    if (m_pendingDetailsAppId.isEmpty() || pkg->appId() != m_pendingDetailsAppId)
      return;
    m_pendingDetailsAppId.clear();
    Q_EMIT packageDetailsReady(pkg);
  });
  connect(PackagesCache::instance(), &PackagesCache::packageFetchError, this, [this](const QString& appId) {
    if (appId != m_pendingDetailsAppId)
      return;
    m_pendingDetailsAppId.clear();
    Q_EMIT packageDetailsError(appId);
  });
}

void RemoteApiSource::requestSearch(const SearchRequest& request)
{
  m_lastRequest = request;
  if (request.offset == 0) {
    // Fresh search; start a new accumulation.
    m_searchList.clear();
    m_searchFetchedAll = false;
    m_searchTotalCount = 0;
  }
  sendSearchRequest(request.offset);
}

void RemoteApiSource::sendSearchRequest(int skip)
{
  m_searchSignature = OpenStoreNetworkManager::instance()->generateNewSignature();

  if (!m_lastRequest.queryUrl.isEmpty()) {
    OpenStoreNetworkManager::instance()->getByUrl(m_searchSignature, m_lastRequest.queryUrl);
  } else if (m_lastRequest.filterString.isEmpty() && m_lastRequest.category.isEmpty()) {
    QString sortMode = QStringLiteral("-updated_date");
    if (!m_lastRequest.sortMode.isEmpty()) {
      sortMode = m_lastRequest.sortMode;
    }
    OpenStoreNetworkManager::instance()->getSearch(
      m_searchSignature, skip, REQUEST_LIMIT, QString(), QString(), sortMode, m_lastRequest.filterType, m_lastRequest.filterPackageType);
  } else {
    OpenStoreNetworkManager::instance()->getSearch(m_searchSignature,
                                                   skip,
                                                   REQUEST_LIMIT,
                                                   m_lastRequest.filterString,
                                                   m_lastRequest.category,
                                                   m_lastRequest.sortMode,
                                                   m_lastRequest.filterType,
                                                   m_lastRequest.filterPackageType);
  }
}

PackageItem* RemoteApiSource::requestPackageDetails(const QString& id)
{
  PackageItem* pkg = PackagesCache::instance()->get(id);
  // Cache hit: return synchronously without emitting; emitting opens a
  // detail page. On a miss, track the pending id; the forwarded PackagesCache
  // signals emit the reply when the fetch lands.
  if (!pkg) {
    m_pendingDetailsAppId = id;
    PackagesCache::instance()->getPackageDetails(id);
  }
  return pkg;
}

void RemoteApiSource::requestDiscover()
{
  m_discoverSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
  OpenStoreNetworkManager::instance()->getDiscover(m_discoverSignature);
}

void RemoteApiSource::requestCategories()
{
  m_categoriesSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
  OpenStoreNetworkManager::instance()->getCategories(m_categoriesSignature);
}

void RemoteApiSource::parseReply(const OpenStoreReply& reply)
{
  if (reply.signature == m_searchSignature) {
    QVariantMap data = reply.data.toMap();
    QVariantList pkgList = data.value("packages").toList();

    QList<SearchPackageItem> page;
    Q_FOREACH (const QVariant& pkg, pkgList) {
      const QVariantMap& pkgMap = pkg.toMap();
      SearchPackageItem item;
      item.appId = pkgMap.value("id").toString();
      item.name = pkgMap.value("name").toString();
      item.tagline = pkgMap.value("tagline").toString();
      item.icon = pkgMap.value("icon").toString();
      item.types = pkgMap.value("types").toStringList();
      item.packageType = pkgMap.value("package_type").toString();
      item.ratings = new Ratings(pkgMap.value("ratings").toMap());
      item.updateAvailable =
        bool(PackagesCache::instance()->getRemoteAppRevision(item.appId) > PackagesCache::instance()->getLocalAppRevision(item.appId));
      item.installed = !PlatformIntegration::instance()->appVersion(item.appId).isNull();
      m_searchList.append(item);
      page.append(item);
    }

    m_searchFetchedAll = !data.value("next").toUrl().isValid();
    m_searchTotalCount = m_searchFetchedAll ? m_searchList.count() : m_searchList.count() + 1;

    SearchReply replyStruct;
    replyStruct.packages = page;
    replyStruct.totalCount = m_searchTotalCount;
    replyStruct.fetchedAll = m_searchFetchedAll;
    replyStruct.refresh = false;
    Q_EMIT searchReplied(replyStruct);
  } else if (reply.signature == m_discoverSignature) {
    QVariantMap data = reply.data.toMap();

    DiscoverReply discover;
    QVariantMap highlight = data.value("highlight").toMap();
    discover.highlightAppId = highlight.value("id").toString();
    discover.highlightBannerUrl = highlight.value("image").toUrl();

    if (!discover.highlightAppId.isEmpty() && !PackagesCache::instance()->contains(discover.highlightAppId)) {
      PackagesCache::instance()->insert(discover.highlightAppId, highlight.value("app").toMap());
    }

    QVariantList categories = data.value("categories").toList();
    Q_FOREACH (const QVariant& categoryVariant, categories) {
      const QVariantMap& category = categoryVariant.toMap();
      DiscoverCategoryItem item;
      item.name = category.value("name").toString();
      item.tagline = category.value("tagline").toString();
      item.queryUrl = category.value("query_url").toString();
      item.appIds = category.value("ids").toStringList();
      discover.categories.append(item);

      QVariantList catAppsList = category.value("apps").toList();
      Q_FOREACH (const QVariant& appVariant, catAppsList) {
        const QVariantMap& app = appVariant.toMap();
        const QString& appId = app.value("id").toString();
        if (!PackagesCache::instance()->contains(appId)) {
          PackagesCache::instance()->insert(appId, app);
        }
      }
    }
    Q_EMIT discoverReplied(discover);
  } else if (reply.signature == m_categoriesSignature) {
    QVariantList data = reply.data.toList();
    QList<CategoryItem> items;
    Q_FOREACH (const QVariant& cat, data) {
      const QVariantMap& catMap = cat.toMap();
      CategoryItem catItem;
      catItem.id = catMap.value("category").toString();
      catItem.name = catMap.value("translation", catItem.id).toString();
      catItem.count = catMap.value("count").toInt();
      catItem.iconUrl = catMap.value("icon").toUrl();
      items.append(catItem);
    }
    Q_EMIT categoriesReplied(items);
  }
}

QList<LocalPackageItem> RemoteApiSource::requestInstalled()
{
  QList<LocalPackageItem> result;

#ifdef ENABLE_CLICK_SUPPORT
  const QVariantList& clickDb = PlatformIntegration::instance()->clickDb();
  Q_FOREACH (const QVariant& pkg, clickDb) {
    QVariantMap map = pkg.toMap();
    QString appId = map.value("name").toString();
    QString version = map.value("version").toString();

    QVariantMap hookMap = map.value("hooks").toMap();
    QString appLaunchUrl;
    Q_FOREACH (const QString& key, hookMap.keys()) {
      QVariantMap hook = hookMap.value(key).toMap();
      if (hook.keys().contains("desktop")) {
        appLaunchUrl = QString("appid://%1/%2/current-user-version").arg(appId).arg(key);
      }
    }

    LocalPackageItem pkgItem;
    pkgItem.appId = appId;
    pkgItem.name = map.value("title").toString();
    pkgItem.version = version;
    pkgItem.packageUrl = PackagesCache::instance()->getPackageUrl(pkgItem.appId);
    pkgItem.appLaunchUrl = appLaunchUrl;
    pkgItem.packageType = QStringLiteral("click");

    int remoteRevision = PackagesCache::instance()->getRemoteAppRevision(pkgItem.appId);
    int localRevision = PackagesCache::instance()->getLocalAppRevision(pkgItem.appId);
    pkgItem.updateAvailable = bool(remoteRevision > localRevision);

    if (localRevision == 0) {
      pkgItem.updateStatus = QStringLiteral("downgrade");
    } else if (pkgItem.updateAvailable) {
      pkgItem.updateStatus = QStringLiteral("available");
    } else {
      pkgItem.updateStatus = QStringLiteral("none");
    }

    if (pkgItem.icon.isEmpty()) {
      const QString& directory = map.value("_directory").toString();
      const QVariantMap& hooks = map.value("hooks").toMap();
      Q_FOREACH (const QString& hook, hooks.keys()) {
        const QVariantMap& h = hooks.value(hook).toMap();
        const QString& desktop = h.value("desktop").toString();
        if (!desktop.isEmpty()) {
          const QString& desktopFile = directory + QDir::separator() + desktop;
          QSettings appInfo(desktopFile, QSettings::IniFormat);
          pkgItem.icon = directory + QDir::separator() + appInfo.value("Desktop Entry/Icon").toString();
          break;
        }
      }
      if (!pkgItem.icon.isEmpty()) {
        pkgItem.icon = pkgItem.icon.prepend("file://");
      }
    }

    result.append(pkgItem);
  }
#endif

#ifdef ENABLE_SNAP_SUPPORT
  QSnapdClient* installer = PlatformIntegration::instance()->snapInstaller();
  if (installer && OpenStoreNetworkManager::instance()->snapSupport()) {
    auto request = installer->getSnaps();
    request->runSync();
    for (int i = 0; i < request->snapCount(); i++) {
      QSnapdSnap* snap = request->snap(i);
      LocalPackageItem pkgItem;
      pkgItem.appId = QStringLiteral("snap.") + snap->name();
      pkgItem.packageType = QStringLiteral("snap");
      pkgItem.name = snap->title().isEmpty() ? snap->name() : snap->title();
      pkgItem.version = snap->version();
      const QString iconPath = snap->icon();
      if (iconPath.startsWith(QStringLiteral("http://")) || iconPath.startsWith(QStringLiteral("https://"))) {
        pkgItem.icon = iconPath;
      } else {
        const QString base = QStringLiteral("/snap/") + snap->name() + QStringLiteral("/current/meta/gui/icon");
        if (QFileInfo::exists(base + QStringLiteral(".svg"))) {
          pkgItem.icon = QStringLiteral("file://") + base + QStringLiteral(".svg");
        } else if (QFileInfo::exists(base + QStringLiteral(".png"))) {
          pkgItem.icon = QStringLiteral("file://") + base + QStringLiteral(".png");
        } else {
          pkgItem.icon = QStringLiteral("qrc:/Assets/fallback.svg");
        }
      }
      pkgItem.updateStatus = QStringLiteral("snap");
      result.append(pkgItem);
    }
  }
#endif

  return result;
}

void RemoteApiSource::refreshInstalledState()
{
  // Recompute flags over the full list; the model merges them by appId.
  for (int i = 0; i < m_searchList.count(); ++i) {
    m_searchList[i].updateAvailable = bool(PackagesCache::instance()->getRemoteAppRevision(m_searchList[i].appId) >
                                           PackagesCache::instance()->getLocalAppRevision(m_searchList[i].appId));
    m_searchList[i].installed = !PlatformIntegration::instance()->appVersion(m_searchList[i].appId).isNull();
  }

  SearchReply replyStruct;
  replyStruct.packages = m_searchList;
  replyStruct.totalCount = m_searchTotalCount;
  replyStruct.fetchedAll = m_searchFetchedAll;
  replyStruct.refresh = true;
  Q_EMIT searchReplied(replyStruct);
}