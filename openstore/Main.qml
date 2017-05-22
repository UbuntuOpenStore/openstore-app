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
import QtQml.Models 2.1

MainView {
    id: root
    applicationName: "openstore.openstore-team"

    width: units.gu(40)
    height: units.gu(75)

    property string appIdToOpen

    Component.onCompleted: {
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

    Connections {
        target: UriHandler
        onOpened: {
            var index = appModel.findApp(uris[0].split("://")[1])
            if (index >= 0) {
                pageStack.addPageToNextColumn(mainPage, Qt.resolvedUrl("AppDetailsPage.qml"), {app: appModel.app(index)})
            }
        }
    }

    Settings {
        id: settings
        property bool firstStart: true
    }

    ClickInstaller {
        id: installer
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

    AppModel {
        id: appModel
        installer: installer
        onRepositoryListFetched: repoFetchingIndicator.visible = false
    }

    ServiceRegistry {
        id: serviceRegistry
        clickInstaller: installer
    }

    QtObject {
        id: categories

        property string categoriesApiEndPoint: "https://open.uappexplorer.com/api/v1/categories"
        property var list

        Component.onCompleted: {
            var doc = new XMLHttpRequest();
            doc.onreadystatechange = function() {
                if (doc.readyState == 4 && doc.status == 200) {
                    var reply = JSON.parse(doc.responseText)
                    if (reply.success) {
                        list = reply.data
                    } else {
                        console.log("Unable to fetch categories from server (success = false).")
                    }
                }
            }

            doc.open("GET", categoriesApiEndPoint, true);
            doc.send();
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
                installer.installPackage(filePath)
            })
        }
    }
    AdaptivePageLayout {
        id: pageStack
        anchors.fill: parent
        primaryPage: mainPage

        Page {
            id: mainPage
            header: PageHeader {
                title: "OpenStore"  // This is the name of the store - no need for translation.
                automaticHeight: false

                leadingActionBar.actions: Action {
                    iconName: "navigation-menu"
                    text: i18n.tr("Categories")
                    onTriggered: mainPage.pageStack.addPageToCurrentColumn(mainPage, Qt.resolvedUrl("CategoriesPage.qml"), {})
                }

                trailingActionBar.actions: Action {
                    iconName: "find"
                    text: i18n.tr("Search")
                    onTriggered: {
                        mainPage.pageStack.addPageToCurrentColumn(mainPage, searchPageComponent, {})
                    }
                }

                sections {
                    model: [ i18n.tr("Discover"), i18n.tr("My Apps") ]
                    selectedIndex: 0   // Should always match "Discover"
                    onSelectedIndexChanged: {
                        // Current section has changed, if there was an opened page
                        // in the second column, it is not anymore related to the
                        // new current section. Remove it.
                        mainPage.pageStack.removePages(mainPage)
                    }
                }
            }

            ListView {
                id: view
                anchors {
                    top: mainPage.header.bottom
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }

                clip: true
                orientation: ListView.Horizontal
                interactive: false
                snapMode: ListView.SnapOneItem
                highlightMoveDuration: 0
                currentIndex: mainPage.header.sections.selectedIndex

                model: ObjectModel {
                    Loader {
                        id: discoverTabLoader
                        width: view.width
                        height: view.height
                        asynchronous: true
                        source: Qt.resolvedUrl("DiscoverTab.qml")

                        active: false
                        Connections {
                            target: appModel
                            onRepositoryListFetched: discoverTabLoader.active = true
                        }

                        onLoaded: {
                            item.storeModel = appModel

                            item.appDetailsRequired.connect(function(appId) {
                                var pageProps = {
                                    app: appModel.app(appModel.findApp(appId))
                                }
                                mainPage.pageStack.addPageToNextColumn(mainPage, Qt.resolvedUrl("AppDetailsPage.qml"), pageProps)
                            })

                            item.categoryViewRequired.connect(function(name, code) {
                                var pageProps = {
                                    title: name,
                                    filterPattern: new RegExp(code.toString()),
                                    filterProperty: "category"
                                }

                                mainPage.pageStack.removePages(mainPage)
                                mainPage.pageStack.addPageToCurrentColumn(mainPage, filteredAppPageComponent, pageProps)
                            })
                        }
                    }
                    Loader {
                        width: view.width
                        height: view.height
                        asynchronous: true
                        source: Qt.resolvedUrl("FilteredAppView.qml")

                        onLoaded: {
                            item.model = appModel

                            item.filterProperty = "installed"
                            item.filterPattern = new RegExp("true")

                            item.sortProperty = "updateAvailable"
                            item.sortOrder = Qt.DescendingOrder

                            item.view.section.property = "updateAvailable"
                            item.view.section.delegate = updateDivider

                            item.showTicks = false

                            item.appDetailsRequired.connect(function(appId) {
                                var pageProps = {
                                    app: appModel.app(appModel.findApp(appId))
                                }
                                mainPage.pageStack.addPageToNextColumn(mainPage, Qt.resolvedUrl("AppDetailsPage.qml"), pageProps)
                            })
                        }

                        Component {
                            id: updateDivider
                            SectionDivider {
                                text: section == "true" ? i18n.tr("Available updates") : i18n.tr("Installed apps")
                            }
                        }
                    }
                }
            }

            Column {
                id: repoFetchingIndicator
                anchors.centerIn: parent
                anchors.verticalCenterOffset: mainPage.header.height * 0.4
                spacing: units.gu(1)
                ActivityIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    running: visible
                }
                Label {
                    textSize: Label.Small
                    text: i18n.tr("Fetching package list...")
                }
            }
        }
    }

    Component {
        id: searchPageComponent

        SearchPage {
            id: searchPage
            model: appModel

            onAppDetailsRequired: {
                var pageProps = { app: appModel.app(appModel.findApp(appId)) }
                searchPage.pageStack.addPageToNextColumn(searchPage, Qt.resolvedUrl("AppDetailsPage.qml"), pageProps)
            }
        }
    }

    Component {
        id: filteredAppPageComponent
        Page {
            id: filteredAppPage
            property alias filterPattern: filteredAppView.filterPattern
            property alias filterProperty: filteredAppView.filterProperty
            header: PageHeader {
                title: filteredAppPage.title
                automaticHeight: false
            }
            FilteredAppView {
                id: filteredAppView
                model: appModel
                onAppDetailsRequired: {
                    var pageProps = { app: appModel.app(appModel.findApp(appId)) }
                    filteredAppPage.pageStack.addPageToNextColumn(filteredAppPage, Qt.resolvedUrl("AppDetailsPage.qml"), pageProps)
                }
            }
        }
    }


    Component {
        id: warningComponent

        Dialog {
            id: warningDialog
            title: i18n.tr("Warning")
            text: i18n.tr("Open Store allows installing unconfined applications. Please make sure that you know about the implications of that. ") +
                  i18n.tr("An unconfined application has the ability to break the system, reduce its performance and/or spy on you. ") +
                  i18n.tr("While we are doing our best to prevent that by reviewing applications, we don't take any responsibility if something bad slips through. ") +
                  i18n.tr("Use this at your own risk.")

            signal accepted();
            signal rejected();

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
            title: i18n.tr("Install package?")
            text: i18n.tr("Do you want to install %1?").arg(fileName)

            property string fileName
            signal accepted();
            signal rejected();

            Button {
                text: i18n.tr("Yes")
                color: UbuntuColors.green
                onClicked: {
                    installQuestionDialog.accepted();
                    PopupUtils.close(installQuestionDialog)
                }

            }
            Button {
                text: i18n.tr("No")
                color: UbuntuColors.red
                onClicked: {
                    installQuestionDialog.rejected();
                    PopupUtils.close(installQuestionDialog)
                }
            }
        }
    }

    Component {
        id: installedConfirmation
        Dialog {
            id: installedConfirmationDialog
            title: i18n.tr("Package installed")
            text: i18n.tr("The package has been installed successfully.")
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
}

