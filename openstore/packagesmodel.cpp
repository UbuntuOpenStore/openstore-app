#include "packagesmodel.h"
#include "package.h"
#include "platformintegration.h"
#include "openstorenetworkmanager.h"
#include "packagescache.h"

#include <QJsonDocument>
#include <QJsonArray>

// For desktop file / scope settings parsing
#include <QSettings>
#include <QFileInfo>
#include <QDir>

#define MODEL_START_REFRESH() m_ready = false; Q_EMIT readyChanged();
#define MODEL_END_REFRESH() m_ready = true; Q_EMIT readyChanged();

PackagesModel::PackagesModel(QAbstractListModel * parent)
    : QAbstractListModel(parent)
    , m_ready(false)
    , m_appStoreUpdateAvailable(false)
{
    connect(PlatformIntegration::instance(), &PlatformIntegration::updated, this, &PackagesModel::refresh);
    connect(PackagesCache::instance(), &PackagesCache::updatingCacheChanged, this, &PackagesModel::refresh);

    refresh();
}

QHash<int, QByteArray> PackagesModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles.insert(RoleName, "name");
    roles.insert(RoleAppId, "appId");
    roles.insert(RoleIcon, "icon");
    roles.insert(RoleUpdateAvailable, "updateAvailable");

    return roles;
}

int PackagesModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return m_list.count();
}

QVariant PackagesModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() > rowCount())
        return QVariant();

    auto pkg = m_list.at(index.row());

    switch (role) {
    case RoleName:
        return pkg.name;
    case RoleAppId:
        return pkg.appId;
    case RoleIcon:
        return pkg.icon;
    case RoleUpdateAvailable:
        return pkg.updateAvailable;

    default:
        return QVariant();
    }
}

int PackagesModel::updatesAvailableCount() const
{
    int result = 0;

    Q_FOREACH (const LocalPackageItem &pkg, m_list) {
        if (pkg.updateAvailable) {
            ++result;
        }
    }

    return result;
}

void PackagesModel::refresh()
{
    qDebug() << Q_FUNC_INFO << "called";

    MODEL_START_REFRESH();

    beginResetModel();
    m_list.clear();
    endResetModel();

    const QVariantList &clickDb = PlatformIntegration::instance()->clickDb();

    beginInsertRows(QModelIndex(), m_list.count(), m_list.count() + PackagesCache::instance()->numberOfInstalledAppsInStore() - 1);
    Q_FOREACH(const QVariant &pkg, clickDb) {
        QVariantMap map = pkg.toMap();

        LocalPackageItem pkgItem;
        pkgItem.appId = map.value("name").toString();
        pkgItem.name = map.value("title").toString();
        pkgItem.updateAvailable = bool(PackagesCache::instance()->getRemoteAppRevision(pkgItem.appId) > PackagesCache::instance()->getLocalAppRevision(pkgItem.appId));

        //pkgItem.icon = map.value("icon").toString();
        if (pkgItem.icon.isEmpty()) {
            const QString &directory = map.value("_directory").toString();

            const QVariantMap &hooks = map.value("hooks").toMap();
            Q_FOREACH(const QString &hook, hooks.keys()) {
                const QVariantMap &h = hooks.value(hook).toMap();

                const QString &desktop = h.value("desktop").toString();
                const QString &scope = h.value("scope").toString();
                if (!desktop.isEmpty()) {
                    //        qDebug() << "Getting icon from .desktop file.";
                    const QString &desktopFile = directory + QDir::separator() + desktop;
                    QSettings appInfo(desktopFile, QSettings::IniFormat);
                    pkgItem.icon = directory + QDir::separator() + appInfo.value("Desktop Entry/Icon").toString();
                    //        qDebug() << pkgItem.icon;
                    break;
                } else if (!scope.isEmpty()) {
                    //        qDebug() << "Getting icon from scope.";
                    const QString &scopeFile = directory + QDir::separator() + scope + QDir::separator() + pkgItem.appId + "_" + scope + ".ini";
                    QSettings appInfo(scopeFile, QSettings::IniFormat);
                    QFileInfo fileInfo(scopeFile);

                    const QString &possibleIconFile = appInfo.value("ScopeConfig/Icon").toString();

                    if (!possibleIconFile.isEmpty()) {
                        pkgItem.icon = fileInfo.absolutePath() + QDir::separator() + appInfo.value("ScopeConfig/Icon").toString();
                        //        qDebug() << pkgItem.icon;
                        break;
                    }
                    //        qDebug() << "Icon not found in scope.";
                }
            }

            if (!pkgItem.icon.isEmpty()) {
                pkgItem.icon = pkgItem.icon.prepend("file://");
            }

            m_list.append(pkgItem);

            // Check if there's an update for OpenStore
            if (pkgItem.appId == m_appStoreAppId) {
                m_appStoreUpdateAvailable = pkgItem.updateAvailable;
                Q_EMIT appStoreUpdateAvailableChanged();
            }
        }
    }
    //        qDebug() << "Finished refresh.";

    endInsertRows();

    Q_EMIT updated();
    MODEL_END_REFRESH();
}
