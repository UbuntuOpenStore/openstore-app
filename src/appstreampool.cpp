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

#include "appstreampool.h"

#include <QDebug>
#include <QSet>

#include <algorithm>

AppStreamPool::AppStreamPool(QObject* parent)
  : QObject(parent)
{
}

QList<AppStream::Component> AppStreamPool::loadFromDisk()
{
  QList<AppStream::Component> components;
  AppStream::Pool pool;
  if (!pool.load()) {
    qWarning() << "AppStreamPool: failed to load AppStream metadata";
    return components;
  }

  components = pool.components().toList();
  components.erase(std::remove_if(components.begin(),
                                  components.end(),
                                  [](const AppStream::Component& c) {
                                    const AppStream::Component::Kind kind = c.kind();
                                    return kind != AppStream::Component::KindDesktopApp && kind != AppStream::Component::KindConsoleApp &&
                                           kind != AppStream::Component::KindWebApp;
                                  }),
                   components.end());
  qDebug() << "AppStreamPool: loaded" << components.size() << "application components";
  return components;
}

void AppStreamPool::setComponents(const QList<AppStream::Component>& components)
{
  m_components = components;
  m_loaded = true;
}

AppStream::Component AppStreamPool::componentById(const QString& id) const
{
  Q_FOREACH (const AppStream::Component& c, m_components) {
    if (c.id() == id)
      return c;
  }
  Q_FOREACH (const AppStream::Component& c, m_components) {
    if (c.packageNames().contains(id))
      return c;
  }
  return AppStream::Component();
}

QStringList AppStreamPool::componentCategories() const
{
  QSet<QString> seen;
  Q_FOREACH (const AppStream::Component& c, m_components) {
    Q_FOREACH (const QString& cat, c.categories()) {
      seen.insert(cat);
    }
  }
  return seen.values();
}

QList<AppStream::Component> AppStreamPool::componentsInCategory(const QString& categoryId) const
{
  QList<AppStream::Component> result;
  Q_FOREACH (const AppStream::Component& c, m_components) {
    if (c.categories().contains(categoryId))
      result.append(c);
  }
  return result;
}

QList<AppStream::Component> AppStreamPool::allComponents() const
{
  return m_components;
}

QStringList AppStreamPool::repoDescriptions() const
{
  return QStringList() << QStringLiteral("/var/cache/swcatalog/xmls");
}
