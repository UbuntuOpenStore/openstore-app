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

#include "packagekitsource.h"

#include "../apiconstants.h"
#include "../appstreampool.h"
#include "../indexstatus.h"
#include "../installers/packagekitinstaller.h"
#include "../openstorenetworkmanager.h"
#include "../packageindex.h"
#include "../packageitems/package.h"
#include "../packageitems/packagekitpackage.h"
#include "../platformintegration.h"
#include "../xapianindex.h"

#include <AppStreamQt/component.h>
#include <AppStreamQt/icon.h>

#include <QDebug>

#include <QSet>
#include <QSharedPointer>

#include <PackageKit/daemon.h>
#include <PackageKit/details.h>

PackageKitSource::PackageKitSource(QObject* parent)
  : PackageSource(parent)
  , m_installer(PlatformIntegration::instance()->packageKitInstaller())
{
  PackageIndex* index = PackageIndex::instance();
  m_pool = index->pool();
  connect(m_installer, &PackageKitInstaller::transactionFinished, this, &PackageKitSource::onInstallFinished);
  connect(index, &PackageIndex::buildCompleted, this, [this, index]() {
    m_indexBuilt = true;
    refreshInstalledInfo();
    Q_EMIT updated();

    if (m_searchPending) {
      requestSearch(m_lastRequest);
    }

    if (m_categoriesPending) {
      requestCategories();
    }

    if (!m_discoverPayload.isEmpty()) {
      parseDiscoverPayload();
    }
  });

  connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::parsedReply, this, &PackageKitSource::onStoreDiscoverReply);

  connect(OpenStoreNetworkManager::instance(),
          &OpenStoreNetworkManager::error,
          this,
          [this](const QString& signature, const QString& /*error*/, int /*statusCode*/) {
            if (signature != m_storeDiscoverSignature)
              return;

            if (m_hasDiscoverContent) {
              return;
            }

            // In the event of an error, prefer an empty discover reply over nothing at all.
            DiscoverReply empty;
            Q_EMIT discoverReplied(empty);
          });
}

PackageKitSource::~PackageKitSource()
{
  qDeleteAll(m_pkgCache.values());
}

bool PackageKitSource::busy() const
{
  return m_installer->busy();
}

QStringList PackageKitSource::sourceDescriptions() const
{
  return m_pool->repoDescriptions();
}

void PackageKitSource::requestSearch(const SearchRequest& request)
{
  m_lastRequest = request;

  // Discover carousels link here as "category:<slug>". Normalize into
  // m_lastRequest so both the initial reply and any refreshInstalledState()
  // re-emit the same filtered set.
  if (m_lastRequest.queryUrl.isValid() && m_lastRequest.queryUrl.scheme() == QStringLiteral("category"))
    m_lastRequest.category = m_lastRequest.queryUrl.path();

  if (!PackageIndex::instance()->xapian()->isAvailable()) {
    // The index is not built yet. Do not emit an empty reply
    m_searchPending = true;
    return;
  }

  m_searchPending = false;
  emitSearchReply();
}

void PackageKitSource::emitSearchReply(bool refresh)
{
  SearchReply reply;

  XapianIndex* xapian = PackageIndex::instance()->xapian();
  const int fetchAll = m_pool->componentCount();

  QList<SearchPackageItem> items;
  if (!m_lastRequest.category.isEmpty()) {
    const QStringList categories = categoriesForSlug(m_lastRequest.category);
    if (!categories.isEmpty())
      items = xapian->searchCategories(categories, 0, fetchAll, m_lastRequest.sortMode);
  } else {
    items = xapian->search(m_lastRequest.filterString, 0, fetchAll, m_lastRequest.sortMode);
  }

  reply.packages = m_lastSearchList = enrichList(items);
  reply.totalCount = reply.packages.count();
  reply.fetchedAll = true;
  reply.refresh = refresh;
  Q_EMIT searchReplied(reply);
}

