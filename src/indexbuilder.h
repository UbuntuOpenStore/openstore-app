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

#ifndef INDEXBUILDER_H
#define INDEXBUILDER_H

#include <QList>
#include <QObject>
#include <QString>

namespace AppStream {
class Component;
}

class IndexBuilder : public QObject
{
  Q_OBJECT
public:
  explicit IndexBuilder(QObject* parent = 0);

public Q_SLOTS:
  // Runs in the worker thread. Uses its own AppStream::Pool instance and
  // writes the Xapian database at databasePath in place.
  void build(const QString& databasePath);

Q_SIGNALS:
  void buildSucceeded(const QList<AppStream::Component>& components);
  void buildFailed(const QString& message);
  void progressChanged(int percent);
};

#endif // INDEXBUILDER_H