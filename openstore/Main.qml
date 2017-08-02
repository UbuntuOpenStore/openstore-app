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

import "Components" as Components

MainView {
    id: root
    applicationName: "openstore.openstore-team"
    anchorToKeyboard: true

    width: units.gu(130)
    height: units.gu(75)

    property string appIdToOpen
    property var mainPage

    Component.onCompleted: {
        mainPage = pageStack.push(Qt.resolvedUrl("MainPage.qml"))

        PlatformIntegration.update()

        if (settings.firstStart) {
            var popup = PopupUtils.open(warningComponent)
            popup.accepted.connect(function() {
                settings.firstStart = false;
                PopupUtils.close(popup)
            })
            popup.rejected.connect(function() {
                Qt.quit();
            })
        }

        if (cmdArgs[1].length) {
            root.appIdToOpen = cmdArgs[1].split("://")[1];
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
                PlatformIntegration.clickInstaller.installPackage(filePath)
            })
        }
    }

    function slot_packageDetailsReady(pkg) {
        appModel.packageDetailsReady.disconnect(slot_packageDetailsReady)
        bottomEdgeStack.clear()
        bottomEdgeStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), { app: pkg })
    }

    Connections {
        target: UriHandler
        onOpened: {
            var appId = uris[0].split("://")[1]
            console.log("Fetching " + appId + " for UriHandler request")
            appModel.packageDetailsReady.connect(slot_packageDetailsReady)
            appModel.showPackageDetails(appId)
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

    Settings {
        id: settings
        property bool firstStart: true
    }

    property QtObject appModel: AppModel { }
    property QtObject categoriesModel: CategoriesModel { }
    property QtObject discoverModel: DiscoverModel { }
    property QtObject searchModel: SearchModel { }

    PageStack {
        id: pageStack
    }

    Components.BottomEdgePageStack {
        id: bottomEdgeStack
    }

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
                onAppDetailsRequired: {
                    var pageProps = { app: filteredAppView.getPackage(index) }
                    bottomEdgeStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), pageProps)
                }
            }
        }
    }


    Component {
        id: warningComponent

        Dialog {
            id: warningDialog
            title: i18n.tr("Warning")

            signal accepted();
            signal rejected();

            Label {
                anchors { left: parent.left; right: parent.right }
                wrapMode: Text.WordWrap
                maximumLineCount: Number.MAX_VALUE
                text: i18n.tr("OpenStore allows installing unconfined applications. Please make sure that you know about the implications of that.")
            }

            Label {
                anchors { left: parent.left; right: parent.right }
                wrapMode: Text.WordWrap
                maximumLineCount: Number.MAX_VALUE
                text: i18n.tr("An unconfined application has the ability to break the system, reduce its performance and/or spy on you.")
            }

            Label {
                anchors { left: parent.left; right: parent.right }
                wrapMode: Text.WordWrap
                maximumLineCount: Number.MAX_VALUE
                text: i18n.tr("While we are doing our best to prevent that by reviewing applications, we don't take any responsibility if something bad slips through.")
            }

            Label {
                anchors { left: parent.left; right: parent.right }
                wrapMode: Text.WordWrap
                maximumLineCount: Number.MAX_VALUE
                text: i18n.tr("Use this at your own risk.")
            }

            Button {
                text: i18n.tr("Okay. Got it! I'll be careful.")
                color: UbuntuColors.green
                onClicked: {
                    warningDialog.accepted();
                }
            }
            Button {
                text: i18n.tr("Get me out of here!")
                color: UbuntuColors.red
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
            title: i18n.tr("Install app?")
            text: i18n.tr("Do you want to install %1?").arg(fileName)

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
                color: UbuntuColors.green
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
                color: UbuntuColors.blue
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
                color: UbuntuColors.orange
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
                color: UbuntuColors.orange
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