QStringList PackageKitSource::categoriesForSlug(const QString& categoryId) const
{
  Q_FOREACH (const CategoryParser::Category& category, CategoryParser::categories()) {
    if (category.id == categoryId)
      return category.appstreamCategories;
  }
  return QStringList();
}

QList<SearchPackageItem> PackageKitSource::enrichList(const QList<SearchPackageItem>& items) const
{
  QList<SearchPackageItem> result;
  Q_FOREACH (SearchPackageItem item, items) {
    // All rows are Xapian hits with at least appId; pull the display fields from AppStream.
    const AppStream::Component component = m_pool->componentById(item.appId);
    if (!component.id().isEmpty()) {
      item.name = component.name();
      item.tagline = component.summary();
      const bool hasIcon = !component.icons().isEmpty() && !component.icons().first().url().isEmpty();
      item.icon = hasIcon ? component.icons().first().url().toString() : PlatformIntegration::instance()->fallbackIcon();
      item.packageType = QStringLiteral("packagekit");
    }
    // Installed sets are keyed by the binary package name; the appId is the
    // component id. Look up the name explicitly.
    const QString pkgName = component.packageNames().isEmpty() ? item.appId : component.packageNames().first();
    item.installed = installedVersionForPkgName(pkgName).isEmpty() == false;
    item.updateAvailable = isPackageUpdateAvailable(pkgName);
    result.append(item);
  }
  return result;
}

void PackageKitSource::requestCategories()
{
  if (!PackageIndex::instance()->xapian()->isAvailable()) {
    // The index is not built yet. Do not emit an empty reply.
    m_categoriesPending = true;
    return;
  }

  m_categoriesPending = false;

  const QList<CategoryParser::Category>& parsed = CategoryParser::categories();
  const QList<AppStream::Component> components = m_pool->allComponents();

  QList<CategoryItem> categories;
  Q_FOREACH (const CategoryParser::Category& category, parsed) {
    CategoryItem item;
    item.id = category.id;
    item.name = category.name;
    item.count = CategoryParser::countMatches(category, components);
    item.iconUrl = QUrl(category.icon);
    categories.append(item);
  }
  Q_EMIT categoriesReplied(categories);
}

void PackageKitSource::requestDiscover()
{
  m_storeDiscoverSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
  OpenStoreNetworkManager::instance()->getDiscoverV5(m_storeDiscoverSignature, API_PACKAGEKIT_PACKAGE_TYPE);
}

void PackageKitSource::onStoreDiscoverReply(const OpenStoreReply& reply)
{
  if (reply.signature != m_storeDiscoverSignature)
    return;

  m_discoverPayload = reply.data.toMap();
  if (!m_pool->isLoaded())
    return; // the pool is not ready yet; applied on buildCompleted

  parseDiscoverPayload();
}

static QString localComponentId(const QString& storeId)
{
  return storeId.startsWith(API_PACKAGEKIT_ID_PREFIX) ? storeId.mid(API_PACKAGEKIT_ID_PREFIX.size()) : storeId;
}

