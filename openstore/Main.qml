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
import QtQuick 2.0
import Ubuntu.Components 1.1
import Ubuntu.Components.Popups 1.0
import Ubuntu.Components.ListItems 1.0
import OpenStore 1.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0

MainView {
    applicationName: "openstore.mzanetti"

    useDeprecatedToolbar: false

    width: units.gu(100)
    height: units.gu(75)

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
    }

    PageStack {
        id: pageStack

        Component.onCompleted: pageStack.push(mainPage)
    }

    Settings {
        id: settings
        property bool firstStart: true
    }

    AppModel {
        id: appModel
    }

    Page {
        id: mainPage
        title: i18n.tr("Open App Store")

        ListView {
            anchors.fill: parent
            model: appModel

            delegate: Empty {
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
            text: "Open App Store allows installing unconfined applications. Please make sure that you know about the implications of that. " +
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
}

