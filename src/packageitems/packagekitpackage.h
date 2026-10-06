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

#ifndef PACKAGEKITPACKAGE_H
#define PACKAGEKITPACKAGE_H

#include "package.h"

#include <AppStreamQt/component.h>

class PackageKitPackageItem : public PackageItem
{
  Q_OBJECT
public:
  explicit PackageKitPackageItem(const AppStream::Component& component, QObject* parent = 0);

  bool install() const override;
  bool remove() const override;
  bool launch() const override;

  QString packageName() const { return m_packageName; }
  // PackageKit wants a full id ("name;version;arch;data"); fall back to the name until the id is resolved.
  QString installTarget() const { return m_packageId.isEmpty() ? m_packageName : m_packageId; }
  void setPackageId(const QString& packageId) { m_packageId = packageId; }

  void setInstalledState(bool installed, const QString& version, bool updateAvailable);
  void setinstalledSize(int installedSize) override;
  void setDownloadSize(int downloadSize) override;
  void setDependencyCount(int dependencyCount) override;
  void setStoreMetadata(const QVariantMap& ratingsMap);

protected:
  void fillData(const QVariantMap& json) override;

private:
  QString m_componentId;
  QString m_packageName;
  QString m_packageId;
  QString m_desktopEntry;
};

#endif // PACKAGEKITPACKAGE_H