void PackageKitSource::parseDiscoverPayload()
{
  m_storeRatings.clear();
  m_lastHighlights.clear();

  // Highlights: keep every entry whose app resolves locally, in server order.
  const QVariantList highlights = m_discoverPayload.value("highlights").toList();
  Q_FOREACH (const QVariant& entry, highlights) {
    const QVariantMap highlight = entry.toMap();
    const QString appId = localComponentId(highlight.value("id").toString());
    qDebug() << "Discover highlight" << appId << highlight.value("image").toUrl();
    if (appId.isEmpty() || m_pool->componentById(appId).id().isEmpty())
      continue;

    qDebug() << "discover highlight exists";

    DiscoverHighlightItem item;
    item.appId = appId;
    item.imageUrl = highlight.value("image").toUrl();
    item.description = highlight.value("description").toString();
    m_lastHighlights.append(item);
    stashRatings(appId, highlight.value("app").toMap());
  }

  // Categories: server order, deduped, installable only.
  QList<DiscoverCategoryItem> categories;
  const QVariantList rawCategories = m_discoverPayload.value("categories").toList();
  Q_FOREACH (const QVariant& entry, rawCategories) {
    const QVariantMap category = entry.toMap();
    DiscoverCategoryItem item;
    item.name = category.value("name").toString();
    item.tagline = category.value("tagline").toString();
    item.queryUrl = queryUrlForReferral(category.value("referral").toString());

    const QVariantList apps = category.value("apps").toList();
    Q_FOREACH (const QVariant& appEntry, apps) {
      const QVariantMap app = appEntry.toMap();
      // Same strip as the highlights loop: the dedupe set and the ratings key
      // must be the bare id that requestPackageDetails() looks up later.
      const QString appId = localComponentId(app.value("id").toString());
      if (appId.isEmpty() || m_pool->componentById(appId).id().isEmpty())
        continue;

      item.appIds << appId;
      stashRatings(appId, app);
    }

    if (!item.appIds.isEmpty())
      categories.append(item);
  }

  DiscoverReply reply;
  reply.highlights = m_lastHighlights;
  reply.categories = categories;
  m_hasDiscoverContent = !reply.categories.isEmpty() || !reply.highlights.isEmpty();
  Q_EMIT discoverReplied(reply);
}

void PackageKitSource::stashRatings(const QString& appId, const QVariantMap& app)
{
  const QVariantMap ratings = app.value("ratings").toMap();
  if (!ratings.isEmpty())
    m_storeRatings.insert(appId, ratings);
}

QString PackageKitSource::queryUrlForReferral(const QString& referral) const
{
  const QString value = referral.trimmed();
  if (value.isEmpty())
    return QString();

  Q_FOREACH (const CategoryParser::Category& category, CategoryParser::categories()) {
    if (category.name.compare(value, Qt::CaseInsensitive) == 0 || category.id.compare(value, Qt::CaseInsensitive) == 0) {
      return QStringLiteral("category:%1").arg(category.id);
    }
  }

  return QString();
}

QList<LocalPackageItem> PackageKitSource::requestInstalled()
{
  // Start the async query only on the first call; the finished
  // transactions re-call this.
  if (!m_installedFresh)
    refreshInstalledInfo();

  QList<LocalPackageItem> result;
  QHashIterator<QString, QString> it(m_installedVersions);
  while (it.hasNext()) {
    it.next();
    const QString packageName = it.key();
    const QString componentId = PackageIndex::instance()->xapian()->componentIdForPkgName(packageName);
    if (componentId.isEmpty())
      continue; // skip packages without AppStream metadata

    const AppStream::Component component = m_pool->componentById(componentId);
    LocalPackageItem item;
    item.appId = componentId;
    item.name = component.name();
    const bool hasIcon = !component.icons().isEmpty() && !component.icons().first().url().isEmpty();
    item.icon = hasIcon ? component.icons().first().url().toString() : PlatformIntegration::instance()->fallbackIcon();
    item.version = it.value();
    item.packageType = QStringLiteral("packagekit");
    item.appLaunchUrl = QString();
    item.updateAvailable = isPackageUpdateAvailable(packageName);
    item.updateStatus = item.updateAvailable ? QStringLiteral("available") : QStringLiteral("none");
    // Fully qualified id from getPackages, which the daemon's API requires.
    item.packageUrl = m_installedPackageIds.value(packageName, packageName);
    result.append(item);
  }
  return result;
}

