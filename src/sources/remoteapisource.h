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

#ifndef REMOTEAPISOURCE_H
#define REMOTEAPISOURCE_H

#include "../packageSource.h"

struct OpenStoreReply;

class RemoteApiSource : public PackageSource
{
  Q_OBJECT
public:
  explicit RemoteApiSource(QObject* parent = 0);

  void requestSearch(const SearchRequest& request) override;
  void requestDiscover() override;
  void requestCategories() override;
  PackageItem* requestPackageDetails(const QString& id) override;
  QList<LocalPackageItem> requestInstalled() override;
  void refreshInstalledState() override;

private:
  void sendSearchRequest(int skip);

private Q_SLOTS:
  void parseReply(const OpenStoreReply& reply);

private:
  QString m_searchSignature;
  QString m_discoverSignature;
  QString m_categoriesSignature;
  QString m_pendingDetailsAppId; // non-empty while an async getPackageDetails() is in flight

  SearchRequest m_lastRequest;
  // Full search list across fetchMore pages; cleared on a new search and
  // re-emitted by refreshInstalledState() to update flags in place.
  QList<SearchPackageItem> m_searchList;
  bool m_searchFetchedAll = false;
  int m_searchTotalCount = 0;
};

#endif // REMOTEAPISOURCE_H