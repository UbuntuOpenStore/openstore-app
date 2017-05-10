/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
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
 */

#include "appmodel.h"

#include <click.h>
#include <gio/gio.h>
//#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <libintl.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QUrl>
#include <QUrlQuery>

AppModel::AppModel(QObject *parent) :
    QAbstractListModel(parent),
    m_installer(0)
{
    m_nam = new QNetworkAccessManager(this);

    buildInstalledClickList();
    loadAppList();
}

int AppModel::rowCount(const QModelIndex &parent) const
{
    return m_list.count();
}

QVariant AppModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case RoleName:
        return m_list.at(index.row())->name();
    case RoleAppId:
        return m_list.at(index.row())->appId();
    case RoleIcon:
        return m_list.at(index.row())->icon();
    case RoleAuthor:
        return m_list.at(index.row())->author();
    case RoleTagline:
        return m_list.at(index.row())->tagline();
    case RoleDescription:
        return m_list.at(index.row())->description();
    case RoleCategory:
        return m_list.at(index.row())->category();
    case RoleScreenshots:
        return m_list.at(index.row())->screenshots();
    case RoleChangelog:
        return m_list.at(index.row())->changelog();
    case RolePackageUrl:
        return m_list.at(index.row())->packageUrl();
    case RoleVersion:
        return m_list.at(index.row())->version();
    case RoleInstalled:
        return m_list.at(index.row())->installed();
    case RoleInstalledVersion:
        return m_list.at(index.row())->installedVersion();
    case RoleUpdateAvailable:
        return bool(m_list.at(index.row())->installedVersion() < m_list.at(index.row())->version());
    case RoleMaintainer:
        return m_list.at(index.row())->maintainer();
    case RoleSearchHackishString:
        return QString(m_list.at(index.row())->name()) + QString(m_list.at(index.row())->appId()) + QString(m_list.at(index.row())->author());
    }
    return QVariant();
}

QHash<int, QByteArray> AppModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleName, "name");
    roles.insert(RoleAppId, "appId");
    roles.insert(RoleIcon, "icon");
    roles.insert(RoleAuthor, "author");
    roles.insert(RoleTagline, "tagline");
    roles.insert(RoleDescription, "description");
    roles.insert(RoleCategory, "category");
    roles.insert(RoleScreenshots, "screenshots");
    roles.insert(RoleChangelog, "changelog");
    roles.insert(RolePackageUrl, "packageUrl");
    roles.insert(RoleVersion, "version");
    roles.insert(RoleInstalled, "installed");
    roles.insert(RoleInstalledVersion, "installedVersion");
    roles.insert(RoleUpdateAvailable, "updateAvailable");
    roles.insert(RoleMaintainer, "maintainer");
    roles.insert(RoleSearchHackishString, "searchHackishString");
    return roles;
}

int AppModel::findApp(const QString &appId) const
{
    for (int i = 0; i < m_list.count(); i++) {
        if (m_list.at(i)->appId() == appId) {
            return i;
        }
    }
    return -1;
}

ApplicationItem *AppModel::app(int index) const
{
    if (index < -1 || index + 1 > m_list.count()) {
        qWarning() << Q_FUNC_INFO << "Index is out of range. Returning nullptr...";
        return nullptr;
    }

    return m_list.at(index);
}

void AppModel::install(int index)
{
    if (m_installer->busy()) {
//        qDebug() << "Installer busy...";
        return;
    }
    m_installer->installPackage(m_list.at(index)->packageUrl());
}

ClickInstaller *AppModel::installer()
{
    return m_installer;
}

void AppModel::setInstaller(ClickInstaller *installer)
{
    if (m_installer != installer) {

        if (m_installer) {
            disconnect(m_installer, &ClickInstaller::busyChanged, this, &AppModel::installerBusyChanged);
        }

        m_installer = installer;
        Q_EMIT installerChanged();
        connect(m_installer, &ClickInstaller::busyChanged, this, &AppModel::installerBusyChanged);
    }
}

void AppModel::loadAppList()
{
    GList *gframeworks = click_framework_get_frameworks();
    QStringList frameworks;
    while (gframeworks) {
        QString frameworkName = QString::fromUtf8(click_framework_get_name((ClickFramework*)gframeworks->data));
//        qDebug() << "have framework" << frameworkName;
        frameworks << frameworkName;
        gframeworks = gframeworks->next;
    }

    QUrlQuery query;
    query.addQueryItem("frameworks", frameworks.join(','));
    // FIXME: So far we only support armhf
    query.addQueryItem("architecture", "armhf");
    QUrl url("https://open.uappexplorer.com/api/apps");
    url.setQuery(query);
    QNetworkRequest request(url);

    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, &AppModel::repoListFetched);
}