PackageItem* PackageKitSource::requestPackageDetails(const QString& id)
{
  if (id.trimmed().isEmpty())
    return 0;

  PackageKitPackageItem* pkg = 0;
  if (m_pkgCache.contains(id)) {
    pkg = m_pkgCache.value(id);
  } else {
    const AppStream::Component component = m_pool->componentById(id);
    if (component.id().isEmpty()) {
      Q_EMIT packageDetailsError(id);
      return 0;
    }

    pkg = new PackageKitPackageItem(component, this);
    m_pkgCache.insert(id, pkg);

    // Same fresh-gate as requestInstalled(); show cached state first.
    if (!m_installedFresh)
      refreshInstalledInfo();

    const QString packageName = pkg->packageName();
    pkg->setInstalledState(installedVersionForPkgName(packageName).isEmpty() == false,
                           installedVersionForPkgName(packageName),
                           isPackageUpdateAvailable(packageName));

    // Fetch the sizes asynchronously; repeat views hit the cache and skip the
    // query. The daemon requires a full package id, so resolve the name first
    // when needed. Both sizes come back in bytes.
    requestPackageSize(pkg, packageName);
  }

  const QVariantMap ratings = m_storeRatings.value(id);
  if (!ratings.isEmpty())
    pkg->setStoreMetadata(ratings);

  return pkg;
}

void PackageKitSource::refreshInstalledState()
{
  emitSearchReply(true);
}

void PackageKitSource::refresh()
{
  if (!m_indexBuilt)
    return;
  // Quiet background refresh: apt update then rebuild the pool/index if changed.
  m_installer->updateCache();
}

void PackageKitSource::onInstallFinished()
{
  if (m_installer->lastRole() == PackageKit::Transaction::RoleRefreshCache) {
    // A refresh-cache transaction finished: rebuild the catalog/index on the
    // worker thread. The pool and availability are updated by PackageIndex
    // when the build completes.
    PackageIndex::instance()->startBuild();
  } else {
    refreshInstalledInfo();
    refreshInstalledState(); // install/remove changed flags: re-emit with refresh=true
    Q_EMIT installedChanged();
    Q_EMIT updated();
  }
}

void PackageKitSource::syncCachedItemState()
{
  // Cached detail items never had their installed/update state recomputed after a transaction.
  for (auto it = m_pkgCache.constBegin(); it != m_pkgCache.constEnd(); ++it) {
    PackageKitPackageItem* pkg = it.value();
    if (!pkg) {
      continue;
    }

    const QString packageName = pkg->packageName();
    if (packageName.isEmpty()) {
      continue;
    }

    const QString installedVersion = installedVersionForPkgName(packageName);
    const bool updateAvailable = isPackageUpdateAvailable(packageName);
    const bool installed = !installedVersion.isEmpty();

    // setInstalledState() emits unconditionally; skip when nothing changed.
    if (pkg->installed() == installed && pkg->updateAvailable() == updateAvailable) {
      continue;
    }

    pkg->setInstalledState(installed, installedVersion, updateAvailable);
  }
}

void PackageKitSource::refreshInstalledInfo()
{
  // Skip if a query is already in flight; never stack transactions.
  if (!PackageIndex::instance()->xapian()->isAvailable() || m_installedTxsPending > 0)
    return;

  m_installedVersions.clear();
  m_installedPackageIds.clear();
  m_updatePackageIds.clear();

  auto finalize = [this]() {
    if (--m_installedTxsPending > 0)
      return;
    m_installedFresh = true;
    syncCachedItemState();
    Q_EMIT installedChanged();
    refreshInstalledState();
  };

  // Installed set (binary names with versions).
  PackageKit::Transaction* installedTx = PackageKit::Daemon::getPackages(PackageKit::Transaction::FilterInstalled);
  if (!installedTx) {
    qWarning() << "PackageKitSource: getPackages returned no transaction";
    return;
  }
  ++m_installedTxsPending;
  connect(installedTx,
          &PackageKit::Transaction::package,
          this,
          [this](PackageKit::Transaction::Info /*info*/, const QString& packageID, const QString& /*summary*/) {
            const QString name = PackageKit::Transaction::packageName(packageID);
            const QString version = PackageKit::Transaction::packageVersion(packageID);
            if (!name.isEmpty()) {
              m_installedVersions.insert(name, version);
              m_installedPackageIds.insert(name, packageID);
            }
          });
  connect(installedTx, &PackageKit::Transaction::finished, this, finalize);

  // Update set.
  PackageKit::Transaction* updatesTx = PackageKit::Daemon::getUpdates(PackageKit::Transaction::FilterNone);
  if (!updatesTx) {
    qWarning() << "PackageKitSource: getUpdates returned no transaction";
    return;
  }
  ++m_installedTxsPending;
  connect(updatesTx,
          &PackageKit::Transaction::package,
          this,
          [this](PackageKit::Transaction::Info /*info*/, const QString& packageID, const QString& /*summary*/) {
            m_updatePackageIds << packageID;
          });
  connect(updatesTx, &PackageKit::Transaction::finished, this, finalize);
}

