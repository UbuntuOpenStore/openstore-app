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

#ifndef APPSTREAMPOOL_H
#define APPSTREAMPOOL_H

#include <AppStreamQt/pool.h>
#include <QObject>

class AppStreamPool : public QObject
{
  Q_OBJECT
public:
  explicit AppStreamPool(QObject* parent = 0);

  bool load();
  AppStream::Component componentById(const QString& id) const;
  QStringList componentCategories() const;
  QList<AppStream::Component> componentsInCategory(const QString& categoryId) const;
  QList<AppStream::Component> allComponents() const;
  QStringList repoDescriptions() const;

private:
  AppStream::Pool m_pool;
  QList<AppStream::Component> m_components;
  bool m_loaded = false;
};

#endif // APPSTREAMPOOL_H