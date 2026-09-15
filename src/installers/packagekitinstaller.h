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

#ifndef PACKAGEKITINSTALLER_H
#define PACKAGEKITINSTALLER_H

#include <QObject>
#include <QPointer>

#include <PackageKit/transaction.h>

class PackageKitInstaller : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
  Q_PROPERTY(QString lastError READ lastError NOTIFY packageInstallationFailed)

public:
  explicit PackageKitInstaller(QObject* parent = 0);

  bool busy() const { return m_busy; }
  int downloadProgress() const { return m_downloadProgress; }
  QString lastError() const { return m_lastError; }
  PackageKit::Transaction::Role lastRole() const { return m_lastRole; }

Q_SIGNALS:
  void busyChanged();
  void downloadProgressChanged();
  void packageInstalled();
  void packageInstallationFailed();
  void transactionFinished();
  void installCheckResult(const QString& packageId, bool installed, const QString& version);

public Q_SLOTS:
  Q_INVOKABLE void installPackage(const QString& packageId);
  Q_INVOKABLE void removePackage(const QString& packageId);
  void updateCache();
  void checkInstalled(const QString& packageId);
  Q_INVOKABLE bool abortInstallation() const;

private:
  void setupTransaction(PackageKit::Transaction* transaction);

private Q_SLOTS:
  void slotPercentageChanged();
  void slotStatusChanged();
  void slotFinished(PackageKit::Transaction::Exit status, uint runtime);
  void slotErrorCode(PackageKit::Transaction::Error error, const QString& details);
  void slotPackage(PackageKit::Transaction::Info info, const QString& packageID, const QString& summary);

private:
  bool m_busy = false;
  int m_downloadProgress = 0;
  QString m_lastError;
  PackageKit::Transaction::Role m_lastRole = PackageKit::Transaction::RoleUnknown;
  QPointer<PackageKit::Transaction> m_transaction;
  QString m_checkPackageId;
  bool m_checkFound = false; // set by slotPackage when the checked pkg is listed as installed
};

#endif // PACKAGEKITINSTALLER_H
