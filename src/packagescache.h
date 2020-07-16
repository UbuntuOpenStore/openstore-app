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

#ifndef PACKAGESCACHE_H
#define PACKAGESCACHE_H

#include <QObject>
#include <QHash>

    class PackageItem;

class PackagesCache : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool updatingCache READ updatingCache NOTIFY updatingCacheChanged)

public:
    PackagesCache();
    static PackagesCache* instance();

    bool updatingCache() const { return m_updatingCache; }

    bool contains(const QString &appId) const;
    PackageItem* insert(const QString &appId, const QVariantMap &jsonMap);
    PackageItem* get(const QString &appId) const;

    int getLocalAppRevision(const QString &appId) const { return m_localAppRevision.value(appId, -1); }
    int getRemoteAppRevision(const QString &appId) const { return m_remoteAppRevision.value(appId, -1); }
    QString getPackageUrl(const QString &appId) const { return m_packageUrls.value(appId, QStringLiteral("")); }

    int numberOfInstalledAppsInStore() const { return m_remoteAppRevision.count(); }

    Q_INVOKABLE void getPackageDetails(const QString &appId);

Q_SIGNALS:
    void updatingCacheChanged();
    void packageDetailsReady(PackageItem *pkg);
    void packageFetchError(QString appId);

private Q_SLOTS:
    void updateCacheRevisions();

private:
    QHash<QString, PackageItem*> m_cache;
    QHash<QString, int> m_remoteAppRevision; // appid, revision
    QHash<QString, int> m_localAppRevision; // appid, revision
    QHash<QString, QString> m_packageUrls; // appid, packageUrl

    QString m_signature;
    bool m_updatingCache;
};

#endif // PACKAGESCACHE_H
