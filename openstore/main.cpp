/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
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
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QElapsedTimer>
#include <QHostInfo>

#include "apiconstants.h"
#include "platformintegration.h"
#include "clickinstaller.h"
#include "searchmodel.h"
#include "categoriesmodel.h"
#include "packagesmodel.h"
#include "discovermodel.h"
#include "packagescache.h"
#include "openstorenetworkmanager.h"
#include "pamauthentication.h"
#include "cachingnetworkmanagerfactory.h"
#include "review.h"

static QObject *registerNetworkManagerSingleton (QQmlEngine * /*engine*/, QJSEngine * /*scriptEngine*/)
{
    return OpenStoreNetworkManager::instance();
}

static QObject *registerPlatformIntegrationSingleton (QQmlEngine * /*engine*/, QJSEngine * /*scriptEngine*/)
{
    return PlatformIntegration::instance();
}

static QObject *registerPackagesCacheSingleton (QQmlEngine * /*engine*/, QJSEngine * /*scriptEngine*/)
{
    return PackagesCache::instance();
}

int main(int argc, char *argv[])
{
    QElapsedTimer initTimer;
    initTimer.start();

    QCoreApplication::setApplicationName(QStringLiteral("openstore.openstore-team"));

    QGuiApplication app(argc, argv);

    qmlRegisterSingletonType<OpenStoreNetworkManager>("OpenStore", 1, 0, "OpenStoreNetworkManager", registerNetworkManagerSingleton);
    qmlRegisterSingletonType<PlatformIntegration>("OpenStore", 1, 0, "PlatformIntegration", registerPlatformIntegrationSingleton);
    qmlRegisterSingletonType<PackagesCache>("OpenStore", 1, 0, "PackagesCache", registerPackagesCacheSingleton);
    qmlRegisterUncreatableType<ClickInstaller>("OpenStore", 1, 0, "ClickInstaller", "Access ClickInstall from the PlatformIntegration singleton");
    qmlRegisterType<PackagesModel>("OpenStore", 1, 0, "AppModel");
    qmlRegisterType<DiscoverModel>("OpenStore", 1, 0, "DiscoverModel");
    qmlRegisterType<SearchModel>("OpenStore", 1, 0, "SearchModel");
    qmlRegisterType<CategoriesModel>("OpenStore", 1, 0, "CategoriesModel");
    qmlRegisterUncreatableType<PackageItem>("OpenStore", 1, 0, "PackageItem", "PackageItem is only available through AppModel, DiscoverModel, or SearchModel.");

    qmlRegisterType<PamAuthentication>("OpenStore.PamAuthentication", 0, 1, "PamAuthentication");
    //qmlRegisterType<ReviewItem>("OpenStore", 1, 0, "ReviewItem");
    qRegisterMetaType<ReviewItem::Rating>("Rating");
    QQuickView view;

    QObject::connect(view.engine(), &QQmlEngine::quit, &app, &QGuiApplication::quit);

    // This applies to QML requests only
    CachingNetworkManagerFactory *managerFactory = new CachingNetworkManagerFactory();
    view.engine()->setNetworkAccessManagerFactory(managerFactory);


    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();

    qDebug() << "App required" << initTimer.elapsed() << "msec to be initialised.";

    return app.exec();
}

