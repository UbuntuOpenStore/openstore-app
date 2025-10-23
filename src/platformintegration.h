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

#include <QHash>
#include <QObject>
#include <QVariantList>

class ClickInstaller;

class PlatformIntegration : public QObject
{
  Q_OBJECT
  Q_PROPERTY(ClickInstaller* clickInstaller READ clickInstaller CONSTANT)

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

  ClickInstaller* clickInstaller() const { return m_installer; }

  QStringList supportedFrameworks() const { return m_supportedFrameworks; }
  QString supportedArchitecture() const { return m_supportedArchitecture; }
  QString systemLocale() const { return m_systemLocale; }
  QString systemCodename() const { return m_systemCodename; }

  QString appVersion(const QString& appId) const { return m_installedAppIds.value(appId, QString()); }
  QStringList installedAppIds() const { return m_installedAppIds.keys(); }

  QVariantList clickDb() const { return m_clickDb; }

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
  QVariantList m_clickDb;

  ClickInstaller* m_installer;

  static PlatformIntegration* m_instance;
};

#endif // PLATFORMINTEGRATION_H
