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

#ifndef CATEGORIESMODEL_H
#define CATEGORIESMODEL_H

#include "../packageSource.h"
#include <QAbstractListModel>

class CategoriesModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(bool ready READ ready NOTIFY updated)

public:
  enum Roles
  {
    RoleId,
    RoleName,
    RoleCount,
    RoleIconUrl,
  };

  explicit CategoriesModel(QObject* parent = 0);

  QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
  int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

  bool ready() const { return m_ready; }

  Q_INVOKABLE void update();

Q_SIGNALS:
  void updated();

private Q_SLOTS:
  void parseReply(const QList<CategoryItem>& categories);

private:
  PackageSource* m_source;
  QList<CategoryItem> m_list;
  bool m_ready;
};

#endif // CATEGORIESMODEL_H
