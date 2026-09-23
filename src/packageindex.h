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

#ifndef PACKAGEINDEX_H
#define PACKAGEINDEX_H

#include <QList>
#include <QObject>
#include <QThread>

namespace AppStream {
class Component;
}

class AppStreamPool;
class IndexBuilder;
class IndexStatus;
class XapianIndex;

class PackageIndex : public QObject
{
  Q_OBJECT
public:
  explicit PackageIndex(QObject* parent = 0);
  ~PackageIndex();

  static PackageIndex* instance()
  {
    if (!m_instance) {
      m_instance = new PackageIndex();
    }
    return m_instance;
  }

  IndexStatus* status() const { return m_indexStatus; }
  XapianIndex* xapian() const { return m_xapian; }

  Q_INVOKABLE void initialize();

  AppStreamPool* pool() const { return m_pool; }
  void startBuild();

Q_SIGNALS:
  void buildCompleted();

private Q_SLOTS:
  void onBuildSucceeded(const QList<AppStream::Component>& components);
  void onBuildFailed(const QString& message);
  void onProgress(int percent);

private:
  IndexStatus* m_indexStatus;
  XapianIndex* m_xapian;
  AppStreamPool* m_pool;
  IndexBuilder* m_builder;
  QThread m_workerThread;
  bool m_buildInFlight = false;

  static PackageIndex* m_instance;
};

#endif // PACKAGEINDEX_H
