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
    Q_PROPERTY(QString appStoreAppId MEMBER m_appStoreAppId NOTIFY appStoreAppIdChanged)
    Q_PROPERTY(bool appStoreUpdateAvailable READ appStoreUpdateAvailable NOTIFY appStoreUpdateAvailableChanged)

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

    bool appStoreUpdateAvailable() const { return m_appStoreUpdateAvailable; }

    Q_INVOKABLE void refresh();

Q_SIGNALS:
    void countChanged();
    void readyChanged();
    void updated();
    void appStoreAppIdChanged();
    void appStoreUpdateAvailableChanged();

private:
    QList<LocalPackageItem> m_list;
    QString m_appStoreAppId;
    bool m_ready;
    bool m_appStoreUpdateAvailable;
};

#endif // PACKAGESMODEL_H
