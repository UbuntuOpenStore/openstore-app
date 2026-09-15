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

#ifndef BACKENDINSTALLER_H
#define BACKENDINSTALLER_H

#include <QObject>

class BackendInstaller : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QObject* installer READ installer CONSTANT)
  Q_PROPERTY(QObject* indexStatus READ indexStatus CONSTANT)

public:
  static BackendInstaller* instance();

  QObject* installer() const { return m_installer; }
  QObject* indexStatus() const { return m_indexStatus; }

private:
  BackendInstaller();

  QObject* m_installer;
  QObject* m_indexStatus;

  static BackendInstaller* m_instance;
};

#endif // BACKENDINSTALLER_H
