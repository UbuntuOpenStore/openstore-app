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

bool AppStreamPool::load()
{
  m_loaded = m_pool.load();
  if (!m_loaded) {
    qWarning() << "AppStreamPool: failed to load AppStream metadata";
    return false;
  }

  m_components = m_pool.components().toList();
  m_components.erase(std::remove_if(m_components.begin(),
                                    m_components.end(),
                                    [](const AppStream::Component& c) {
                                      const AppStream::Component::Kind kind = c.kind();
                                      return kind != AppStream::Component::KindDesktopApp && kind != AppStream::Component::KindConsoleApp &&
                                             kind != AppStream::Component::KindWebApp;
                                    }),
                     m_components.end());
  qDebug() << "AppStreamPool: loaded" << m_components.size() << "application components";
  return true;
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
