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

#include "packagekitinstaller.h"

#include <PackageKit/daemon.h>

#include <QDebug>

PackageKitInstaller::PackageKitInstaller(QObject* parent)
  : QObject(parent)
{
}

void PackageKitInstaller::setupTransaction(PackageKit::Transaction* transaction)
{
  if (!transaction)
    return;

  m_transaction = transaction;
  m_lastRole = transaction->role();
  m_lastError.clear();
  m_downloadProgress = 0;

  connect(transaction, &PackageKit::Transaction::percentageChanged, this, &PackageKitInstaller::slotPercentageChanged);
  connect(transaction, &PackageKit::Transaction::statusChanged, this, &PackageKitInstaller::slotStatusChanged);
  connect(transaction, &PackageKit::Transaction::finished, this, &PackageKitInstaller::slotFinished);
  connect(transaction, &PackageKit::Transaction::errorCode, this, &PackageKitInstaller::slotErrorCode);
  connect(transaction, &PackageKit::Transaction::package, this, &PackageKitInstaller::slotPackage);

  m_busy = true;
  Q_EMIT busyChanged();
}

void PackageKitInstaller::installPackage(const QString& packageId)
{
  if (busy())
    return;

  PackageKit::Transaction* transaction = PackageKit::Daemon::installPackage(packageId, PackageKit::Transaction::TransactionFlagNone);
  setupTransaction(transaction);
}

void PackageKitInstaller::removePackage(const QString& packageId)
{
  if (busy())
    return;

  PackageKit::Transaction* transaction =
    PackageKit::Daemon::removePackage(packageId, false, false, PackageKit::Transaction::TransactionFlagNone);
  setupTransaction(transaction);
}

void PackageKitInstaller::updateCache()
{
  if (busy())
    return;

  PackageKit::Transaction* transaction = PackageKit::Daemon::refreshCache(true);
  setupTransaction(transaction);
}

void PackageKitInstaller::checkInstalled(const QString& packageId)
{
  if (busy())
    return;

  m_checkPackageId = packageId;
  m_checkFound = false;
  PackageKit::Transaction* transaction = PackageKit::Daemon::getPackages(PackageKit::Transaction::FilterInstalled);
  setupTransaction(transaction);
}

bool PackageKitInstaller::abortInstallation() const
{
  if (!m_transaction || !m_transaction->allowCancel())
    return false;

  QDBusPendingReply<> reply = m_transaction->cancel();
  return !reply.isValid() || reply.isFinished(); // fire-and-forget is acceptable here
}

void PackageKitInstaller::slotPercentageChanged()
{
  if (!m_transaction)
    return;
  m_downloadProgress = int(m_transaction->percentage());
  Q_EMIT downloadProgressChanged();
}

void PackageKitInstaller::slotStatusChanged()
{
  if (!m_transaction)
    return;
  Q_EMIT busyChanged(); // keep pages' busy binding live while status changes
}

void PackageKitInstaller::slotFinished(PackageKit::Transaction::Exit status, uint runtime)
{
  Q_UNUSED(runtime)
  if (m_lastRole == PackageKit::Transaction::RoleGetPackages) {
    // Report from accumulated evidence, not exit code; slotPackage
    // already emits true on a match.
    if (!m_checkFound) {
      Q_EMIT installCheckResult(m_checkPackageId, false, QString());
    }
    m_checkFound = false;
    m_checkPackageId.clear();
  }

  m_transaction = 0;
  if (status == PackageKit::Transaction::ExitSuccess) {
    Q_EMIT packageInstalled();
  } else {
    Q_EMIT packageInstallationFailed();
  }
  Q_EMIT transactionFinished();

  m_busy = false;
  Q_EMIT busyChanged();
}

void PackageKitInstaller::slotErrorCode(PackageKit::Transaction::Error error, const QString& details)
{
  Q_UNUSED(error)
  m_lastError = details;
}

void PackageKitInstaller::slotPackage(PackageKit::Transaction::Info info, const QString& packageID, const QString& summary)
{
  Q_UNUSED(info)
  Q_UNUSED(summary)

  if (m_lastRole == PackageKit::Transaction::RoleGetPackages) {
    // The package ID is "name;version;arch;data"
    const QString name = PackageKit::Transaction::packageName(packageID);
    const QString version = PackageKit::Transaction::packageVersion(packageID);
    if (name == PackageKit::Transaction::packageName(m_checkPackageId)) {
      m_checkFound = true;
      Q_EMIT installCheckResult(m_checkPackageId, true, version);
    }
  }
}
