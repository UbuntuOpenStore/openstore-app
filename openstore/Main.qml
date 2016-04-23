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
                pageStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), {app: appModel.app(index)})
            }
        }
    }

    PageStack {
        id: pageStack

        Component.onCompleted: {
            pageStack.push(mainPage)
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
    }

    ServiceRegistry {
        id: serviceRegistry
        clickInstaller: installer
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
    Page {
        id: mainPage
        title: i18n.tr("Open Store")

        ListView {
            anchors.fill: parent
            model: appModel

            onCountChanged: {
                if (count > 0 && root.appIdToOpen != "") {
                    var index = appModel.findApp(root.appIdToOpen)
                    if (index >= 0) {
                        pageStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), {app: appModel.app(index)})
                        root.appIdToOpen = "";
                    }
                }
            }

            delegate: ListItem {
                height: units.gu(10)
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: units.gu(1)
                    spacing: units.gu(1)
                    UbuntuShape {
                        Layout.fillHeight: true
                        Layout.preferredWidth: height
                        image: Image {
                            source: model.icon
                            height: parent.height
                            width: parent.width
                        }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: units.gu(1)
                        RowLayout {
                            Layout.fillWidth: true
                            Icon {
                                Layout.preferredHeight: units.gu(3)
                                Layout.preferredWidth: height
                                implicitHeight: height
                                implicitWidth: width
                                visible: model.installed
                                name: "tick"
                                color: model.installedVersion >= model.version ? UbuntuColors.green : UbuntuColors.orange
                            }

                            Label {
                                Layout.fillWidth: true
                                text: model.name
                                fontSize: "large"
                                elide: Text.ElideRight
                            }
                        }
                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: model.tagline
                            fontSize: "medium"
                            wrapMode: Text.WordWrap
                        }
                    }
                }
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), {app: appModel.app(index)})
                }
            }
        }
    }

    Component {
        id: warningComponent

        Dialog {
            id: warningDialog
            title: "Warning"
            text: "Open Store allows installing unconfined applications. Please make sure that you know about the implications of that. " +
                  "An unconfined application has the ability to break the system, reduce its performance and/or spy on you. " +
                  "While we are doing our best to prevent that by reviewing applications, we don't take any responsibility if something bad slips through. " +
                  "Use this at your own risk."

            signal accepted();
            signal rejected();

            Button {
                text: "Okay. Got it! I'll be careful."
                color: UbuntuColors.green
                onClicked: {
                    warningDialog.accepted();
                }
            }
            Button {
                text: "Get me out of here!"
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
            title: "Install package?"
            text: i18n.tr("Do you want to install %1?").arg(fileName)

            property string fileName
            signal accepted();
            signal rejected();

            Button {
                text: "Yes"
                color: UbuntuColors.green
                onClicked: {
                    installQuestionDialog.accepted();
                    PopupUtils.close(installQuestionDialog)
                }

            }
            Button {
                text: "No"
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
            title: "Package installed"
            text: "The package has been installed successfully."
            Button {
                color: UbuntuColors.blue
                text: "OK"
                onClicked: PopupUtils.close(installedConfirmationDialog)
            }
        }
    }
    Component {
        id: installationError
        Dialog {
            id: installationErrorDialog
            title: "Installation failed"
            text: "The package could not be installed. Make sure it is a valid click package."
            Button {
                color: UbuntuColors.orange
                text: "OK"
                onClicked: PopupUtils.close(installationErrorDialog)
            }
        }
    }
}

