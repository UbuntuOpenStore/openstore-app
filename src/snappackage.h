/*
 * Copyright (C) 2020 Brian Douglass
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

#ifndef SNAPPACKAGE_H
#define SNAPPACKAGE_H

#include "package.h"

class SnapPackageItem : public PackageItem
{
  Q_OBJECT

public:
  explicit SnapPackageItem(const QVariantMap& json, QObject* parent = 0);
  ~SnapPackageItem();

  Q_INVOKABLE bool install() const override;
  Q_INVOKABLE bool remove() const override;
  Q_INVOKABLE QString appLaunchUrl() const override;

  virtual bool containsApp() const override { return m_containsApp; }
  virtual bool isLocalVersionSideloaded() const override { return false; }

private Q_SLOTS:
  virtual void fillData(const QVariantMap& json) override;

private:
  mutable bool m_containsApp;
};

#endif // SNAPPACKAGE_H
