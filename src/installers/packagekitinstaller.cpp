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

namespace {
const char* errorName(PackageKit::Transaction::Error error)
{
  switch (error) {
    case PackageKit::Transaction::ErrorUnknown:
      return "Unknown";
    case PackageKit::Transaction::ErrorOom:
      return "Out Of Memory";
    case PackageKit::Transaction::ErrorNoNetwork:
      return "No Network";
    case PackageKit::Transaction::ErrorNotSupported:
      return "Not Supported";
    case PackageKit::Transaction::ErrorInternalError:
      return "Internal Error";
    case PackageKit::Transaction::ErrorGpgFailure:
      return "Gpg Failure";
    case PackageKit::Transaction::ErrorPackageIdInvalid:
      return "Package Id Invalid";
    case PackageKit::Transaction::ErrorPackageNotInstalled:
      return "Package Not Installed";
    case PackageKit::Transaction::ErrorPackageNotFound:
      return "Package Not Found";
    case PackageKit::Transaction::ErrorPackageAlreadyInstalled:
      return "Package Already Installed";
    case PackageKit::Transaction::ErrorPackageDownloadFailed:
      return "Package Download Failed";
    case PackageKit::Transaction::ErrorDepResolutionFailed:
      return "Dep Resolution Failed";
    case PackageKit::Transaction::ErrorTransactionError:
      return "Transaction Error";
    case PackageKit::Transaction::ErrorTransactionCancelled:
      return "Transaction Cancelled";
    case PackageKit::Transaction::ErrorNoCache:
      return "No Cache";
    case PackageKit::Transaction::ErrorRepoNotFound:
      return "Repo Not Found";
    case PackageKit::Transaction::ErrorCannotCancel:
      return "Cannot Cancel";
    case PackageKit::Transaction::ErrorCannotGetLock:
      return "Cannot Get Lock";
    case PackageKit::Transaction::ErrorNoPackagesToUpdate:
      return "No Packages To Update";
    case PackageKit::Transaction::ErrorRepoConfigurationError:
      return "Repo Configuration Error";
    case PackageKit::Transaction::ErrorNoLicenseAgreement:
      return "No License Agreement";
    case PackageKit::Transaction::ErrorFileConflicts:
      return "File Conflicts";
    case PackageKit::Transaction::ErrorPackageConflicts:
      return "Package Conflicts";
    case PackageKit::Transaction::ErrorRepoNotAvailable:
      return "Repo Not Available";
    case PackageKit::Transaction::ErrorPackageCorrupt:
      return "Package Corrupt";
    case PackageKit::Transaction::ErrorAllPackagesAlreadyInstalled:
      return "All Packages Already Installed";
    case PackageKit::Transaction::ErrorFileNotFound:
      return "File Not Found";
    case PackageKit::Transaction::ErrorNoMoreMirrorsToTry:
      return "No More Mirrors To Try";
    case PackageKit::Transaction::ErrorIncompatibleArchitecture:
      return "Incompatible Architecture";
    case PackageKit::Transaction::ErrorNoSpaceOnDevice:
      return "No Space On Device";
    case PackageKit::Transaction::ErrorNotAuthorized:
      return "Not Authorized";
    case PackageKit::Transaction::ErrorPackageFailedToConfigure:
      return "Package Failed To Configure";
    case PackageKit::Transaction::ErrorPackageFailedToInstall:
      return "Package Failed To Install";
    case PackageKit::Transaction::ErrorPackageFailedToRemove:
      return "Package Failed To Remove";
    case PackageKit::Transaction::ErrorInstallRootInvalid:
      return "Install Root Invalid";
    case PackageKit::Transaction::ErrorUnfinishedTransaction:
      return "Unfinished Transaction";
    case PackageKit::Transaction::ErrorLockRequired:
      return "Lock Required";
    default:
      return "Unknown";
  }
}
} // namespace

PackageKitInstaller::PackageKitInstaller(QObject* parent)
  : QObject(parent)
{
}

void PackageKitInstaller::beginOperation(const QString& operation, const QString& packageId)
{
  m_operation = operation;
  m_requestedPackageId = packageId;
}

void PackageKitInstaller::failImmediately(const QString& reason)
{
  m_lastError = reason;
  m_lastErrorName = QStringLiteral("NoTransaction");
  m_transaction = 0;

  // Walk busy through true -> false so busyChanged listeners still unwind the item.
  m_busy = true;
  Q_EMIT busyChanged();
  m_busy = false;
  Q_EMIT busyChanged();

  if (m_operation == QStringLiteral("install") || m_operation == QStringLiteral("remove")) {
    Q_EMIT packageInstallationFailed();
  }

  Q_EMIT transactionFinished();
}

void PackageKitInstaller::setupTransaction(PackageKit::Transaction* transaction)
{
  if (!transaction) {
    // The daemon refused the request before creating a transaction; report the
    // failure instead of leaving the caller waiting forever.
    failImmediately(m_requestedPackageId.isEmpty()
                      ? QStringLiteral("PackageKit is not available. Check that the PackageKit service is running.")
                      : QStringLiteral("PackageKit refused the request for '%1'. Check that the package name is correct and the "
                                       "PackageKit service is running.")
                          .arg(m_requestedPackageId));
    return;
  }

  m_transaction = transaction;
  m_lastRole = transaction->role();
  m_lastError.clear();
  m_lastErrorName.clear();
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

  beginOperation(QStringLiteral("install"), packageId);

  // OnlyTrusted avoids PackageKit's stricter package-install-untrusted polkit action.
  PackageKit::Transaction* transaction = PackageKit::Daemon::installPackage(packageId, PackageKit::Transaction::TransactionFlagOnlyTrusted);
  setupTransaction(transaction);
}

void PackageKitInstaller::removePackage(const QString& packageId)
{
  if (busy())
    return;

  beginOperation(QStringLiteral("remove"), packageId);

  PackageKit::Transaction* transaction =
    PackageKit::Daemon::removePackage(packageId, false, false, PackageKit::Transaction::TransactionFlagNone);
  setupTransaction(transaction);
}

void PackageKitInstaller::updateCache()
{
  if (busy())
    return;

  beginOperation(QStringLiteral("refresh-cache"), QString());

  PackageKit::Transaction* transaction = PackageKit::Daemon::refreshCache(true);
  setupTransaction(transaction);
}

void PackageKitInstaller::checkInstalled(const QString& packageId)
{
  if (busy())
    return;

  beginOperation(QStringLiteral("check-installed"), packageId);
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

  // finished() fires twice on failure; the follow-up arrives with no transaction.
  if (m_transaction.isNull())
    return;

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

  // Clear busy before the result signals so queued operations can start the next transaction.
  m_busy = false;
  Q_EMIT busyChanged();

  if (status == PackageKit::Transaction::ExitSuccess) {
    Q_EMIT packageInstalled();
  } else {
    Q_EMIT packageInstallationFailed();
  }
  Q_EMIT transactionFinished();
}

void PackageKitInstaller::slotErrorCode(PackageKit::Transaction::Error error, const QString& details)
{
  m_lastError = details;
  m_lastErrorName = errorName(error);
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
