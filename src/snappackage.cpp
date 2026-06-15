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

#include "snappackage.h"

#include "platformintegration.h"

#include <Snapd/Client>
#include <Snapd/App>

SnapPackageItem::SnapPackageItem(const QVariantMap& json, QObject* parent)
  : PackageItem(json, parent), m_containsApp(false)
{
  fillData(json);
}

SnapPackageItem::~SnapPackageItem()
{
  // qDebug() << Q_FUNC_INFO << m_appId;
}

bool SnapPackageItem::install() const
{
  const QString snapName = appId().mid(5);
  qDebug() << "Installing snap name:" << snapName;

  QSnapdClient* installer = PlatformIntegration::instance()->snapInstaller();
  if (!installer)
    return false;

  auto request = installer->install(QSnapdClient::InstallFlag::Classic, snapName);
  QObject::connect(request, &QSnapdRequest::progress, this, [=](){
    const auto change = request->change();
    qint64 totalProgressDone = 0, progressTotal = 0;

    for (int i = 0, c = change->taskCount(); i < c; ++i) {
        auto task = change->task(i);
        if (!task->progressLabel().isEmpty()) {
            totalProgressDone += task->progressDone();
            progressTotal += task->progressTotal();
        }
    }

    if (progressTotal > 0 && m_downloadSize != progressTotal) {
      m_downloadSize = progressTotal;
      Q_EMIT updated();
    }
    m_downloadProgress = totalProgressDone;

    // Reset back to indeterminate progress bar when download finished
    if (m_downloadProgress >= m_downloadSize)
      m_downloadProgress = 0;

    Q_EMIT downloadProgressChanged();
  });

  connect(request, &QSnapdRequest::complete, this, [=]() mutable {
    qInfo() << "Snap installation transaction complete!";

    auto getRequest = installer->getSnap(snapName);
    getRequest->runSync();

    const auto installedVersion = getRequest->snap()->version();
    const auto installedRevision = getRequest->snap()->revision().replace("x", "").toInt();
    if (!m_containsApp) {
      for (int i = 0; i < getRequest->snap()->appCount(); i++) {
        auto snapApp = getRequest->snap()->app(i);
        if (!snapApp->desktopFile().isEmpty()) {
          m_containsApp = true;
          break;
        }
      }
    }

    m_isBusy = false;
    updateLocalInfo(installedRevision, installedVersion);
    PlatformIntegration::instance()->update();
  });

  request->runAsync();
  m_isBusy = true;
  Q_EMIT updated();
  return true;
}

bool SnapPackageItem::remove() const
{
  qDebug() << Q_FUNC_INFO;

  const QString snapName = appId().mid(5);
  qDebug() << "Removing snap name:" << snapName;

  QSnapdClient* installer = PlatformIntegration::instance()->snapInstaller();
  if (!installer)
    return false;

  auto request = installer->remove(QSnapdClient::RemoveFlag::Purge, snapName);

  connect(request, &QSnapdRequest::complete, this, [=]() mutable {
    qInfo() << "Snap removal transaction complete!";

    m_installedVersion = QString();
    m_installedRevision = 0;
    m_downloadProgress = 0;
    m_isBusy = false;
    Q_EMIT updated();
    Q_EMIT installedChanged();
    Q_EMIT downloadProgressChanged();
    PlatformIntegration::instance()->update();
  });

  request->runAsync();
  m_isBusy = true;
  Q_EMIT updated();
  return true;
}

QString SnapPackageItem::appLaunchUrl() const
{
  if (m_installedVersion.isEmpty())
    return QString();

  const auto snapName = m_appId.mid(5);
  QSnapdClient* client = PlatformIntegration::instance()->snapInstaller();
  auto request = client->getSnap(snapName);
  request->runSync();
  for (int i = 0; i < request->snap()->appCount(); i++) {
    auto snapApp = request->snap()->app(i);
    if (snapApp->desktopFile().isEmpty())
      continue;

    return "appid://" + snapName + "/" + snapApp->name() + "/current-user-version";
  }

  return QString();
}

