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

#ifndef PACKAGEBACKENDMANAGER_H
#define PACKAGEBACKENDMANAGER_H

#include <QObject>

class PackageItem;
class PackageSource;
#ifdef ENABLE_DEB_SUPPORT
class PackageKitSource;
#endif

class PackageBackendManager : public QObject
{
  Q_OBJECT
  Q_PROPERTY(PackageSource* activeSource READ activeSource CONSTANT)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
  static PackageBackendManager* instance();

  PackageSource* activeSource() const { return m_source; }
  bool busy() const { return m_busy; }

  Q_INVOKABLE void refreshCatalog();
  Q_INVOKABLE void getPackageDetails(const QString& appId);
  Q_INVOKABLE QStringList sources() const;

Q_SIGNALS:
  void busyChanged();
  void updated();
  void packageDetailsReady(PackageItem* pkg);
  void packageDetailsError(const QString& appId);

private:
  PackageBackendManager();

  PackageSource* m_source;
  bool m_busy;

  static PackageBackendManager* m_instance;
};

#endif // PACKAGEBACKENDMANAGER_H
