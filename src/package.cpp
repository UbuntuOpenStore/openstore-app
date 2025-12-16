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

#include "package.h"

#include "clickinstaller.h"
#include "platformintegration.h"

PackageItem::PackageItem(const QVariantMap& json, QObject* parent)
  : QObject(parent)
{
  fillData(json);
}

PackageItem::~PackageItem()
{
  // qDebug() << Q_FUNC_INFO << m_appId;
}

bool PackageItem::install() const
{
  ClickInstaller* installer = PlatformIntegration::instance()->clickInstaller();

  if (installer->busy()) {
    qDebug() << Q_FUNC_INFO << "Installer is busy...";
    return false;
  }

  if (packageUrl().isEmpty()) {
    qDebug() << Q_FUNC_INFO << "Package url for" << appId() << "not valid";
    return false;
  }

  installer->installPackage(packageUrl());
  return true;
}

bool PackageItem::remove() const
{
  ClickInstaller* installer = PlatformIntegration::instance()->clickInstaller();

  if (installer->busy()) {
    qDebug() << Q_FUNC_INFO << "Installer is busy...";
    return false;
  }

  if (packageUrl().isEmpty()) {
    qDebug() << Q_FUNC_INFO << "Package url for" << appId() << "not valid";
    return false;
  }

  installer->removePackage(appId(), installedVersionString());
  return true;
}

QString PackageItem::appLaunchUrl() const
{
  if (m_installedVersion.isEmpty())
    return QString();

  Q_FOREACH (const HookStruct& h, m_hooks) {
    if (h.hooks & PackageItem::HookDesktop) {
      return "appid://" + m_appId + "/" + h.name + "/current-user-version";
    }
  }

  return QString();
}

void PackageItem::fillData(const QVariantMap& json)
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
  m_totalDownloads = json.value("totalDownloads").toString();
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

  QStringList deviceCompatibilities = json.value("device_compatibilities").toStringList();
  m_matchingFramework = false;
  for (QString deviceCompatibility : deviceCompatibilities) {
    QStringList parts = deviceCompatibility.split(":");

    // We only use one channel
    if (parts[0] != "focal") {
      continue;
    }

    if (parts.size() == 3) {
      m_frameworks << parts[2];
      if (supportedFrameworks.contains(parts[2])) {
        m_matchingFramework = true;
      }
    }
  }

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

  Q_EMIT updated();
}

void PackageItem::updateLocalInfo(int localRevision, const QString& localVersion)
{
  m_installedVersion = localVersion;
  m_installedRevision = localRevision;

  Q_EMIT updated();
  Q_EMIT installedChanged();
}

bool PackageItem::review(const QString& review, Rating rating, const QString& apiKey) const
{
  return m_reviews->sendReview(m_version, review, rating, apiKey, false);
}

bool PackageItem::editReview(const QString& review, Rating rating, const QString& apiKey) const
{
  return m_reviews->sendReview(m_version, review, rating, apiKey, true);
}
