#include "platformintegration.h"
#include "clickinstaller.h"
#include "serviceregistry.h"
#include "openstorenetworkmanager.h"

#include <QJsonDocument>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <click.h>
#include <gio/gio.h>
#include <glib.h>

Q_GLOBAL_STATIC(PlatformIntegration, s_platformIntegration)

PlatformIntegration::PlatformIntegration()
{
    m_supportedFrameworks = getSupportedFrameworks();
    m_supportedArchitecture = getSupportedArchitecture();
    m_systemLocale = getSystemLocale();

    m_installer = new ClickInstaller();

    m_serviceRegistry = new ServiceRegistry();
    m_serviceRegistry->setClickInstaller(m_installer);

    connect(m_installer, &ClickInstaller::busyChanged, [=]() {
       if (!m_installer->busy()) {
           this->update();
       }
    });

    update();
}

PlatformIntegration::~PlatformIntegration()
{
    delete m_serviceRegistry;
    delete m_installer;
}

PlatformIntegration *PlatformIntegration::instance()
{
    return s_platformIntegration();
}

void PlatformIntegration::update()
{
    m_installedAppIds.clear();

    ClickDB *clickdb;
    GError *err = nullptr;
    gchar *clickmanifest = nullptr;

    clickdb = click_db_new();
    click_db_read(clickdb, nullptr, &err);

    if (err != nullptr) {
        g_warning("Unable to read Click database: %s", err->message);
        g_error_free(err);
        g_object_unref(clickdb);

        Q_EMIT updated();
        return;
    }

    ClickUser *clickUser = click_user_new_for_user(clickdb, "phablet", &err);

    if (err != nullptr) {
        g_error_free(err);
        g_object_unref(clickdb);
        g_object_unref(clickUser);

        Q_EMIT updated();
        return;
    }

    clickmanifest = click_user_get_manifests_as_string(clickUser, &err);
    g_object_unref(clickdb);
    g_object_unref(clickUser);

    if (err != nullptr) {
        g_warning("Unable to get the manifests: %s", err->message);
        g_error_free(err);

        Q_EMIT updated();
        return;
    }

    QJsonParseError error;

    QJsonDocument jsond = QJsonDocument::fromJson(clickmanifest, &error);

    g_free(clickmanifest);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << error.errorString();

        Q_EMIT updated();
        return;
    }

    //     qDebug() << "loaded stuff" << jsond.toJson();
    m_clickDb = jsond.toVariant().toList();

    qDebug() << "building click list:";
    Q_FOREACH(const QVariant &appJson, m_clickDb) {
        QVariantMap appMap = appJson.toMap();

        QString appId = appMap.value("name").toString();
        QVersionNumber version = QVersionNumber::fromString(appMap.value("version").toString());

        //         qDebug() << "have installed app:" << appId << version << appMap;

        if (!m_installedAppIds.contains(appId) || m_installedAppIds.value(appId) < version) {
            m_installedAppIds[appId] = version;
        }
    }

    Q_EMIT updated();
}

QStringList PlatformIntegration::getSupportedFrameworks()
{
    QStringList result;

    GList *gframeworks = click_framework_get_frameworks();

    while (gframeworks) {
        QString frameworkName = QString::fromUtf8(click_framework_get_name((ClickFramework*)gframeworks->data));
        //        qDebug() << "have framework" << frameworkName;
        result << frameworkName;
        gframeworks = gframeworks->next;
    }

    return result;
}

QString PlatformIntegration::getSupportedArchitecture()
{
    // FIXME: Shouldn't be hardcoded
    return QString("armhf");
}

QString PlatformIntegration::getSystemLocale()
{
    // https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s04.html

    QString locale = qgetenv("LANG");

    if (locale.isEmpty())
        locale = qgetenv("LANGUAGE");

    if (locale.isEmpty())
        locale = qgetenv("LC_MESSAGE");

    if (locale.isEmpty())
        locale = qgetenv("LC_ALL");

    if (!locale.isEmpty()) {
        int i = locale.indexOf("@");
        if (i > -1)
            locale.truncate(i);

        int j = locale.indexOf(".");
        if (j > -1)
            locale.truncate(j);
    }

    qDebug() << Q_FUNC_INFO << locale;

    return locale;
}
