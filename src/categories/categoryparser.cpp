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

#include "categoryparser.h"

#include <AppStreamQt/component.h>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

QList<CategoryParser::Category> CategoryParser::loadFromResource()
{
  QFile categoriesFile(QStringLiteral(":/categories/categories.json"));
  if (!categoriesFile.open(QIODevice::ReadOnly)) {
    qWarning() << "CategoryParser: could not open"
               << ":/categories/categories.json"
               << ":" << categoriesFile.errorString();
    return QList<Category>();
  }

  return parse(categoriesFile.readAll());
}

const QList<CategoryParser::Category>& CategoryParser::categories()
{
  static const QList<Category> cached = loadFromResource();
  return cached;
}

QString CategoryParser::matchCategory(const QStringList& appstreamCategories)
{
  const QList<Category>& all = categories();
  Q_FOREACH (const QString& term, appstreamCategories) {
    Q_FOREACH (const Category& category, all) {
      if (category.appstreamCategories.contains(term))
        return category.id;
    }
  }
  return QString();
}

QList<CategoryParser::Category> CategoryParser::parse(const QByteArray& categoriesData)
{
  QList<Category> result;

  QJsonParseError error;
  const QJsonDocument categoriesDoc = QJsonDocument::fromJson(categoriesData, &error);
  if (error.error != QJsonParseError::NoError || !categoriesDoc.isObject()) {
    qWarning() << "CategoryParser: failed to parse the categories file:" << error.errorString();
    return result;
  }

  const QJsonArray data = categoriesDoc.object().value("categories").toArray();
  Q_FOREACH (const QJsonValue& value, data) {
    const QJsonObject json = value.toObject();
    const QString slug = json.value("slug").toString();
    if (slug.isEmpty())
      continue;

    Category category;
    category.id = slug;
    category.name = json.value("name").toString();
    if (category.name.isEmpty())
      category.name = slug;

    const QString iconFile = json.value("icon").toString();
    category.icon = QStringLiteral("qrc:/categories/icons/%1").arg(iconFile);

    const QJsonArray appstreamCategories = json.value("packagekitCategories").toArray();
    Q_FOREACH (const QJsonValue& catValue, appstreamCategories) {
      const QString cat = catValue.toString();
      if (!cat.isEmpty())
        category.appstreamCategories.append(cat);
    }

    result.append(category);
  }

  return result;
}

QList<AppStream::Component> CategoryParser::matchingComponents(const Category& category, const QList<AppStream::Component>& components)
{
  QList<AppStream::Component> result;
  Q_FOREACH (const AppStream::Component& component, components) {
    const QStringList componentCategories = component.categories();
    Q_FOREACH (const QString& cat, category.appstreamCategories) {
      if (componentCategories.contains(cat)) {
        result.append(component);
        break;
      }
    }
  }

  return result;
}

int CategoryParser::countMatches(const Category& category, const QList<AppStream::Component>& components)
{
  int count = 0;
  Q_FOREACH (const AppStream::Component& component, components) {
    const QStringList componentCategories = component.categories();
    Q_FOREACH (const QString& cat, category.appstreamCategories) {
      if (componentCategories.contains(cat)) {
        ++count;
        break;
      }
    }
  }

  return count;
}