void SnapPackageItem::fillData(const QVariantMap& json)
{
  m_appId = json.value("id").toString();

  m_icon = json.value("icon").toString();
  m_name = json.value("name").toString();
  m_publisher = json.value("publisher").toString();

  QStringList supportedFrameworks = PlatformIntegration::instance()->supportedFrameworks();

  QList<QVariant> downloads = json.value("downloads").toList();
  m_version = json.value("version").toString();
  m_revision = json.value("revision").toInt();
  Q_FOREACH (QVariant download, downloads) {
    QMap<QString, QVariant> downloadData = download.toMap();

    if (downloadData.value("channel").toString() == QStringLiteral("focal") &&
        supportedFrameworks.contains(downloadData.value("framework").toString()) &&
        (downloadData.value("architecture") == PlatformIntegration::instance()->supportedArchitecture() ||
         downloadData.value("architecture") == QStringLiteral("all"))) {
      m_version = downloadData.value("version").toString();
      m_revision = downloadData.value("revision").toInt();
      m_packageUrl = downloadData.value("download_url").toString();
      m_installedSize = downloadData.value("installedSize").toInt();
      m_downloadSize = downloadData.value("downloadSize").toInt();
    }
  }

  m_source = json.value("source").toString();
  m_donateUrl = json.value("donate_url").toString();
  m_supportUrl = json.value("support_url").toString();
  m_translationUrl = json.value("translation_url").toString();
  m_license = json.value("license").toString();
  m_latestDownloads = json.value("latestDownloads").toString();
  m_totalDownloads = (json.value("totalDownloads").toInt() == 0) ? "none, unknown" :
                       json.value("totalDownloads").toString();
  m_maintainer = json.value("maintainer_name").toString();
  m_tagline = json.value("tagline").toString();
  m_description = json.value("description").toString();
  m_category = json.value("category").toString();
  m_screenshots = json.value("screenshots").toStringList();
  m_changelog = json.value("changelog").toString();
  m_installedVersion = PlatformIntegration::instance()->appVersion(m_appId);
  m_publishedDate = json.value("published_date").toDateTime();
  m_updatedDate = json.value("updated_date").toDateTime();
  m_channels = json.value("channels").toStringList();
  m_types = json.value("types").toStringList();
  m_packageType = QStringLiteral("snap");
  m_matchingFramework = true;

  // Parse hook information from top-level API fields
  // Note: The manifest.hooks field has been deprecated. We now use aggregated
  // permissions, read_paths, and write_paths provided at the top level of the API response.
  QList<PackageItem::HookStruct> hooksList;

  PackageItem::HookStruct hookStruct;

  if (json.contains("desktop_names") && json.value("desktop_names").toList().size() > 0) {
    hookStruct.name = json.value("desktop_names").toList().first().toString();
  } else {
    // Fallback: derive from app ID (e.g., "openstore.openstore-team" -> "openstore")
    QStringList appIdParts = m_appId.split(".");
    hookStruct.name = appIdParts.size() > 0 ? appIdParts.first() : m_appId;
  }

  QStringList permissions;
  if (json.contains("permissions")) {
    Q_FOREACH (const QVariant& perm, json.value("permissions").toList()) {
      permissions.append(perm.toString());
    }
  }

  QStringList readPaths;
  if (json.contains("read_paths")) {
    Q_FOREACH (const QVariant& path, json.value("read_paths").toList()) {
      readPaths.append(path.toString());
    }
  }

  QStringList writePaths;
  if (json.contains("write_paths")) {
    Q_FOREACH (const QVariant& path, json.value("write_paths").toList()) {
      writePaths.append(path.toString());
    }
  }

  // Infer apparmor template from permissions
  QString apparmorTemplate;
  if (permissions.contains("unconfined")) {
    apparmorTemplate = "unconfined";
  } else {
    apparmorTemplate = "";
  }

  // Infer hook types from available metadata
  hookStruct.hooks = PackageItem::HookDesktop; // Since we no longer have scopes, everything has a desktop hook

  // Infer content-hub from permissions (heuristic)
  if (permissions.contains("content_exchange") ||
      permissions.contains("content_exchange_source")) {
    hookStruct.hooks |= PackageItem::HookContentHub;
  }

  // Infer push-helper from permissions (heuristic)
  if (permissions.contains("push-notification-client")) {
    hookStruct.hooks |= PackageItem::HookPushHelper;
  }

  hookStruct.permissions = permissions;
  hookStruct.apparmorTemplate = apparmorTemplate;
  hookStruct.readPaths = readPaths;
  hookStruct.writePaths = writePaths;

  hooksList.append(hookStruct);

  m_hooks = hooksList;

  m_reviews = new ReviewsModel(m_appId, this);
  m_ratings = new Ratings(json["ratings"].toMap(), this);

  if (auto snapClient = PlatformIntegration::instance()->snapInstaller()) {
    const auto snapName = m_appId.mid(5);
    auto request = snapClient->getSnap(snapName);
    request->runSync();
    for (int i = 0; i < request->snap()->appCount(); i++) {
      auto snapApp = request->snap()->app(i);
      if (!snapApp->desktopFile().isEmpty() && !m_containsApp) {
        m_containsApp = true;
        break;
      }
    }

    const auto rev = request->snap()->revision();
    if (rev.startsWith("x"))
      m_installedRevision = 0;
    else
      m_installedRevision = rev.toInt();
      // qDebug() << m_installedVersion << m_installedRevision
      //          << isLocalVersionSideloaded() << frameworkSupported();
  } else {
    // Silence UI advertising reverts to stable from the store
    m_installedRevision = 1;
    m_installedVersion = "n/a";
  }

  m_isBusy = false;

  Q_EMIT updated();
}
