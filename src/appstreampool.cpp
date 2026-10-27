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

#include "platformintegration.h"

#include <QDebug>
#include <QIcon>
#include <QSet>
#include <QUrl>

#include <AppStreamQt/icon.h>

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

QString AppStreamPool::iconSource(const AppStream::Component& component)
{
  QString localPath;
  int localSize = 0;
  QString stockName;
  QString remoteUrl;

  const QList<AppStream::Icon> icons = component.icons();
  Q_FOREACH (const AppStream::Icon& icon, icons) {
    switch (icon.kind()) {
      case AppStream::Icon::KindCached:
      case AppStream::Icon::KindLocal: {
        // libappstream resolves these to an absolute path in the AppStream icon
        // cache. The cache holds several sizes per app, so keep the largest one.
        const QString path = icon.url().toLocalFile();
        if (path.isEmpty())
          break;

        const int size = qMax(icon.width(), icon.height());
        if (size > localSize) {
          localSize = size;
          localPath = path;
        }

        break;
      }
      case AppStream::Icon::KindStock:
        if (stockName.isEmpty())
          stockName = icon.name();
        break;
      case AppStream::Icon::KindRemote:
        if (remoteUrl.isEmpty())
          remoteUrl = icon.url().toString();
        break;
      default:
        break;
    }
  }

  if (!localPath.isEmpty())
    return QUrl::fromLocalFile(localPath).toString();

  if (!stockName.isEmpty()) {
    QString name = stockName;
    if (name.endsWith(QStringLiteral(".desktop")))
      name.chop(8);

    if (!name.isEmpty() && !QIcon::fromTheme(name).isNull())
      return QStringLiteral("image://theme/") + name;
  }

  if (!remoteUrl.isEmpty())
    return remoteUrl;

  return PlatformIntegration::instance()->fallbackIcon();
}
