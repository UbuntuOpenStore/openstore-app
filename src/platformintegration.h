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

#ifndef PLATFORMINTEGRATION_H
#define PLATFORMINTEGRATION_H

#ifdef ENABLE_CLICK_SUPPORT
#include "installers/clickinstaller.h"
#endif

#include <QHash>
#include <QObject>
#include <QVariantList>

#ifdef ENABLE_SNAP_SUPPORT
#include <Snapd/Client>
#endif

#ifdef ENABLE_DEB_SUPPORT
class PackageKitInstaller;
#endif

class PlatformIntegration : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool clickSupport READ clickSupport CONSTANT)
  Q_PROPERTY(bool snapSupport READ snapSupport CONSTANT)
  Q_PROPERTY(bool debSupport READ debSupport CONSTANT)
#ifdef ENABLE_CLICK_SUPPORT
  Q_PROPERTY(ClickInstaller* clickInstaller READ clickInstaller CONSTANT)
#endif
#ifdef ENABLE_SNAP_SUPPORT
  Q_PROPERTY(QSnapdClient* snapInstaller READ snapInstaller CONSTANT)
#endif
#ifdef ENABLE_DEB_SUPPORT
  Q_PROPERTY(PackageKitInstaller* packageKitInstaller READ packageKitInstaller CONSTANT)
#endif

public:
  PlatformIntegration();
  ~PlatformIntegration();

  static PlatformIntegration* instance()
  {
    if (!m_instance) {
      m_instance = new PlatformIntegration();
    }
    return m_instance;
  }

#ifdef ENABLE_CLICK_SUPPORT
  ClickInstaller* clickInstaller() const
  {
    return m_installer;
  }
#endif

#ifdef ENABLE_SNAP_SUPPORT
  QSnapdClient* snapInstaller() const
  {
    return m_snapInstaller;
  }
#endif

  bool clickSupport() const
  {
#ifdef ENABLE_CLICK_SUPPORT
    return true;
#else
    return false;
#endif
  }

  bool snapSupport() const
  {
#ifdef ENABLE_SNAP_SUPPORT
    return m_snapInstaller != nullptr;
#else
    return false;
#endif
  }

  bool debSupport() const
  {
#ifdef ENABLE_DEB_SUPPORT
    return true;
#else
    return false;
#endif
  }

#ifdef ENABLE_DEB_SUPPORT
  PackageKitInstaller* packageKitInstaller() const
  {
    return m_packageKitInstaller;
  }
#endif

  QStringList supportedFrameworks() const
  {
    return m_supportedFrameworks;
  }

  QString supportedArchitecture() const
  {
    return m_supportedArchitecture;
  }

  QString systemLocale() const
  {
    return m_systemLocale;
  }

  QString systemCodename() const
  {
    return m_systemCodename;
  }

  QString appVersion(const QString& appId) const
  {
    return m_installedAppIds.value(appId, QString());
  }

  QStringList installedAppIds() const
  {
    return m_installedAppIds.keys();
  }

#ifdef ENABLE_CLICK_SUPPORT
  QVariantList clickDb() const
  {
    return m_clickDb;
  }
#endif

Q_SIGNALS:
  void updated();

public Q_SLOTS:
  void update();

private:
  QStringList getSupportedFrameworks();
  QString getSupportedArchitecture();
  QString getSystemLocale();
  QString getSystemCodename();

private:
  QStringList m_supportedFrameworks;
  QString m_supportedArchitecture;
  QString m_systemLocale;
  QString m_systemCodename;

  QHash<QString, QString> m_installedAppIds; // appid, version
#ifdef ENABLE_CLICK_SUPPORT
  QVariantList m_clickDb;

  ClickInstaller* m_installer;
#endif
#ifdef ENABLE_SNAP_SUPPORT
  QSnapdClient* m_snapInstaller;
#endif
#ifdef ENABLE_DEB_SUPPORT
  PackageKitInstaller* m_packageKitInstaller;
#endif

  static PlatformIntegration* m_instance;
};

#endif // PLATFORMINTEGRATION_H
