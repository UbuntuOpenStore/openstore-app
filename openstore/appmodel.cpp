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

AppModel::AppModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_nam = new QNetworkAccessManager(this);
    m_installer = new ClickInstaller(this);
    connect(m_installer, &ClickInstaller::busyChanged, this, &AppModel::installerBusyChanged);

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
    case RoleIcon:
        return m_list.at(index.row())->icon();
    case RoleTagline:
        return m_list.at(index.row())->tagline();
    case RoleDescription:
        return m_list.at(index.row())->description();
    case RolePackageUrl:
        return m_list.at(index.row())->packageUrl();
    case RoleVersion:
        return m_list.at(index.row())->version();
    case RoleInstalled:
        return m_list.at(index.row())->installed();
    case RoleInstalledVersion:
        return m_list.at(index.row())->installedVersion();
    }
    return QVariant();
}

QHash<int, QByteArray> AppModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleName, "name");
    roles.insert(RoleIcon, "icon");
    roles.insert(RoleTagline, "tagline");
    roles.insert(RoleDescription, "description");
    roles.insert(RolePackageUrl, "packageUrl");
    roles.insert(RoleVersion, "version");
    roles.insert(RoleInstalled, "installed");
    roles.insert(RoleInstalledVersion, "installedVersion");
    return roles;
}

ApplicationItem *AppModel::app(int index) const
{
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

void AppModel::loadAppList()
{
    QNetworkRequest request(QUrl("http://notyetthere.org/openstore/v1/repolist.json"));
//    QNetworkRequest request(QUrl("http://notyetthere.org/openstore/testing/repolist.json"));
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

    if (!replyMap.contains("packages")) {
//        qDebug() << "Repolist doesn't contain a package list";
        return;
    }

    QVariantList packageVariantList = replyMap.value("packages").toList();

    beginResetModel();
    Q_FOREACH (const QVariant &packageVariant, packageVariantList) {
        QVariantMap packageMap = packageVariant.toMap();
        QString appId = packageMap.value("id").toString();
//        qDebug() << "have app" << appId;
        ApplicationItem *item = new ApplicationItem(appId, this);
        item->setIcon(packageMap.value("icon").toString());
        item->setName(packageMap.value("name").toString());
        item->setPackageUrl(packageMap.value("package").toString());
        item->setSource(packageMap.value("source").toString());
        item->setLicense(packageMap.value("license").toString());
        item->setTagline(packageMap.value("tagline").toString());
        item->setDescription(packageMap.value("description").toString());
        item->setVersion(packageMap.value("version").toString());
        item->setFileSize(packageMap.value("filesize").toInt());
        item->setInstalledVersion(m_installedAppIds.value(item->appId()));
        m_list.append(item);
    }
    endResetModel();
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

     clickmanifest = click_db_get_manifests_as_string(clickdb, TRUE, &err);
     g_object_unref(clickdb);

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
     Q_FOREACH(const QVariant &appJson, appListJson) {
         QVariantMap appMap = appJson.toMap();

         QString appId = appMap.value("name").toString();
         QString version = appMap.value("version").toString();
         m_installedAppIds.insert(appId, version);
     }

     Q_FOREACH(ApplicationItem *app, m_list) {
         if (m_installedAppIds.contains(app->appId())) {
             app->setInstalledVersion(m_installedAppIds.value(app->appId()));
             int idx = m_list.indexOf(app);
             Q_EMIT dataChanged(index(idx), index(idx));
         }
     }
}

void AppModel::installerBusyChanged()
{
    if (!m_installer->busy()) {
        buildInstalledClickList();
    }
}
