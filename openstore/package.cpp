#include "package.h"

#include "platformintegration.h"
#include "clickinstaller.h"

PackageItem::PackageItem(const QVariantMap &json, QObject *parent)
    : QObject(parent)
{
    connect(PlatformIntegration::instance(), &PlatformIntegration::updated, this, &PackageItem::updateLocalInformations);
    fillData(json);
}

PackageItem::~PackageItem()
{
    //qDebug() << Q_FUNC_INFO << m_appId;
}

bool PackageItem::install() const
{
    ClickInstaller* installer = PlatformIntegration::instance()->clickInstaller();

    if (installer->busy()) {
        qDebug() << Q_FUNC_INFO << "Installer is busy...";
        return false;
    }

    if (packageUrl().isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Package url for" << appId() << "not valid";
        return false;
    }

    installer->installPackage(packageUrl());
    return true;
}

bool PackageItem::remove() const
{
    ClickInstaller* installer = PlatformIntegration::instance()->clickInstaller();

    if (installer->busy()) {
        qDebug() << Q_FUNC_INFO << "Installer is busy...";
        return false;
    }

    if (packageUrl().isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Package url for" << appId() << "not valid";
        return false;
    }

    installer->removePackage(appId(), installedVersionString());
    return true;
}

QString PackageItem::appLaunchUrl() const
{
    if (m_installedVersion.toString().isEmpty())
        return QString();

    Q_FOREACH(const HookStruct &h, m_hooks) {
        if (h.hooks & PackageItem::HookDesktop) {
            return "appid://" + m_appId + "/" + h.name + "/" + m_installedVersion.toString();
        }
    }

    return QString();
}

void PackageItem::fillData(const QVariantMap &json)
{
    m_appId = json.value("id").toString();

    m_icon = json.value("icon").toString();
    m_name = json.value("name").toString();
    m_author = json.value("author").toString();

    m_packageUrl = json.value("package").toString();    // Used by the 'discover' API
    if (m_packageUrl.isEmpty()) {
        m_packageUrl = json.value("download").toString();
    }

    m_source = json.value("source").toString();
    m_license = json.value("license").toString();
    m_maintainer = json.value("maintainer_name").toString();
    m_tagline = json.value("tagline").toString();
    m_description = json.value("description").toString();
    m_category = json.value("category").toString();
    m_screenshots = json.value("screenshots").toStringList();
    m_changelog = json.value("changelog").toString();
    m_version = QVersionNumber::fromString(json.value("version").toString());
    m_fileSize = json.value("filesize").toInt();
    m_installedVersion = PlatformIntegration::instance()->appVersion(m_appId);
    m_publishedDate = json.value("published_date").toDateTime();
    m_updatedDate = json.value("updated_date").toDateTime();

    QList<PackageItem::HookStruct> hooksList;
    if (json.contains("manifest") && json.value("manifest").toMap().contains("hooks")) {
        QVariantMap hookMap = json.value("manifest").toMap().value("hooks").toMap();

        Q_FOREACH (const QString & hook, hookMap.keys()) {
            PackageItem::HookStruct hookStruct;
            hookStruct.name = hook;
            hookStruct.hooks = PackageItem::HookNone;

            QStringList permissions;
            QStringList readPaths;
            QStringList writePaths;

            QVariantMap apparmorMap = hookMap.value(hook).toMap().value("apparmor").toMap();

            //                qDebug() << "have apparmor for" << hook << apparmorMap;
            Q_FOREACH (const QVariant &perm, apparmorMap.value("policy_groups").toList()) {
                permissions.append(perm.toString());
            }

            Q_FOREACH (const QVariant &perm, apparmorMap.value("read_path").toList()) {
                readPaths.append(perm.toString());
            }

            Q_FOREACH (const QVariant &perm, apparmorMap.value("write_path").toList()) {
                writePaths.append(perm.toString());
            }

            hookStruct.apparmorTemplate = apparmorMap.value("template").toString();
            hookStruct.readPaths = readPaths;
            hookStruct.writePaths = writePaths;
            hookStruct.permissions = permissions;

            if (hookMap.value(hook).toMap().contains("desktop")) {
                hookStruct.hooks |= PackageItem::HookDesktop;
            }
            if (hookMap.value(hook).toMap().contains("scope")) {
                hookStruct.hooks |= PackageItem::HookScope;
            }
            if (hookMap.value(hook).toMap().contains("content-hub")) {
                hookStruct.hooks |= PackageItem::HookContentHub;
            }
            if (hookMap.value(hook).toMap().contains("urls")) {
                hookStruct.hooks |= PackageItem::HookUrls;
            }
            if (hookMap.value(hook).toMap().contains("push-helper")) {
                hookStruct.hooks |= PackageItem::HookPushHelper;
            }
            if (hookMap.value(hook).toMap().contains("account-provider")) {
                hookStruct.hooks |= PackageItem::HookAccountService;
            }
            hooksList.append(hookStruct);
        }

        m_hooks = hooksList;
    }

    Q_EMIT updated();
}

void PackageItem::updateLocalInformations()
{
    const QVersionNumber &localAppVersion = PlatformIntegration::instance()->appVersion(m_appId);

    if (!localAppVersion.isNull()) {
        m_installedVersion = localAppVersion;
        Q_EMIT updated();
        Q_EMIT installedChanged();
    }
}
