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

#include "packagekitpackage.h"
#include "../installers/packagekitinstaller.h"
#include "../platformintegration.h"

#include <AppStreamQt/developer.h>
#include <AppStreamQt/icon.h>
#include <AppStreamQt/image.h>
#include <AppStreamQt/screenshot.h>

PackageKitPackageItem::PackageKitPackageItem(const AppStream::Component& component, QObject* parent)
  : PackageItem(QVariantMap(), parent)
{
  m_componentId = component.id();
  m_packageName = component.packageNames().isEmpty() ? component.id() : component.packageNames().first();
  m_appId = m_componentId;
  m_name = component.name();
  m_tagline = component.summary();
  m_description = component.description();
  const QList<AppStream::Release> releases = component.releasesPlain().entries();
  m_version = releases.isEmpty() ? QString() : releases.first().version();
  m_publisher = component.developer().name();
  m_license = component.projectLicense();
  const QStringList categories = component.categories();
  m_category = categories.isEmpty() ? QString() : categories.first();
  m_packageType = QStringLiteral("packagekit");
  m_types = QStringList() << QStringLiteral("packagekit");

  if (!component.icons().isEmpty()) {
    const AppStream::Icon& icon = component.icons().first();
    m_icon = !icon.isEmpty() && !icon.url().isEmpty() ? icon.url().toString() : QString();
  }
  if (m_icon.isEmpty())
    m_icon = QStringLiteral("qrc:/Assets/fallback.svg");

  Q_FOREACH (const AppStream::Screenshot& shot, component.screenshotsAll()) {
    Q_FOREACH (const AppStream::Image& image, shot.images()) {
      if (!image.url().isEmpty())
        m_screenshots << image.url().toString();
    }
  }

  const AppStream::Launchable launchable = component.launchable(AppStream::Launchable::KindDesktopId);
  if (launchable.kind() == AppStream::Launchable::KindDesktopId && !launchable.entries().isEmpty()) {
    m_desktopEntry = launchable.entries().first();
  }

  m_hooks = QList<PackageItem::HookStruct>();
  m_ratings = new Ratings(QVariantMap(), this);
  m_reviews = new ReviewsModel(m_appId, this);
  m_matchingFramework = true;
  m_isBusy = false;

  // Installed/update state is supplied by PackageKitSource via setInstalledState().
  Q_EMIT updated();
}

bool PackageKitPackageItem::install() const
{
  PackageKitInstaller* installer = PlatformIntegration::instance()->packageKitInstaller();
  if (!installer || installer->busy() || m_packageName.isEmpty())
    return false;

  // Disconnect first; repeated installs stack signal wiring.
  disconnect(installer, nullptr, this, nullptr);

  connect(installer, &PackageKitInstaller::busyChanged, this, [this, installer]() {
    if (installer->busy())
      return;
    disconnect(installer, &PackageKitInstaller::downloadProgressChanged, nullptr, nullptr);
    m_isBusy = false;
    Q_EMIT updated();
  });
  connect(installer, &PackageKitInstaller::downloadProgressChanged, this, [this, installer]() {
    m_downloadProgress = installer->downloadProgress();
    Q_EMIT downloadProgressChanged();
  });

  installer->installPackage(m_packageName);
  m_isBusy = true;
  Q_EMIT updated();
  return true;
}

bool PackageKitPackageItem::remove() const
{
  PackageKitInstaller* installer = PlatformIntegration::instance()->packageKitInstaller();
  if (!installer || installer->busy() || m_packageName.isEmpty())
    return false;

  // Same wiring concern as install(); disconnect prior connections first.
  disconnect(installer, nullptr, this, nullptr);

  connect(installer, &PackageKitInstaller::packageInstallationFailed, this, [this]() {
    m_isBusy = false;
    Q_EMIT updated();
  });
  connect(installer, &PackageKitInstaller::packageInstalled, this, [this]() {
    m_isBusy = false;
    Q_EMIT updated();
  });

  m_isBusy = true;
  Q_EMIT updated();

  installer->removePackage(m_packageName);
  return true;
}

QString PackageKitPackageItem::appLaunchUrl() const
{
  if (m_desktopEntry.isEmpty())
    return QString();

  // TODO verify if this works
  return QStringLiteral("exec://") + m_desktopEntry;
}

void PackageKitPackageItem::setInstalledState(bool installed, const QString& version, bool updateAvailable)
{
  if (installed) {
    m_installedVersion = version;
    m_installedRevision = 1; // installed state is binary; update vs current is carried by m_revision below
    m_revision = updateAvailable ? 2 : 1;
  } else {
    m_installedVersion.clear();
    m_installedRevision = 0;
    m_revision = 1;
  }
  Q_EMIT updated();
  Q_EMIT installedChanged();
}

void PackageKitPackageItem::fillData(const QVariantMap& json)
{
  Q_UNUSED(json)
}