QString PackageKitSource::installedVersionForPkgName(const QString& packageName) const
{
  return m_installedVersions.value(packageName, QString());
}

bool PackageKitSource::isPackageUpdateAvailable(const QString& packageName) const
{
  Q_FOREACH (const QString& packageID, m_updatePackageIds) {
    if (PackageKit::Transaction::packageName(packageID) == packageName)
      return true;
  }
  return false;
}

void PackageKitSource::requestPackageSize(PackageKitPackageItem* pkg, const QString& packageName)
{
  if (m_detailsFetched.contains(packageName))
    return;
  m_detailsFetched.insert(packageName);

  // Use the installed id when available; otherwise Resolve the name first (the daemon rejects bare names).
  const QString installedId = m_installedPackageIds.value(packageName);
  if (!installedId.isEmpty()) {
    pkg->setPackageId(installedId);
    startGetDetails(installedId, pkg, packageName);
    return;
  }

  PackageKit::Transaction* resolveTx = PackageKit::Daemon::resolve(packageName);
  if (!resolveTx)
    return;
  connect(
    resolveTx,
    &PackageKit::Transaction::package,
    this,
    [this, resolveTx, pkg, packageName](PackageKit::Transaction::Info /*info*/, const QString& packageID, const QString& /*summary*/) {
      if (PackageKit::Transaction::packageName(packageID) != packageName)
        return;
      disconnect(resolveTx, nullptr, this, nullptr);
      pkg->setPackageId(packageID);
      startGetDetails(packageID, pkg, packageName);
    });
}

void PackageKitSource::startGetDetails(const QString& packageId, PackageKitPackageItem* pkg, const QString& packageName)
{
  PackageKit::Transaction* detailsTx = PackageKit::Daemon::getDetails(packageId);
  if (!detailsTx)
    return;
  connect(detailsTx, &PackageKit::Transaction::details, this, [this, pkg, packageName](const PackageKit::Details& details) {
    if (PackageKit::Transaction::packageName(details.packageId()) != packageName)
      return;

    const qulonglong size = details.size();
    if (size != 0)
      pkg->setinstalledSize(int(size));

    // No named getter for this in the 1.1.4 binding; read the raw key
    const qulonglong downloadSize = details.value(QStringLiteral("download-size")).toULongLong();
    if (downloadSize != 0)
      pkg->setDownloadSize(int(downloadSize));
  });

  // Count direct Depends-only dependencies (non-recursive), deduped by name.
  PackageKit::Transaction* depsTx = PackageKit::Daemon::dependsOn(packageId, PackageKit::Transaction::FilterNone, false);
  if (!depsTx)
    return;
  const QSharedPointer<QSet<QString>> deps(new QSet<QString>);
  connect(depsTx,
          &PackageKit::Transaction::package,
          this,
          [deps](PackageKit::Transaction::Info /*info*/, const QString& packageID, const QString& /*summary*/) {
            const QString name = PackageKit::Transaction::packageName(packageID);
            if (!name.isEmpty())
              deps->insert(name);
          });
  connect(depsTx, &PackageKit::Transaction::finished, this, [this, pkg, deps]() { pkg->setDependencyCount(deps->count()); });
}
