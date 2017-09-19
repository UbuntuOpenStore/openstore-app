#ifndef PACKAGESMODEL_H
#define PACKAGESMODEL_H

#include <QAbstractListModel>
#include <QJsonObject>

class PackageItem;

struct LocalPackageItem {
    QString name;
    QString appId;
    QString icon;
    bool updateAvailable;
};

class PackagesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(PackagesModel)
    Q_PROPERTY(int count READ rowCount NOTIFY updated)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(int updatesAvailableCount READ updatesAvailableCount NOTIFY updated)

public:
    enum Roles {
        RoleName,
        RoleAppId,
        RoleIcon,
        RoleUpdateAvailable,
    };

    explicit PackagesModel(QAbstractListModel *parent = 0);

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    bool ready() const { return m_ready; }
    int updatesAvailableCount() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void showPackageDetails(const QString &appId);


Q_SIGNALS:
    void countChanged();
    void readyChanged();
    void updated();
    void packageDetailsReady(PackageItem* pkg);

private:
    QList<LocalPackageItem> m_list;
    QHash<QString, int> m_remoteAppRevision; // appid, revision
    QHash<QString, int> m_localAppRevision; // appid, revision

    QString m_signature;

    bool m_ready;
};

#endif // PACKAGESMODEL_H