void AppModel::repoListFetched()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    QByteArray data = reply->readAll();
//    qDebug() << "have repolist" << data;

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
//        qDebug() << "Error parsing json";
        return;
    }

    QVariantMap replyMap = jsonDoc.toVariant().toMap();

    if (!replyMap.value("success").toBool() || !replyMap.contains("data")) {
//        qDebug() << "Repolist doesn't contain a package list";
        return;
    }

    QVariantList packageVariantList = replyMap.value("data").toList();

    beginResetModel();
    Q_FOREACH (const QVariant &packageVariant, packageVariantList) {
        QVariantMap packageMap = packageVariant.toMap();
        QString appId = packageMap.value("id").toString();
//        qDebug() << "have app" << appId;
        ApplicationItem *item = new ApplicationItem(appId, this);
        item->setIcon(packageMap.value("icon").toString());
        item->setName(packageMap.value("name").toString());
        item->setAuthor(packageMap.value("author").toString());
        item->setPackageUrl(packageMap.value("download").toString());
        item->setSource(packageMap.value("source").toString());
        item->setLicense(packageMap.value("license").toString());
        item->setMaintainer(packageMap.value("maintainer_name").toString());
        item->setTagline(packageMap.value("tagline").toString());
        item->setDescription(packageMap.value("description").toString());
        item->setCategory(packageMap.value("category").toString());
        item->setScreenshots(packageMap.value("screenshots").toStringList());
        item->setChangelog(packageMap.value("changelog").toString());
        item->setVersion(packageMap.value("version").toString());
        item->setFileSize(packageMap.value("filesize").toInt());
        item->setInstalledVersion(m_installedAppIds.value(item->appId()));

        QList<ApplicationItem::HookStruct> hooksList;
        if (packageMap.contains("manifest") && packageMap.value("manifest").toMap().contains("hooks")) {
            QVariantMap hookMap = packageMap.value("manifest").toMap().value("hooks").toMap();
            Q_FOREACH (const QString & hook, hookMap.keys()) {
                ApplicationItem::HookStruct hookStruct;
                hookStruct.name = hook;
                hookStruct.hooks = ApplicationItem::HookNone;
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
                    hookStruct.hooks |= ApplicationItem::HookDesktop;
                }
                if (hookMap.value(hook).toMap().contains("scope")) {
                    hookStruct.hooks |= ApplicationItem::HookScope;
                }
                if (hookMap.value(hook).toMap().contains("content-hub")) {
                    hookStruct.hooks |= ApplicationItem::HookContentHub;
                }
                if (hookMap.value(hook).toMap().contains("urls")) {
                    hookStruct.hooks |= ApplicationItem::HookUrls;
                }
                if (hookMap.value(hook).toMap().contains("push-helper")) {
                    hookStruct.hooks |= ApplicationItem::HookPushHelper;
                }
                if (hookMap.value(hook).toMap().contains("account-provider")) {
                    hookStruct.hooks |= ApplicationItem::HookAccountService;
                }
                hooksList.append(hookStruct);
            }
            item->setHooks(hooksList);
        }
        
        m_list.append(item);
    }
    endResetModel();

    Q_EMIT repositoryListFetched();
}

void AppModel::buildInstalledClickList()
{
    ClickDB *clickdb;
     GError *err = nullptr;
     gchar *clickmanifest = nullptr;

     clickdb = click_db_new();
     click_db_read(clickdb, nullptr, &err);
     if (err != nullptr) {
         g_warning("Unable to read Click database: %s", err->message);
         g_error_free(err);
         g_object_unref(clickdb);
         return;
     }

     ClickUser *clickUser = click_user_new_for_user(clickdb, "phablet", &err);
     if (err != nullptr) {
         g_error_free(err);
         g_object_unref(clickdb);
         g_object_unref(clickUser);
         return;
     }
     clickmanifest = click_user_get_manifests_as_string(clickUser, &err);
     g_object_unref(clickdb);
     g_object_unref(clickUser);

     if (err != nullptr) {
         g_warning("Unable to get the manifests: %s", err->message);
         g_error_free(err);
         return;
     }

     QJsonParseError error;

     QJsonDocument jsond =
             QJsonDocument::fromJson(clickmanifest, &error);
     g_free(clickmanifest);

     if (error.error != QJsonParseError::NoError) {
         qWarning() << error.errorString();
         return;
     }

//     qDebug() << "loaded stuff" << jsond.toJson();
     QVariantList appListJson = jsond.toVariant().toList();

     m_installedAppIds.clear();
     qDebug() << "building click list:";
     Q_FOREACH(const QVariant &appJson, appListJson) {
         QVariantMap appMap = appJson.toMap();

         QString appId = appMap.value("name").toString();
         QString version = appMap.value("version").toString();
//         qDebug() << "have installed app:" << appId << version << appMap;
         if (!m_installedAppIds.contains(appId) || m_installedAppIds.value(appId) < version) {
             m_installedAppIds[appId] = version;
         }
     }

     Q_FOREACH(ApplicationItem *app, m_list) {
         if (m_installedAppIds.contains(app->appId())) {
             app->setInstalledVersion(m_installedAppIds.value(app->appId()));
         } else {
             app->setInstalledVersion("");
         }
         int idx = m_list.indexOf(app);
         Q_EMIT dataChanged(index(idx), index(idx));
     }
}

void AppModel::installerBusyChanged()
{
    if (!m_installer->busy()) {
        buildInstalledClickList();
    }
}

