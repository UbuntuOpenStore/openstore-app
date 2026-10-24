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

#ifndef LOCALPACKAGESMODEL_H
#define LOCALPACKAGESMODEL_H

#include "../packageSource.h"
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class PackageItem;

class LocalPackagesModel : public QAbstractListModel
{
  Q_OBJECT
  Q_DISABLE_COPY(LocalPackagesModel)
  Q_PROPERTY(int count READ rowCount NOTIFY updated)
  Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
  Q_PROPERTY(int updatesAvailableCount READ updatesAvailableCount NOTIFY updated)
  Q_PROPERTY(int downgradesAvailableCount READ downgradesAvailableCount NOTIFY updated)
  Q_PROPERTY(int snapsCount READ snapsCount NOTIFY updated)
  Q_PROPERTY(int systemUpdatesCount READ systemUpdatesCount NOTIFY updated)
  Q_PROPERTY(QAbstractItemModel* availableUpdatesModel READ availableUpdatesModel CONSTANT)
  Q_PROPERTY(QAbstractItemModel* downgradeAppsModel READ downgradeAppsModel CONSTANT)
  Q_PROPERTY(QAbstractItemModel* installedAppsModel READ installedAppsModel CONSTANT)
  Q_PROPERTY(QAbstractItemModel* snapAppsModel READ snapAppsModel CONSTANT)
  Q_PROPERTY(QString appStoreAppId MEMBER m_appStoreAppId NOTIFY appStoreAppIdChanged)
  Q_PROPERTY(bool appStoreUpdateAvailable READ appStoreUpdateAvailable NOTIFY appStoreUpdateAvailableChanged)

public:
  enum Roles
  {
    RoleName,
    RoleAppId,
    RoleVersion,
    RoleIcon,
    RoleUpdateAvailable,
    RoleUpdateStatus,
    RolePackageUrl,
    RoleUpdatePackageId,
    RoleAppLaunchUrl,
    RoleLaunch,
    RolePackageType,
  };

  explicit LocalPackagesModel(QAbstractListModel* parent = 0);

  QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
  int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

  bool ready() const { return m_ready; }
  int updatesAvailableCount() const;
  int downgradesAvailableCount() const;
  int snapsCount() const;
  int systemUpdatesCount() const;

  QAbstractItemModel* availableUpdatesModel() const { return m_availableUpdatesModel; }
  QAbstractItemModel* downgradeAppsModel() const { return m_downgradeAppsModel; }
  QAbstractItemModel* installedAppsModel() const { return m_installedAppsModel; }
  QAbstractItemModel* snapAppsModel() const { return m_snapAppsModel; }

  bool appStoreUpdateAvailable() const { return m_appStoreUpdateAvailable; }

  Q_INVOKABLE void refresh();
  Q_INVOKABLE QVariantMap get(int row);
  Q_INVOKABLE QVariantMap getByAppId(const QString& appId);
  Q_INVOKABLE QVariantList systemUpdates();

Q_SIGNALS:
  void countChanged();
  void readyChanged();
  void updated();
  void appStoreAppIdChanged();
  void appStoreUpdateAvailableChanged();

private:
  QSortFilterProxyModel* createFilteredModel(const QString& updateStatus);

  QList<LocalPackageItem> m_list;
  PackageSource* m_source;
  QString m_appStoreAppId;
  bool m_ready;
  bool m_appStoreUpdateAvailable;
  QSortFilterProxyModel* m_availableUpdatesModel;
  QSortFilterProxyModel* m_downgradeAppsModel;
  QSortFilterProxyModel* m_installedAppsModel;
  QSortFilterProxyModel* m_snapAppsModel;
};

#endif // LOCALPACKAGESMODEL_H
