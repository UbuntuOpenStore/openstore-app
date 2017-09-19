#ifndef PACKAGE_H
#define PACKAGE_H

#include <QObject>
#include <QVariantMap>
#include <QJsonDocument>
#include <QDebug>
#include <QDateTime>

#if QT_VERSION <= 0x050600
#include "thirdparty/qversionnumber.h"
#else
#include <QVersionNumber>
#endif

class PackageItem: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY updated)
    Q_PROPERTY(QString icon READ icon NOTIFY updated)
    Q_PROPERTY(QString appId READ appId NOTIFY updated)
    Q_PROPERTY(QString author READ author NOTIFY updated)
    Q_PROPERTY(QString tagline READ tagline NOTIFY updated)
    Q_PROPERTY(QString description READ description NOTIFY updated)
    Q_PROPERTY(QString category READ category NOTIFY updated)
    Q_PROPERTY(QStringList screenshots READ screenshots NOTIFY updated)
    Q_PROPERTY(QString changelog READ changelog NOTIFY updated)
    Q_PROPERTY(QString versionString READ versionString NOTIFY updated)
    Q_PROPERTY(QString installedVersionString READ installedVersionString NOTIFY updated)
    Q_PROPERTY(int revision READ revision NOTIFY updated)
    Q_PROPERTY(int installedRevision READ installedRevision NOTIFY updated)
    Q_PROPERTY(QString packageUrl READ packageUrl NOTIFY updated)
    Q_PROPERTY(QString source READ source NOTIFY updated)
    Q_PROPERTY(QString license READ license NOTIFY updated)
    Q_PROPERTY(QString maintainer READ maintainer NOTIFY updated)
    Q_PROPERTY(int fileSize READ fileSize NOTIFY updated)
    Q_PROPERTY(bool installed READ installed NOTIFY updated)
    Q_PROPERTY(int hooksCount READ hooksCount NOTIFY updated)
    Q_PROPERTY(bool containsApp READ containsApp NOTIFY updated)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updated)
    Q_PROPERTY(QDateTime publishedDate READ publishedDate NOTIFY updated)
    Q_PROPERTY(QDateTime updatedDate READ updatedDate NOTIFY updated)

    Q_ENUMS(Hook)
    Q_FLAGS(Hooks)
public:
    enum Hook {
        HookNone = 0,
        HookDesktop = 1,
        HookScope = 2,
        HookAccountService = 4,
        HookUrls = 8,
        HookContentHub = 16,
        HookPushHelper = 32
    };
    Q_DECLARE_FLAGS(Hooks, Hook)

    struct HookStruct {
        QString name;
        Hooks hooks;
        QStringList permissions;
        QString apparmorTemplate;
        QStringList readPaths;
        QStringList writePaths;
    };

    explicit PackageItem(const QVariantMap &json, QObject *parent = 0);
    ~PackageItem();

    QString appId() const { return m_appId; }
    QString name() const { return m_name; }
    QString author() const { return m_author; }
    QString icon() const { return m_icon; }
    QString tagline() const { return m_tagline; }
    QString description() const { return m_description; }
    QString category() const { return m_category; }
    QStringList screenshots() const { return m_screenshots; }
    QString changelog() const { return m_changelog; }
    QString versionString() const { return m_version; }
    QString installedVersionString() const { return m_installedVersion; }
    int revision() const { return m_revision; }
    int installedRevision() const { return m_installedRevision; }
    QString packageUrl() const { return m_packageUrl; }
    QString source() const { return m_source; }
    QString license() const { return m_license; }
    QString maintainer() const { return m_maintainer; }
    int fileSize() const { return m_fileSize; }
    void setFileSize(int fileSize) { m_fileSize = fileSize; }
    bool installed() const { return !m_installedVersion.isNull(); }
    QDateTime publishedDate() const { return m_publishedDate; }
    QDateTime updatedDate() const { return m_updatedDate; }

    Q_INVOKABLE QString permissions(int index) const { return m_hooks.at(index).permissions.join(", "); }
    Q_INVOKABLE Hooks hooks(int index) const { return m_hooks.at(index).hooks; }
    Q_INVOKABLE QString hookName(int index) { return m_hooks.at(index).name; }
    Q_INVOKABLE QString apparmorTemplate(int index) { return m_hooks.at(index).apparmorTemplate; }
    Q_INVOKABLE QString readPaths(int index) { return m_hooks.at(index).readPaths.join(", "); }
    Q_INVOKABLE QString writePaths(int index) { return m_hooks.at(index).writePaths.join(", "); }
    int hooksCount() const { return m_hooks.count(); }
    bool containsApp() const { Q_FOREACH (const HookStruct &hook, m_hooks) { if (hook.hooks & HookDesktop) return true; } return false; }
    bool updateAvailable() const { return !m_installedVersion.isEmpty() && (m_revision > m_installedRevision); }

    Q_INVOKABLE bool install() const;
    Q_INVOKABLE bool remove() const;
    Q_INVOKABLE QString appLaunchUrl() const;

Q_SIGNALS:
    void updated();
    void installedChanged();

public Q_SLOTS:
    void updateLocalInfo(int localRevision, const QString &localVersion);

private Q_SLOTS:
    void fillData(const QVariantMap &json);

private:
    QString m_appId;
    QString m_name;
    QString m_icon;
    QString m_author;
    QString m_tagline;
    QString m_description;
    QString m_category;
    QStringList m_screenshots;
    QString m_changelog;
    QString m_packageUrl;
    int m_revision;
    QString m_version;
    QString m_source;
    QString m_license;
    QString m_maintainer;
    int m_fileSize;
    int m_installedRevision;
    QString m_installedVersion;
    QList<HookStruct> m_hooks;
    QDateTime m_publishedDate;
    QDateTime m_updatedDate;
};

#endif // PACKAGE_H
