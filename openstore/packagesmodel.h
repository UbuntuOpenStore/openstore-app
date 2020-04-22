#ifndef PACKAGESMODEL_H
#define PACKAGESMODEL_H

#include <QAbstractListModel>
#include <QJsonObject>

class PackageItem;

struct LocalPackageItem {
    QString name;
    QString appId;
    QString version;
    QString icon;
    bool updateAvailable;
    QString updateStatus;
    QString packageUrl;
};

class PackagesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(PackagesModel)
    Q_PROPERTY(int count READ rowCount NOTIFY updated)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(int updatesAvailableCount READ updatesAvailableCount NOTIFY updated)
    Q_PROPERTY(int downgradesAvailableCount READ downgradesAvailableCount NOTIFY updated)
    Q_PROPERTY(QString appStoreAppId MEMBER m_appStoreAppId NOTIFY appStoreAppIdChanged)
    Q_PROPERTY(bool appStoreUpdateAvailable READ appStoreUpdateAvailable NOTIFY appStoreUpdateAvailableChanged)

public:
    enum Roles {
        RoleName,
        RoleAppId,
        RoleVersion,
        RoleIcon,
        RoleUpdateAvailable,
        RoleUpdateStatus,
        RolePackageUrl,
    };

    explicit PackagesModel(QAbstractListModel *parent = 0);

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    bool ready() const { return m_ready; }
    int updatesAvailableCount() const;
    int downgradesAvailableCount() const;

    bool appStoreUpdateAvailable() const { return m_appStoreUpdateAvailable; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariantMap get(int row);
    Q_INVOKABLE QVariantMap getByAppId(const QString &appId);

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
