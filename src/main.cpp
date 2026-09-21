/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
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
 */

#include <QElapsedTimer>
#include <QGuiApplication>
#include <QHostInfo>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>

#if QT_VERSION_MAJOR >= 6
#include <QNetworkInformation>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#endif

#ifdef ENABLE_SNAP_SUPPORT
#include <Snapd/Client>
#endif

#include "apiconstants.h"
#include "backendinstaller.h"
#include "cachingnetworkmanagerfactory.h"
#ifdef ENABLE_CLICK_SUPPORT
#include "installers/clickinstaller.h"
#endif
#include "models/categoriesmodel.h"
#include "models/discovermodel.h"
#include "models/localpackagesmodel.h"
#include "models/searchmodel.h"
#include "openstorenetworkmanager.h"
#include "packagebackendmanager.h"
#ifdef ENABLE_CLICK_SUPPORT
#include "packageitems/clickpackage.h"
#endif
#ifdef ENABLE_DEB_SUPPORT
#include "indexstatus.h"
#include "installers/packagekitinstaller.h"
#include "packageindex.h"
#include "packageitems/packagekitpackage.h"
#include "sources/packagekitsource.h"
#endif
#ifdef ENABLE_SNAP_SUPPORT
#include "packageitems/snappackage.h"
#endif
#include "packagescache.h"
#include "platformintegration.h"
#include "review.h"
#include "sources/remoteapisource.h"

static QObject* registerNetworkManagerSingleton(QQmlEngine* /*engine*/, QJSEngine* /*scriptEngine*/)
{
  return OpenStoreNetworkManager::instance();
}

static QObject* registerPlatformIntegrationSingleton(QQmlEngine* /*engine*/, QJSEngine* /*scriptEngine*/)
{
  return PlatformIntegration::instance();
}

static QObject* registerPackagesCacheSingleton(QQmlEngine* /*engine*/, QJSEngine* /*scriptEngine*/)
{
  return PackagesCache::instance();
}

static QObject* registerBackendManagerSingleton(QQmlEngine* /*engine*/, QJSEngine* /*scriptEngine*/)
{
  return PackageBackendManager::instance();
}

static QObject* registerBackendInstallerSingleton(QQmlEngine* /*engine*/, QJSEngine* /*scriptEngine*/)
{
  return BackendInstaller::instance();
}

#ifdef ENABLE_DEB_SUPPORT
static QObject* registerPackageIndexSingleton(QQmlEngine* /*engine*/, QJSEngine* /*scriptEngine*/)
{
  return PackageIndex::instance();
}
#endif

int main(int argc, char* argv[])
{
  QElapsedTimer initTimer;
  initTimer.start();

  QCoreApplication::setApplicationName(QStringLiteral("openstore.openstore-team"));
  QCoreApplication::setApplicationVersion(QString(BUILD_VERSION));
  qDebug() << "OpenStore" << QCoreApplication::applicationVersion();

#if QT_VERSION_MAJOR >= 6
  QtWebEngineQuick::initialize();
#endif

  QGuiApplication app(argc, argv);

#if QT_VERSION_MAJOR >= 6
  QNetworkInformation::loadDefaultBackend();
#endif

  qmlRegisterSingletonType<OpenStoreNetworkManager>("OpenStore", 1, 0, "OpenStoreNetworkManager", registerNetworkManagerSingleton);
  qmlRegisterSingletonType<PlatformIntegration>("OpenStore", 1, 0, "PlatformIntegration", registerPlatformIntegrationSingleton);
  qmlRegisterSingletonType<PackagesCache>("OpenStore", 1, 0, "PackagesCache", registerPackagesCacheSingleton);
  qmlRegisterSingletonType<PackageBackendManager>("OpenStore", 1, 0, "PackageBackendManager", registerBackendManagerSingleton);
  qmlRegisterSingletonType<BackendInstaller>("OpenStore", 1, 0, "BackendInstaller", registerBackendInstallerSingleton);
#ifdef ENABLE_CLICK_SUPPORT
  qmlRegisterUncreatableType<ClickInstaller>(
    "OpenStore", 1, 0, "ClickInstaller", "Access ClickInstall from the PlatformIntegration singleton");
#endif
  qmlRegisterType<LocalPackagesModel>("OpenStore", 1, 0, "LocalAppModel");
  qmlRegisterType<DiscoverModel>("OpenStore", 1, 0, "DiscoverModel");
  qmlRegisterType<SearchModel>("OpenStore", 1, 0, "SearchModel");
  qmlRegisterType<CategoriesModel>("OpenStore", 1, 0, "CategoriesModel");
  qmlRegisterUncreatableType<PackageItem>(
    "OpenStore", 1, 0, "PackageItem", "PackageItem is only available through LocalAppModel, DiscoverModel, or SearchModel.");
#ifdef ENABLE_CLICK_SUPPORT
  qmlRegisterUncreatableType<ClickPackageItem>(
    "OpenStore", 1, 0, "ClickPackageItem", "ClickPackageItem is only available through LocalAppModel, DiscoverModel, or SearchModel.");
#endif
#ifdef ENABLE_SNAP_SUPPORT
  qmlRegisterUncreatableType<SnapPackageItem>(
    "OpenStore", 1, 0, "SnapPackageItem", "SnapPackageItem is only available through LocalAppModel, DiscoverModel, or SearchModel.");
  qmlRegisterUncreatableType<QSnapdClient>("OpenStore", 1, 0, "SnapInstaller", "Access SnapInstall from the PlatformIntegration singleton");
  qmlRegisterUncreatableType<QSnapdRemoveRequest>("OpenStore", 1, 0, "SnapdRemoveRequest", "Created by snapInstaller");
#endif
  qmlRegisterUncreatableType<PackageSource>("OpenStore", 1, 0, "PackageSource", "PackageSource is created by PackageBackendManager.");
#ifdef ENABLE_DEB_SUPPORT
  qmlRegisterUncreatableType<PackageKitSource>(
    "OpenStore", 1, 0, "PackageKitSource", "PackageKitSource is created by PackageBackendManager.");
  qmlRegisterSingletonType<PackageIndex>("OpenStore", 1, 0, "PackageIndex", registerPackageIndexSingleton);
  qmlRegisterUncreatableType<IndexStatus>("OpenStore", 1, 0, "IndexStatus", "Access IndexStatus from PackageIndex.status");
  qmlRegisterUncreatableType<PackageKitInstaller>(
    "OpenStore", 1, 0, "PackageKitInstaller", "Access PackageKitInstaller from the PlatformIntegration singleton");
  qmlRegisterUncreatableType<PackageKitPackageItem>(
    "OpenStore", 1, 0, "PackageKitPackageItem", "PackageKitPackageItem is only available through the PackageKitSource catalog.");
#endif

  qmlRegisterType<Ratings>("OpenStore", 1, 0, "Ratings");
  qRegisterMetaType<Ratings::Rating>("Rating");
  QQuickView view;

  QObject::connect(view.engine(), &QQmlEngine::quit, &app, &QGuiApplication::quit);

  // This applies to QML requests only
  CachingNetworkManagerFactory* managerFactory = new CachingNetworkManagerFactory();
  view.engine()->setNetworkAccessManagerFactory(managerFactory);

  view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
  view.setResizeMode(QQuickView::SizeRootObjectToView);
  view.show();

  qDebug() << "App required" << initTimer.elapsed() << "msec to be initialised.";

  return app.exec();
}
