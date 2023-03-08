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

#include "platformintegration.h"
#include "clickinstaller.h"

    PackageItem::PackageItem(const QVariantMap &json, QObject *parent)
    : QObject(parent)
{
    fillData(json);
}

PackageItem::~PackageItem()
{
    //qDebug() << Q_FUNC_INFO << m_appId;
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

    Q_FOREACH(const HookStruct &h, m_hooks) {
        if (h.hooks & PackageItem::HookDesktop) {
            return "appid://" + m_appId + "/" + h.name + "/" + m_installedVersion;
        }
    }

    return QString();
}

void PackageItem::fillData(const QVariantMap &json)
{
    m_appId = json.value("id").toString();

    m_icon = json.value("icon").toString();
    m_name = json.value("name").toString();
    m_publisher = json.value("publisher").toString();

    QList<QVariant> downloads = json.value("downloads").toList();
    m_version = json.value("version").toString();
    m_revision = json.value("revision").toInt();
    Q_FOREACH (QVariant download, downloads) {
        QMap<QString, QVariant> downloadData = download.toMap();

        if (
            downloadData.value("channel") == PlatformIntegration::instance()->systemCodename() &&
            (
                downloadData.value("architecture") == PlatformIntegration::instance()->supportedArchitecture() ||
                downloadData.value("architecture") == QStringLiteral("all")
            )
        ) {
            m_version = downloadData.value("version").toString();
            m_revision = downloadData.value("revision").toInt();
            m_packageUrl = downloadData.value("download_url").toString();
            m_fileSize = json.value("filesize").toInt();
        }
    }

    m_source = json.value("source").toString();
    m_donateUrl = json.value("donate_url").toString();
    m_supportUrl = json.value("support_url").toString();
    m_translationUrl = json.value("translation_url").toString();
    m_license = json.value("license").toString();
    m_latestDownloads = json.value("latestDownloads").toString();
    m_totalDownloads= json.value("totalDownloads").toString();
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

    QList<PackageItem::HookStruct> hooksList;
    if (json.contains("manifest") && json.value("manifest").toMap().contains("hooks")) {
        QVariantMap manifest = json.value("manifest").toMap();
        QVariantMap hookMap = manifest.value("hooks").toMap();

        Q_FOREACH (const QString & hook, hookMap.keys()) {
            PackageItem::HookStruct hookStruct;
            hookStruct.name = hook;
            hookStruct.hooks = PackageItem::HookNone;

            QStringList permissions;
            QStringList readPaths;
            QStringList writePaths;

            QVariantMap apparmorMap = hookMap.value(hook).toMap().value("apparmor").toMap();

            //                qDebug() << "have apparmor for" << hook << apparmorMap;
            Q_FOREACH (const QVariant &perm, apparmorMap.value("policy_groups").toList()) {
                permissions.append(perm.toString());
            }

            Q_FOREACH (const QVariant &perm, apparmorMap.value("read_path").toList()) {
                readPaths.append(perm.toString());
            }

            Q_FOREACH (const QVariant &perm, apparmorMap.value("write_path").toList()) {
                writePaths.append(perm.toString());
            }

            hookStruct.apparmorTemplate = apparmorMap.value("template").toString();
            hookStruct.readPaths = readPaths;
            hookStruct.writePaths = writePaths;
            hookStruct.permissions = permissions;

            if (hookMap.value(hook).toMap().contains("desktop")) {
                hookStruct.hooks |= PackageItem::HookDesktop;
            }
            if (hookMap.value(hook).toMap().contains("content-hub")) {
                hookStruct.hooks |= PackageItem::HookContentHub;
            }
            if (hookMap.value(hook).toMap().contains("urls")) {
                hookStruct.hooks |= PackageItem::HookUrls;
            }
            if (hookMap.value(hook).toMap().contains("push-helper")) {
                hookStruct.hooks |= PackageItem::HookPushHelper;
            }
            if (hookMap.value(hook).toMap().contains("account-provider")) {
                hookStruct.hooks |= PackageItem::HookAccountService;
            }
            hooksList.append(hookStruct);
        }

        m_hooks = hooksList;
    }

    m_reviews = new ReviewsModel(m_appId, this);
    m_ratings = new Ratings(json["ratings"].toMap(), this);

    Q_EMIT updated();
}

void PackageItem::updateLocalInfo(int localRevision, const QString &localVersion)
{
    m_installedVersion = localVersion;
    m_installedRevision = localRevision;

    Q_EMIT updated();
    Q_EMIT installedChanged();
}

bool PackageItem::review(const QString &review, Rating rating, const QString &apiKey) const
{
    return m_reviews->sendReview(m_version, review, rating, apiKey, false);
}

bool PackageItem::editReview(const QString &review, Rating rating, const QString &apiKey) const
{
    return m_reviews->sendReview(m_version, review, rating, apiKey, true);
}
