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

#ifndef DISCOVERMODEL_H
#define DISCOVERMODEL_H

#include <QAbstractListModel>

#include "../openstorenetworkmanager.h"

    class QNetworkReply;
class PackageItem;

struct DiscoverCategoryItem {
    QString name;
    QString tagline;
    QString queryUrl;
    QStringList appIds;
};

class DiscoverModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl highlightBannerUrl READ highlightBannerUrl NOTIFY updated)
    Q_PROPERTY(QString highlightAppId READ highlightAppId NOTIFY updated)
    Q_PROPERTY(bool ready READ ready NOTIFY updated)

public:
    enum Roles {
        RoleName,
        RoleTagline,
        RoleQueryUrl,
        RoleAppIds
    };

    explicit DiscoverModel(QObject *parent = 0);

    QUrl highlightBannerUrl() const { return m_highlightBannerUrl; }
    QString highlightAppId() const { return m_highlightAppId; }
    bool ready() const { return m_ready; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    Q_INVOKABLE PackageItem* getPackage(const QString &appId);

    Q_INVOKABLE void refresh();

Q_SIGNALS:
    void updated();

private Q_SLOTS:
    void parseReply(OpenStoreReply reply);

private:
    QString m_requestSignature;

    QList<DiscoverCategoryItem> m_list;

    QUrl m_highlightBannerUrl;
    QString m_highlightAppId;
    bool m_ready = false;
};

#endif // DISCOVERMODEL_H
