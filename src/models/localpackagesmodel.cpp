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

#include "localpackagesmodel.h"
#include "../package.h"
#include "../platformintegration.h"
#include "../openstorenetworkmanager.h"
#include "../packagescache.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <algorithm>

// For desktop file parsing
#include <QSettings>
#include <QFileInfo>
#include <QDir>

#define MODEL_START_REFRESH() m_ready = false; Q_EMIT readyChanged();
#define MODEL_END_REFRESH() m_ready = true; Q_EMIT readyChanged();

/*
    Sort available updates first, then by name
*/
bool sortPackage(const LocalPackageItem &a, const LocalPackageItem &b)
{
    if (a.updateStatus == b.updateStatus) {
        return (a.name.compare(b.name, Qt::CaseInsensitive) < 0);
    }

    return (a.updateStatus.compare(b.updateStatus) < 0);
}

LocalPackagesModel::LocalPackagesModel(QAbstractListModel *parent)
    : QAbstractListModel(parent), m_ready(false), m_appStoreUpdateAvailable(false)
{
    connect(PlatformIntegration::instance(), &PlatformIntegration::updated, this, &LocalPackagesModel::refresh);
    connect(PackagesCache::instance(), &PackagesCache::updatingCacheChanged, this, &LocalPackagesModel::refresh);

    refresh();
}

QHash<int, QByteArray> LocalPackagesModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles.insert(RoleName, "name");
    roles.insert(RoleAppId, "appId");
    roles.insert(RoleVersion, "version");
    roles.insert(RoleIcon, "icon");
    roles.insert(RoleUpdateAvailable, "updateAvailable");
    roles.insert(RoleUpdateStatus, "updateStatus");
    roles.insert(RolePackageUrl, "packageUrl");
    roles.insert(RoleAppLaunchUrl, "appLaunchUrl");

    return roles;
}

int LocalPackagesModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return m_list.count();
}

QVariant LocalPackagesModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() > rowCount())
        return QVariant();

    auto pkg = m_list.at(index.row());

    switch (role) {
    case RoleName:
        return pkg.name;
    case RoleAppId:
        return pkg.appId;
    case RoleVersion:
        return pkg.version;
    case RoleIcon:
        return pkg.icon;
    case RoleUpdateAvailable:
        return pkg.updateAvailable;
    case RoleUpdateStatus:
        return pkg.updateStatus;
    case RolePackageUrl:
        return pkg.packageUrl;
    case RoleAppLaunchUrl:
        return pkg.appLaunchUrl;

    default:
        return QVariant();
    }
}

int LocalPackagesModel::updatesAvailableCount() const
{
    int result = 0;

    Q_FOREACH (const LocalPackageItem &pkg, m_list) {
        if (pkg.updateStatus == QStringLiteral("available")) {
            ++result;
        }
    }

    return result;
}

int LocalPackagesModel::downgradesAvailableCount() const
{
    int result = 0;

    Q_FOREACH (const LocalPackageItem &pkg, m_list) {
        if (pkg.updateStatus == QStringLiteral("downgrade")) {
            ++result;
        }
    }

    return result;
}

void LocalPackagesModel::refresh()
{
    //qDebug() << Q_FUNC_INFO << "called";

    MODEL_START_REFRESH();

    beginResetModel();
    m_list.clear();
    endResetModel();

    const QVariantList &clickDb = PlatformIntegration::instance()->clickDb();

    beginInsertRows(QModelIndex(), m_list.count(), m_list.count() + PackagesCache::instance()->numberOfInstalledAppsInStore() - 1);
    Q_FOREACH(const QVariant &pkg, clickDb) {
        QVariantMap map = pkg.toMap();
        QString appId = map.value("name").toString();
        QString version = map.value("version").toString();

        QVariantMap hookMap = map.value("hooks").toMap();
        QString appLaunchUrl;
        Q_FOREACH (const QString &key, hookMap.keys())
        {
            QVariantMap hook = hookMap.value(key).toMap();
            if (hook.keys().contains("desktop")) {
                appLaunchUrl = QString("appid://%1/%2/%3")
                    .arg(appId)
                    .arg(key)
                    .arg(version);
            }
        }

        LocalPackageItem pkgItem;
        pkgItem.appId = appId;
        pkgItem.name = map.value("title").toString();
        pkgItem.version = version;
        pkgItem.packageUrl = PackagesCache::instance()->getPackageUrl(pkgItem.appId);
        pkgItem.appLaunchUrl = appLaunchUrl;

        int remoteRevision = PackagesCache::instance()->getRemoteAppRevision(pkgItem.appId);
        int localRevision = PackagesCache::instance()->getLocalAppRevision(pkgItem.appId);
        pkgItem.updateAvailable = bool(remoteRevision > localRevision);

        if (localRevision == 0) {
            pkgItem.updateStatus = QStringLiteral("downgrade");
        }
        else if (pkgItem.updateAvailable) {
            pkgItem.updateStatus = QStringLiteral("available");
        }
        else {
            pkgItem.updateStatus = QStringLiteral("none");
        }

        //pkgItem.icon = map.value("icon").toString();
        if (pkgItem.icon.isEmpty()) {
            const QString &directory = map.value("_directory").toString();

            const QVariantMap &hooks = map.value("hooks").toMap();
            Q_FOREACH(const QString &hook, hooks.keys()) {
                const QVariantMap &h = hooks.value(hook).toMap();

                const QString &desktop = h.value("desktop").toString();
                if (!desktop.isEmpty()) {
                    //        qDebug() << "Getting icon from .desktop file.";
                    const QString &desktopFile = directory + QDir::separator() + desktop;
                    QSettings appInfo(desktopFile, QSettings::IniFormat);
                    pkgItem.icon = directory + QDir::separator() + appInfo.value("Desktop Entry/Icon").toString();
                    //        qDebug() << pkgItem.icon;
                    break;
                }
            }

            if (!pkgItem.icon.isEmpty()) {
                pkgItem.icon = pkgItem.icon.prepend("file://");
            }

            m_list.append(pkgItem);

            // Check if there's an update for OpenStore
            if (pkgItem.appId == m_appStoreAppId && localRevision != 0) {
                m_appStoreUpdateAvailable = pkgItem.updateAvailable;
                Q_EMIT appStoreUpdateAvailableChanged();
            }
        }
    }
    //        qDebug() << "Finished refresh.";

    std::sort(m_list.begin(), m_list.end(), sortPackage);

    endInsertRows();

    Q_EMIT updated();
    MODEL_END_REFRESH();
}

QVariantMap LocalPackagesModel::get(int row) {
    QHash<int,QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> ittr(names);
    QVariantMap map;
    while (ittr.hasNext()) {
        ittr.next();

        QModelIndex idx = index(row, 0);
        QVariant data = idx.data(ittr.key());
        map[ittr.value()] = data;
    }

    return map;
}

QVariantMap LocalPackagesModel::getByAppId(const QString &appId) {
    for (int i = 0; i < m_list.count(); i++) {
        if (m_list[i].appId == appId) {
            return get(i);
        }
    }

    return QVariantMap();
}
