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

#include <PackageKit/daemon.h>

PackageKitSource::PackageKitSource(QObject* parent)
  : PackageSource(parent)
  , m_pool(new AppStreamPool(this))
  , m_installer(PlatformIntegration::instance()->packageKitInstaller())
{
  PackageIndex* index = PackageIndex::instance();
  connect(m_installer, &PackageKitInstaller::transactionFinished, this, &PackageKitSource::onInstallFinished);
  connect(index->status(), &IndexStatus::stateChanged, this, [this, index]() {
    // Use stateChanged, not ready(); ready() emits only after a build
    // succeeds and re-wiring it can re-enter the build.
    if (index->status()->state() != QStringLiteral("initializing"))
      return;
    if (!m_pool->load()) {
      index->status()->setError(QStringLiteral("Failed to read AppStream metadata."), true);
      return;
    }
    if (!index->build(m_pool->allComponents()))
      return; // build() already set the error state
    m_indexBuilt = true;
    refreshInstalledInfo();
    Q_EMIT updated();
  });
  connect(OpenStoreNetworkManager::instance(), &OpenStoreNetworkManager::parsedReply, this, &PackageKitSource::onStoreDiscoverReply);
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
  if (!PackageIndex::instance()->xapian()->isAvailable()) {
    SearchReply empty;
    empty.fetchedAll = true;
    Q_EMIT searchReplied(empty);
    return;
  }

  emitSearchReply();
}

void PackageKitSource::emitSearchReply(bool refresh)
{
  SearchReply reply;

  XapianIndex* xapian = PackageIndex::instance()->xapian();
  const bool hasFilter = !m_lastRequest.filterString.isEmpty();
  const bool hasCategory = !m_lastRequest.category.isEmpty();

  if (hasCategory) {
    reply.packages = m_lastSearchList =
      enrichList(xapian->allInCategory(m_lastRequest.category, m_lastRequest.offset, m_lastRequest.limit));
  } else if (hasFilter) {
    reply.packages = m_lastSearchList = enrichList(xapian->search(m_lastRequest.filterString, m_lastRequest.offset, m_lastRequest.limit));
  } else {
    // Browse-all: empty query. Passing QString() (not "*") selects MatchAll
    // in XapianIndex::search, giving docid order.
    reply.packages = m_lastSearchList = enrichList(xapian->search(QString(), m_lastRequest.offset, m_lastRequest.limit));
  }

  reply.totalCount = xapian->totalMatches();
  reply.fetchedAll = (m_lastRequest.offset + reply.packages.count()) >= reply.totalCount;
  reply.refresh = refresh;
  Q_EMIT searchReplied(reply);
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
      if (!component.icons().isEmpty())
        item.icon = component.icons().first().url().toString();
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
  QList<CategoryItem> categories;
  XapianIndex* xapian = PackageIndex::instance()->xapian();
  if (xapian->isAvailable()) {
    categories = xapian->categories();
  }
  Q_EMIT categoriesReplied(categories);
}

void PackageKitSource::requestDiscover()
{
  DiscoverReply reply;

  XapianIndex* xapian = PackageIndex::instance()->xapian();
  if (xapian->isAvailable()) {
    for (int i = 0; i < xapian->categories().count() && reply.categories.count() < 6; ++i) {
      const CategoryItem& cat = xapian->categories().at(i);
      DiscoverCategoryItem item;
      item.name = cat.name;
      item.queryUrl = QStringLiteral("category:%1").arg(cat.id);
      Q_FOREACH (const SearchPackageItem& pkg, xapian->allInCategory(cat.id, 0, 8)) {
        item.appIds << pkg.appId;
      }
      reply.categories.append(item);
    }
  }

  m_lastDiscoverCategories = reply.categories;
  Q_EMIT discoverReplied(reply);

  // Ask the store for the discover highlight; use it only if a
  // local component matches (onStoreDiscoverReply).
  if (!m_storeDiscoverPending) {
    m_storeDiscoverPending = true;
    m_storeDiscoverSignature = OpenStoreNetworkManager::instance()->generateNewSignature();
    OpenStoreNetworkManager::instance()->getDiscover(m_storeDiscoverSignature);
  }
}

void PackageKitSource::onStoreDiscoverReply(const OpenStoreReply& reply)
{
  if (reply.signature != m_storeDiscoverSignature)
    return;
  m_storeDiscoverPending = false;

  const QVariantMap highlight = reply.data.toMap().value("highlight").toMap();
  const QString storeId = highlight.value("id").toString();
  if (storeId.isEmpty())
    return;

  // Re-emit discover with the highlight if the component id matches.
  const AppStream::Component component = m_pool->componentById(storeId);
  if (!component.id().isEmpty()) {
    m_highlightAppId = component.id();
    m_highlightBannerUrl = highlight.value("image").toUrl();

    DiscoverReply replyStruct;
    replyStruct.highlightAppId = m_highlightAppId;
    replyStruct.highlightBannerUrl = m_highlightBannerUrl;
    replyStruct.categories = m_lastDiscoverCategories;
    Q_EMIT discoverReplied(replyStruct);
  }
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
    item.icon = component.icons().isEmpty() ? QString() : component.icons().first().url().toString();
    item.version = it.value();
    item.packageType = QStringLiteral("packagekit");
    item.appLaunchUrl = QString();
    item.updateAvailable = isPackageUpdateAvailable(packageName);
    item.updateStatus = item.updateAvailable ? QStringLiteral("available") : QStringLiteral("none");
    item.packageUrl = QStringLiteral("%1").arg(packageName); // install target passed to PackageKitInstaller
    result.append(item);
  }
  return result;
}

PackageItem* PackageKitSource::requestPackageDetails(const QString& id)
{
  if (m_pkgCache.contains(id))
    return m_pkgCache.value(id);

  const AppStream::Component component = m_pool->componentById(id);
  if (component.id().isEmpty()) {
    Q_EMIT packageDetailsError(id);
    return 0;
  }

  PackageKitPackageItem* pkg = new PackageKitPackageItem(component, this);
  m_pkgCache.insert(id, pkg);

  // Same fresh-gate as requestInstalled(); show cached state first.
  if (!m_installedFresh)
    refreshInstalledInfo();

  const QString packageName = pkg->packageName();
  pkg->setInstalledState(installedVersionForPkgName(packageName).isEmpty() == false,
                         installedVersionForPkgName(packageName),
                         isPackageUpdateAvailable(packageName));

  // Return synchronously without emitting packageDetailsReady; emitting
  // pushes an unwanted detail page.
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
    // A refresh-cache transaction finished: re-read AppStream and rebuild.
    if (m_pool->load()) {
      PackageIndex::instance()->build(m_pool->allComponents());
    }
    refreshInstalledInfo();
    Q_EMIT updated();
    Q_EMIT installedChanged();
  } else {
    refreshInstalledInfo();
    refreshInstalledState(); // install/remove changed flags: re-emit with refresh=true
    Q_EMIT installedChanged();
    Q_EMIT updated();
  }
}

void PackageKitSource::refreshInstalledInfo()
{
  // Skip if a query is already in flight; never stack transactions.
  if (!PackageIndex::instance()->xapian()->isAvailable() || m_installedTxsPending > 0)
    return;

  m_installedVersions.clear();
  m_updatePackageIds.clear();

  // When the last transaction finishes, mark the set fresh and re-emit.
  auto finalize = [this]() {
    if (--m_installedTxsPending > 0)
      return;
    m_installedFresh = true;
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
            if (!name.isEmpty())
              m_installedVersions.insert(name, version);
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
