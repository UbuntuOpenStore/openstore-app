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

#ifndef PACKAGEKITSOURCE_H
#define PACKAGEKITSOURCE_H

#include "../categories/categoryparser.h"
#include "../packageSource.h"

#include <QHash>
#include <QSet>
#include <QStringList>

struct OpenStoreReply;

class AppStreamPool;
class PackageKitPackageItem;
class PackageIndex;
class PackageKitInstaller;

class PackageKitSource : public PackageSource
{
  Q_OBJECT
public:
  explicit PackageKitSource(QObject* parent = 0);
  ~PackageKitSource();

  void requestSearch(const SearchRequest& request) override;
  void requestDiscover() override;
  void requestCategories() override;
  PackageItem* requestPackageDetails(const QString& id) override;
  QList<LocalPackageItem> requestInstalled() override;
  void refreshInstalledState() override;
  void refresh() override;
  bool busy() const override;
  QStringList sourceDescriptions() const override;

private:
  void refreshInstalledInfo();
  void emitSearchReply(bool refresh = false);
  QList<SearchPackageItem> enrichList(const QList<SearchPackageItem>& items) const;
  QString installedVersionForPkgName(const QString& packageName) const;
  bool isPackageUpdateAvailable(const QString& packageName) const;
  void requestPackageSize(PackageKitPackageItem* pkg, const QString& packageName);
  void startGetDetails(const QString& packageId, PackageKitPackageItem* pkg, const QString& packageName);
  // Refresh cached items' installed/update state once the installed sets are fresh.
  void syncCachedItemState();

  void parseDiscoverPayload();
  void stashRatings(const QString& appId, const QVariantMap& app);
  QString queryUrlForReferral(const QString& referral) const;

  // The AppStream category terms backing an OpenStore category slug; empty
  // when the slug is unknown. XapianIndex::searchCategories() matches any of
  // them (exact boolean OR).
  QStringList categoriesForSlug(const QString& categoryId) const;

private Q_SLOTS:
  void onInstallFinished();
  void onStoreDiscoverReply(const OpenStoreReply& reply);

private:
  AppStreamPool* m_pool;
  PackageKitInstaller* m_installer;

  QHash<QString, PackageKitPackageItem*> m_pkgCache; // componentId -> item
  QHash<QString, QString> m_installedVersions;       // packageName -> version
  QHash<QString, QString> m_installedPackageIds;     // packageName -> full package id
  QStringList m_updatePackageIds;                    // PackageKit "name;version;arch;data"

  QVariantMap m_discoverPayload;
  QList<DiscoverHighlightItem> m_lastHighlights;
  QHash<QString, QVariantMap> m_storeRatings; // appId -> ratings map from the payload
  bool m_hasDiscoverContent = false;          // true once a non-empty reply was emitted
  bool m_installedFresh = false;              // installed/update sets populated at least once
  int m_installedTxsPending = 0;              // outstanding getPackages/getUpdates transactions
  QSet<QString> m_detailsFetched;             // package names with a GetDetails query already issued

  SearchRequest m_lastRequest;
  QList<SearchPackageItem> m_lastSearchList;
  bool m_searchPending = false;     // search requested before the index was built
  bool m_categoriesPending = false; // categories requested before the index was built

  QString m_storeDiscoverSignature;
  bool m_indexBuilt = false; // pool+xapian built at least once; gates quiet refresh()
};

#endif // PACKAGEKITSOURCE_H
