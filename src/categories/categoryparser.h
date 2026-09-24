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

#ifndef CATEGORYPARSER_H
#define CATEGORYPARSER_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

namespace AppStream {
class Component;
}

class CategoryParser
{
public:
  struct Category
  {
    QString id;
    QString name;
    QString icon;
    QStringList appstreamCategories;
  };

  static QList<Category> parse(const QByteArray& categoriesData);

  static QList<Category> loadFromResource();

  static const QList<Category>& categories();

  // Map a component's AppStream categories to the matching OpenStore category
  // id (empty when none match). The first AppStream category that is listed by
  // an OpenStore category decides; AppStream categories are ordered by
  // importance in the spec.
  static QString matchCategory(const QStringList& appstreamCategories);

  static QList<AppStream::Component> matchingComponents(const Category& category, const QList<AppStream::Component>& components);
  static int countMatches(const Category& category, const QList<AppStream::Component>& components);
};

#endif // CATEGORYPARSER_H
