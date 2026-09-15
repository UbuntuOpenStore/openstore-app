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

#ifndef CLICKPACKAGE_H
#define CLICKPACKAGE_H

#include "package.h"

class ClickPackageItem : public PackageItem
{
  Q_OBJECT

public:
  explicit ClickPackageItem(const QVariantMap& json, QObject* parent = 0);
  ~ClickPackageItem();

  Q_INVOKABLE bool install() const override;
  Q_INVOKABLE bool remove() const override;
  Q_INVOKABLE QString appLaunchUrl() const override;

private Q_SLOTS:
  virtual void fillData(const QVariantMap& json) override;
};

#endif // CLICKPACKAGE_H
