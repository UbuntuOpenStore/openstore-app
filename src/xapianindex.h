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

#ifndef XAPIANINDEX_H
#define XAPIANINDEX_H

#include "packageSource.h"
#include <QDateTime>

namespace AppStream {
class Component;
}

class XapianIndex : public QObject
{
  Q_OBJECT
public:
  explicit XapianIndex(QObject* parent = 0);
  ~XapianIndex();

  bool isAvailable() const;
  QString errorMessage() const;

  bool build(const QList<AppStream::Component>& components);

  QList<SearchPackageItem> search(const QString& queryText, int offset, int limit);
  QList<SearchPackageItem> allInCategory(const QString& categoryId, int offset, int limit);
  QList<CategoryItem> categories() const;
  int totalMatches() const;
  QString componentIdForPkgName(const QString& packageName) const;

  QDateTime lastBuilt() const;

Q_SIGNALS:
  void indexBuilt();
  void indexError(const QString& message);

private:
  QString databasePath() const;
  void rebuildDatabase(const QList<AppStream::Component>& components);
  // Xapian terms allow only [a-z0-9_]; normalize on both write and query
  // sides because component ids and package names contain '.', '-' and '+'.
  static QString sanitizeTerm(const QString& text);
  static SearchPackageItem packageItemFromDocument(const QByteArray& data);

  bool m_available = false;
  QString m_errorMessage;
  QString m_databasePath;
  int m_totalMatches = 0;
  QDateTime m_lastBuilt;
};

#endif // XAPIANINDEX_H