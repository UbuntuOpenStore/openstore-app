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

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import OpenStore 1.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0
import Ubuntu.Content 1.3
import Ubuntu.Connectivity 1.0

import "Components" as Components
import "Dialogs" as Dialogs

MainView {
    id: root
    applicationName: "openstore.openstore-team"
    anchorToKeyboard: true

    //Width must be !isBigScreen for the PageStacks to resize correctly
    width: units.gu(50)
    height: units.gu(75)

    property var mainPage
    property bool isLandscape: width > height
    property bool isBigScreen: width > units.gu(70)
    property string mainStackPage

    readonly property string appColorText: UbuntuColors.porcelain
    readonly property string    appColor: "#292929"

    property alias apiKey: settings.apiKey

    function slot_packageFetchError(appId) {
        PackagesCache.packageDetailsReady.disconnect(slot_packageDetailsReady);
        PackagesCache.packageFetchError.disconnect(slot_packageFetchError);

        var app = localAppModel.getByAppId(appId);
        if (app.appId) {
            bottomEdgeStack.clear();
            bottomEdgeStack.push(Qt.resolvedUrl("AppLocalDetailsPage.qml"), { app: app });
        }
    }

    function slot_packageDetailsReady(pkg) {
        PackagesCache.packageDetailsReady.disconnect(slot_packageDetailsReady);
        PackagesCache.packageFetchError.disconnect(slot_packageFetchError);

        bottomEdgeStack.clear();
        bottomEdgeStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), { app: pkg });
    }

    function openApp(appId) {
        if (Connectivity.online) {
            PackagesCache.packageDetailsReady.connect(slot_packageDetailsReady);
            PackagesCache.packageFetchError.connect(slot_packageFetchError);
            PackagesCache.getPackageDetails(appId);
        }
        else {
            var app = localAppModel.getByAppId(appId);
            if (app.appId) {
                bottomEdgeStack.clear();
                bottomEdgeStack.push(Qt.resolvedUrl("AppLocalDetailsPage.qml"), { app: app });
            }
        }
    }

    function parseUrl(url) {
        var result = "";
        if (url.match(/^(openstore|http[s]?):\/\/(open-store\.io\/app\/)?.*/)) {
            if (url[url.length - 1] == '/') {
                url = url.substr(0, url.length - 1);
            }

            // Get last part of path as ID to open, and strip the /
            result = url.substr(url.lastIndexOf("/") + 1);
        }

        return result;
    }

    function loadAppId(appId) {
        if (appId == 'my-apps' || appId == 'updates') {
            console.log('Opening "My Apps" from URL request');

            bottomEdgeStack.clear();
            mainPage.openMyApps();
        }
        else {
            console.log("Fetching package details for %1".arg(appId));
            openApp(appId);
        }
    }

    Component.onCompleted: {
        pageStack.push(Qt.resolvedUrl("DiscoverPage.qml"))
        PlatformIntegration.update()

        if (OpenStoreNetworkManager.isDifferentDomain) {
            var popup = PopupUtils.open(domainWarningDialog)
            popup.accepted.connect(function() {
                PopupUtils.close(popup)
            })
            popup.rejected.connect(function() {
                Qt.quit();
            })
        }

        var appIdToOpen = "";
        var appArgs = Qt.application.arguments;
        for (var i = 0; i < appArgs.length; i++) {
            appIdToOpen = parseUrl(appArgs[i]);
        }

        if (appIdToOpen != "") {
            loadAppId(appIdToOpen);
        }
    }

    property bool contentHubInstallInProgress: false

    Connections {
        target: ContentHub

        onImportRequested: {
            var filePath = String(transfer.items[0].url).replace('file://', '')
            print("Should import file", filePath)
            var fileName = filePath.split("/").pop();
            var popup = PopupUtils.open(installWarningDialog, root, {fileName: fileName});
            popup.accepted.connect(function() {
                contentHubInstallInProgress = true;
                PlatformIntegration.clickInstaller.installPackage(filePath, true)
            })
        }
    }

    Connections {
        target: UriHandler
        onOpened: {
            var appIdToOpen = parseUrl(uris[0]);
            if (appIdToOpen) {
                loadAppId(appIdToOpen);
            }
        }
    }

    Connections {
        target: PlatformIntegration.clickInstaller

        onPackageInstalled: {
            print("******* package installed")
            if (contentHubInstallInProgress) {
                PopupUtils.open(installedConfirmation, root)
                contentHubInstallInProgress = false;
            }
        }
        onPackageInstallationFailed: {
            if (contentHubInstallInProgress) {
                PopupUtils.open(installationError, root)
                contentHubInstallInProgress = false;
            }
        }
    }

    Connections {
        target: OpenStoreNetworkManager
        onNetworkAccessibleChanged: console.log("[OpenStoreNetworkManager] Is network accessible?", OpenStoreNetworkManager.networkAccessible)
    }

    Binding {
        target: OpenStoreNetworkManager
        property: "showNsfw"
        value: !settings.hideNsfw
    }

    Settings {
        id: settings
        property bool firstStart: true
        property bool hideNsfw: true
        property string apiKey: ""

        Component.onCompleted: OpenStoreNetworkManager.showNsfw = !settings.hideNsfw
    }

    property QtObject localAppModel: LocalAppModel {
        appStoreAppId: root.applicationName
    }
    property QtObject categoriesModel: CategoriesModel { }
    property QtObject discoverModel: DiscoverModel { }
    property QtObject searchModel: SearchModel { }

    PageStack {
        id: pageStack

        width: isBigScreen && bottomEdgeStack.isStackVisible
            ? parent.width - bottomEdgeStack.width
            : 0 //Hack. Width must be inferior to the parent.width
        anchors {
            fill: isBigScreen && bottomEdgeStack.isStackVisible
                ? undefined
                : parent
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }

        onDepthChanged: bottomEdgeStack.clear();
        onCurrentPageChanged: mainStackPage = currentPage.objectName
    }

    Components.BottomEdgePageStack {
        id: bottomEdgeStack
        showDivider: isBigScreen && bottomEdgeStack.isStackVisible

        width: isBigScreen && bottomEdgeStack.isStackVisible
            ? units.gu(47)
            : 0 //Hack. Width must be inferior to the parent.width
        anchors {
            fill: isBigScreen && bottomEdgeStack.isStackVisible
                ? undefined
                : parent
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
    }

    Component {
        id: filteredAppPageComponent

        Page {
            id: filteredAppPage

            property alias filterString: filteredAppList.filterString
            property alias sortMode: filteredAppList.sortMode
            property alias category: filteredAppList.category

            header: Components.HeaderBase {
                title: filteredAppPage.title
                automaticHeight: false

                trailingActionBar {
                    actions: [
                        Action {
                            iconName: 'filters'
                            text: i18n.tr('Filters')

                            onTriggered: {
                                var popup = PopupUtils.open(filterDialog, filteredAppPage, {
                                    selectedSort: filteredAppList.sortMode ? filteredAppList.sortMode : 'name',
                                    selectedType: filteredAppList.filterType,
                                });
                                popup.accepted.connect(function(selectedSort, selectedType) {
                                    PopupUtils.close(popup);
                                    filteredAppList.sortMode = selectedSort;
                                    filteredAppList.filterType = selectedType;
                                });
                                popup.rejected.connect(function(selectedSort, selectedType) {
                                    PopupUtils.close(popup);
                                });
                            }
                        }
                    ]
                }
            }

            FilteredAppList {
                anchors.fill: parent
                anchors.topMargin: filteredAppPage.header.height

                id: filteredAppList
                onAppDetailsRequired: openApp(appId)
            }

            Dialogs.FilterDialog {
                id: filterDialog
            }
        }
    }

    Dialogs.DomainWarningDialog {
        id: domainWarningDialog
    }

    Dialogs.InstallWarningDialog {
        id: installWarningDialog
    }

    Component {
        id: installedConfirmation
        Dialog {
            id: installedConfirmationDialog
            title: i18n.tr("App installed")
            text: i18n.tr("The app has been installed successfully.")
            Button {
                color: theme.palette.normal.positive
                text: i18n.tr("OK")
                onClicked: PopupUtils.close(installedConfirmationDialog)
            }
        }
    }
    Component {
        id: installationError
        Dialog {
            id: installationErrorDialog
            title: i18n.tr("Installation failed")
            text: i18n.tr("The package could not be installed. Make sure it is a valid click package.")
            Button {
                color: theme.palette.normal.positive
                text: i18n.tr("OK")
                onClicked: PopupUtils.close(installationErrorDialog)
            }
        }
    }
    Component {
        id: timeoutError
        Dialog {
            id: timeoutErrorDialog
            property int errorCode
            property string errorString
            title: i18n.tr("Installation failed (Error %1)").arg(errorCode)
            text: errorString
            Button {
                color: theme.palette.normal.positive
                text: i18n.tr("OK")
                onClicked: PopupUtils.close(timeoutErrorDialog)
            }
        }
    }

    //Function from mainPage

    function showCategory(name, id) {
        if (root.mainStackPage !== "discoverPage") {
            pageStack.pop()
        }
        pageStack.push(Qt.resolvedUrl("CategoriesPage.qml"));
        bottomEdgeStack.push(filteredAppPageComponent, {"title": name, "category": id});
        //CategoriesPage.categoryClicked(name, id)
    }
    function showSearch(text) {
        if (root.mainStackPage !== "discoverPage") {
            pageStack.pop()
        }
        pageStack.push(Qt.resolvedUrl("../SearchPage.qml"), {"searchText": text || ''});
    }

    function showSearchQuery(url) {
        pageStack.push(Qt.resolvedUrl("../SearchPage.qml"), {"queryUrl": url || ''});
    }

    // *** WORKAROUNDS ***

    // Placed in MainView for convenience.
    function flickable_responsive_scroll_fix(flickable) {
        // WORKAROUND: Fix for wrong grid unit size
        flickable.flickDeceleration = 1500 * units.gridUnit / 8
        flickable.maximumFlickVelocity = 2500 * units.gridUnit / 8
    }
}
