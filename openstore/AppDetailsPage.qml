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

import QtQuick 2.2
import Ubuntu.Components 1.1
import Ubuntu.Components.ListItems 1.0
import QtQuick.Layouts 1.1

Page {
    title: app ? app.name : "App details"

    property var app: null

    Column {
        anchors.fill: parent
        anchors.margins: units.gu(2)
        spacing: units.gu(1)

        RowLayout {
            anchors { left: parent.left; right: parent.right }
            height: units.gu(10)
            spacing: units.gu(1)

            UbuntuShape {
                Layout.fillHeight: true
                Layout.preferredWidth: height

                image: Image {
                    height: parent.height
                    width: parent.width
                    source: app ? app.icon : ""
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: units.gu(1)
                Label {
                    text: app.name
                    Layout.fillWidth: true
                    fontSize: "large"
                }
                Button {
                    Layout.fillWidth: true
                    text: app.installed ? "Upgrade" : "Install"
                    visible: !appModel.installer.busy && (!app.installed || app.installedVersion < app.version)
                    onClicked: {
                        appModel.installer.installPackage(app.packageUrl)
                    }
                }
                ProgressBar {
                    Layout.fillWidth: true
                    maximumValue: app ? app.fileSize : 0
                    value: appModel.installer.downloadProgress
                    visible: appModel.installer.busy
                    indeterminate: appModel.installer.downloadProgress == 0
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: units.gu(1)
                    visible: !appModel.installer.busy && app.installed && app.installedVersion >= app.version
                    Icon {
                        Layout.preferredHeight: units.gu(3)
                        Layout.preferredWidth: units.gu(3)
                        name: "tick"
                        color: UbuntuColors.green
                    }
                    Label {
                        text: "Installed"
                        Layout.fillWidth: true
                    }
                }
            }
        }

        ThinDivider { }

        Label {
            anchors { left: parent.left; right: parent.right }
            text: app.tagline
            wrapMode: Text.WordWrap
        }

        Label {
            anchors { left: parent.left; right: parent.right }
            text: app.description
            wrapMode: Text.WordWrap
        }

        ThinDivider { }

        Label {
            anchors { left: parent.left; right: parent.right }
            text: "Installed version: " + (app.installedVersion ? app.installedVersion : "None")
            wrapMode: Text.WordWrap
        }

        Label {
            anchors { left: parent.left; right: parent.right }
            text: "Latest available version: " + app.version
            wrapMode: Text.WordWrap
        }

        Label {
            anchors { left: parent.left; right: parent.right }
            text: "License: " + app.license
            wrapMode: Text.WordWrap
        }
        Label {
            anchors { left: parent.left; right: parent.right }
            text: "Source code:"
            wrapMode: Text.WordWrap
        }
        AbstractButton {
            anchors { left: parent.left; right: parent.right }
            height: linkLabel.implicitHeight
            Label {
                id: linkLabel
                anchors { left: parent.left; right: parent.right }
                text: app.source
                wrapMode: Text.WordWrap
                color: "blue"
            }
            onClicked: {
                Qt.openUrlExternally(app.source)
            }
        }

    }
}
