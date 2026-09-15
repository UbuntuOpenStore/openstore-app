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

#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QAbstractListModel>
#include <QUrl>

#include "../packageSource.h"
#include "../packageitems/package.h"
#include "../review.h"

class SearchModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY updated)

  Q_PROPERTY(QString filterString MEMBER m_filterString NOTIFY filterStringChanged)
  Q_PROPERTY(QString category MEMBER m_category NOTIFY categoryChanged)
  Q_PROPERTY(QString sortMode MEMBER m_sortMode NOTIFY sortModeChanged)
  Q_PROPERTY(QString filterType MEMBER m_filterType NOTIFY filterTypeChanged)
  Q_PROPERTY(QString filterPackageType MEMBER m_filterPackageType NOTIFY filterPackageTypeChanged)
  Q_PROPERTY(QUrl queryUrl MEMBER m_queryUrl NOTIFY queryUrlChanged)

public:
  enum Roles
  {
    RoleName,
    RoleAppId,
    RoleIcon,
    RoleRatings,
    RoleTagline,
    RoleInstalled,
    RoleUpdateAvailable,
    RoleTypes,
    RolePackageType,
  };

  explicit SearchModel(QObject* parent = 0);

  int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
  QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

  Q_INVOKABLE int find(const QString& appId) const;

Q_SIGNALS:
  void filterStringChanged();
  void categoryChanged();
  void queryUrlChanged();
  void sortModeChanged();
  void filterTypeChanged();
  void filterPackageTypeChanged();

  void updated();

protected:
  bool canFetchMore(const QModelIndex& parent) const Q_DECL_OVERRIDE;
  void fetchMore(const QModelIndex& parent) Q_DECL_OVERRIDE;

private Q_SLOTS:
  void update();
  void searchReplied(const SearchReply& reply);

private:
  QString m_filterString;
  QString m_category;
  QString m_sortMode;
  QString m_filterType;
  QString m_filterPackageType;
  QUrl m_queryUrl;

  bool m_fetchedAll;

  QList<SearchPackageItem> m_list;
  PackageSource* m_source;
};

#endif // SEARCHMODEL_H
