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

#ifndef PACKAGESOURCE_H
#define PACKAGESOURCE_H

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QUrl>

#include "review.h"

class PackageItem;

struct CategoryItem
{
  QString id;
  QString name;
  int count = 0;
  QUrl iconUrl;
};

struct SearchPackageItem
{
  QString name;
  QString appId;
  QString icon;
  QPointer<Ratings> ratings;
  QString tagline;
  bool installed = false;
  bool updateAvailable = false;
  QStringList types;
  QString packageType;
};

struct DiscoverCategoryItem
{
  QString name;
  QString tagline;
  QString queryUrl;
  QStringList appIds;
};

struct SearchRequest
{
  QString filterString;
  QString category;
  QString sortMode;
  QString filterType;
  QString filterPackageType;
  int offset = 0;
  int limit = 30;
  QString lang;
  QUrl queryUrl;
};

struct SearchReply
{
  QList<SearchPackageItem> packages;
  int totalCount = 0;
  bool fetchedAll = false;
  // True when the reply replaces the rows instead of appending a page.
  // Emitted only by refreshInstalledState(), never by a fresh search.
  bool refresh = false;
};

struct DiscoverReply
{
  QString highlightAppId;
  QUrl highlightBannerUrl;
  QList<DiscoverCategoryItem> categories;
};

struct LocalPackageItem
{
  QString name;
  QString appId;
  QString version;
  QString icon;
  bool updateAvailable = false;
  QString updateStatus; // "none" | "available" | "downgrade" | "snap"
  QString packageUrl;   // click: download URL; packagekit: PackageKit package id passed to installer
  QString appLaunchUrl;
  QString packageType;
};

class PackageSource : public QObject
{
  Q_OBJECT
public:
  explicit PackageSource(QObject* parent = 0);
  ~PackageSource();

  virtual void requestSearch(const SearchRequest& request) = 0;
  virtual void requestDiscover() = 0;
  virtual void requestCategories() = 0;
  virtual Q_INVOKABLE PackageItem* requestPackageDetails(const QString& id) = 0;
  virtual QList<LocalPackageItem> requestInstalled() = 0;
  virtual void refreshInstalledState() = 0;
  virtual void refresh() {}
  virtual bool busy() const { return false; }
  virtual QStringList sourceDescriptions() const { return QStringList(); }

Q_SIGNALS:
  void searchReplied(const SearchReply& reply);
  void discoverReplied(const DiscoverReply& reply);
  void categoriesReplied(const QList<CategoryItem>& categories);
  void packageDetailsReady(PackageItem* pkg);
  void packageDetailsError(const QString& appId);
  void installedChanged();
  void updated();
  void busyChanged();

protected:
  void setBusyChanged() { Q_EMIT busyChanged(); }
};

Q_DECLARE_METATYPE(SearchRequest)
Q_DECLARE_METATYPE(SearchReply)
Q_DECLARE_METATYPE(DiscoverReply)

#endif // PACKAGESOURCE_H
