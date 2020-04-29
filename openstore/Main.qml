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
import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import OpenStore 1.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0
import Ubuntu.Content 1.3
import Ubuntu.Connectivity 1.0

import "Components" as Components

MainView {
    id: root
    applicationName: "openstore.openstore-team"
    anchorToKeyboard: true

    width: units.gu(130)
    height: units.gu(75)

    property var mainPage

    readonly property string appColorText: UbuntuColors.porcelain
    readonly property string    appColor: "#292929"

    function slot_packageFetchError(appId) {
        PackagesCache.packageDetailsReady.disconnect(slot_packageDetailsReady);
        PackagesCache.packageFetchError.disconnect(slot_packageFetchError);

        var app = appModel.getByAppId(appId);
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
            var app = appModel.getByAppId(appId);
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
        mainPage = pageStack.push(Qt.resolvedUrl("DiscoverTab.qml"))

        PlatformIntegration.update()

        if (OpenStoreNetworkManager.isDifferentDomain) {
            var popup = PopupUtils.open(domainWarningComponent)
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
            var popup = PopupUtils.open(installQuestion, root, {fileName: fileName});
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

        Component.onCompleted: OpenStoreNetworkManager.showNsfw = !settings.hideNsfw
    }

    property QtObject appModel: AppModel {
        appStoreAppId: root.applicationName
    }
    property QtObject categoriesModel: CategoriesModel { }
    property QtObject discoverModel: DiscoverModel { }
    property QtObject searchModel: SearchModel { }

    PageStack {
        id: pageStack
    }

    Components.BottomEdgePageStack {
        id: bottomEdgeStack
    }

    /*
    Loader {
        anchors.fill: parent
        z: Number.MAX_VALUE
        active: !OpenStoreNetworkManager.networkAccessible
        sourceComponent: MouseArea {
            // Capture all mouse/touch events beneath 'mainContainer'
            anchors.fill: parent
            onWheel: wheel.accepted = true  // wheel events are not captured by default

            Rectangle {
                anchors.fill: parent
                color: root.backgroundColor

                Components.EmptyState {
                    title: i18n.tr("Slow or no internet connection available")
                    subTitle: i18n.tr("Please check your internet settings and try again")
                    iconName: "airplane-mode"

                    controlComponent: Button {
                        color: theme.palette.normal.positive
                        text: i18n.tr("Close OpenStore")
                        onClicked: Qt.quit()
                    }

                    anchors.centerIn: parent
                }
            }
        }
    }
    */

    Component {
        id: filteredAppPageComponent
        Page {
            id: filteredAppPage
            property alias filterString: filteredAppView.filterString
            property alias sortMode: filteredAppView.sortMode
            property alias category: filteredAppView.category
            header: PageHeader {
                title: filteredAppPage.title
                automaticHeight: false
            }
            FilteredAppView {
                anchors.fill: parent
                anchors.topMargin: filteredAppPage.header.height

                id: filteredAppView
                onAppDetailsRequired: openApp(appId)
            }
        }
    }

    Component {
        id: domainWarningComponent

        Dialog {
            id: warningDialog
            title: i18n.tr("Warning")

            signal accepted();
            signal rejected();

            Label {
                anchors { left: parent.left; right: parent.right }
                wrapMode: Text.WordWrap
                maximumLineCount: Number.MAX_VALUE
                text: i18n.tr("You are currently using a non-standard domain for the OpenStore. This is a development feature. The domain you are using is:")
            }

            Label {
                anchors { left: parent.left; right: parent.right }
                wrapMode: Text.WordWrap
                maximumLineCount: Number.MAX_VALUE
                text: OpenStoreNetworkManager.domain
            }

            Label {
                anchors { left: parent.left; right: parent.right }
                wrapMode: Text.WordWrap
                maximumLineCount: Number.MAX_VALUE
                text: i18n.tr("Are you sure you want to continue?")
            }

            Button {
                text: i18n.tr("Yes, I know what I'm doing")
                color: theme.palette.normal.positive
                onClicked: {
                    warningDialog.accepted();
                }
            }

            Button {
                text: i18n.tr("Get me out of here!")
                onClicked: {
                    warningDialog.rejected();
                }
            }
        }
    }

    Component {
        id: installQuestion
        Dialog {
            id: installQuestionDialog
            title: i18n.tr("Install unknown app?")
            text: i18n.tr("Do you want to install the unkown app %1?").arg(fileName)

            property string fileName
            signal accepted();
            signal rejected();

            ActivityIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: running
                running: PlatformIntegration.clickInstaller.busy
            }

            Button {
                text: i18n.tr("Install")
                color: theme.palette.normal.positive
                visible: !PlatformIntegration.clickInstaller.busy
                onClicked: {
                    installQuestionDialog.accepted()
                }
            }
            Button {
                text: i18n.tr("Cancel")
                visible: !PlatformIntegration.clickInstaller.busy
                onClicked: {
                    installQuestionDialog.rejected()
                    PopupUtils.close(installQuestionDialog)
                }
            }

            Connections {
                target: PlatformIntegration.clickInstaller
                onPackageInstalled: PopupUtils.close(installQuestionDialog)
                onPackageInstallationFailed: PopupUtils.close(installQuestionDialog)
            }
        }
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

    // *** WORKAROUNDS ***

    // Placed in MainView for convenience.
    function flickable_responsive_scroll_fix(flickable) {
        // WORKAROUND: Fix for wrong grid unit size
        flickable.flickDeceleration = 1500 * units.gridUnit / 8
        flickable.maximumFlickVelocity = 2500 * units.gridUnit / 8
    }
}
