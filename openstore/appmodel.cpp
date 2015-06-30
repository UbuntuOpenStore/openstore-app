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
        qDebug() << "have framework" << frameworkName;
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

