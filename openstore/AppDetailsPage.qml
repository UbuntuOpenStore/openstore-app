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
import OpenStore 1.0


Page {
    title: app ? app.name : "App details"

    property var app: null


    Flickable {
        anchors.fill: parent
        contentHeight: mainColumn.height + units.gu(2)
        interactive: contentHeight > height - topMargin

        Column {
            id: mainColumn
            anchors { left: parent.left; top: parent.top; right: parent.right }
            anchors.margins: units.gu(1)
            spacing: units.gu(1)
            height: childrenRect.height

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
                        text: "Install"
                        visible: !appModel.installer.busy && !app.installed
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
                        visible: !appModel.installer.busy && app.installed
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
                        Button {
                            Layout.fillWidth: true
                            text: "Upgrade"
                            visible: app.installed && app.installedVersion < app.version
                            onClicked: {
                                appModel.installer.installPackage(app.packageUrl)
                            }
                        }
                        Button {
                            Layout.fillWidth: true
                            text: "Remove"
                            visible: app.installed
                            onClicked: {
                                appModel.installer.removePackage(app.appId, app.version)
                            }
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
            ThinDivider {}

            Label {
                anchors { left: parent.left; right: parent.right }
                text: "Package contents:"
                font.bold: true
            }

            Repeater {
                model: app.hooksCount

                delegate: Column {
                    width: parent.width
                    property var hooks: app.hooks(index)
                    property string permissions: app.permissions(index)
                    property string readpaths: app.readPaths(index)
                    property string writepaths: app.writePaths(index)
                    property string hookName: app.hookName(index)
                    property string apparmorTemplate: app.apparmorTemplate(index)
                    spacing: units.gu(1)

                    RowLayout {
                        width: parent.width

                        Label {
                            text: hookName
                            Layout.fillWidth: true
                            font.bold: true
                        }
                        HookIcon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "stock_application"
                            visible: (hooks & ApplicationItem.HookDesktop)
                        }
                        HookIcon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "search"
                            visible: (hooks & ApplicationItem.HookScope)
                        }
                        HookIcon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "stock_website"
                            visible: (hooks & ApplicationItem.HookUrls)
                        }
                        HookIcon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "share"
                            visible: (hooks & ApplicationItem.HookContentHub)
                        }
                        HookIcon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "notification"
                            visible: (hooks & ApplicationItem.HookPushHelper)
                        }
                        HookIcon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "contact-group"
                            visible: (hooks & ApplicationItem.HookAccountService)
                        }
                    }
                    RowLayout {
                        anchors { left: parent.left; right: parent.right }
                        spacing: units.gu(1)
                        Icon {
                            height: units.gu(3)
                            width: units.gu(3)
                            name: "security-alert"
                            visible: apparmorTemplate.indexOf("unconfined") >= 0
                        }

                        Label {
                            id: templateLabel
                            Layout.fillWidth: true
                            text: "Apparmor profile: " + apparmorTemplate
                            visible: apparmorTemplate
                            color: apparmorTemplate.indexOf("unconfined") >= 0 ? UbuntuColors.red : permissionLabel.color
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }


                    Row {
                        anchors { left: parent.left; right: parent.right }
                        spacing: units.gu(1)
                        visible: permissions.length > 0

                        Icon {
                            height: units.gu(3)
                            width: units.gu(3)
                            name: "security-alert"
                        }

                        Label {
                            id: permissionLabel
                            text: "Permissions: " + (permissions ? permissions : "<i>none</i>")
                            width: parent.width
                            wrapMode: Text.WordWrap
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    RowLayout {
                        anchors { left: parent.left; right: parent.right }
                        spacing: units.gu(1)
                        visible: readpaths.length > 0
                        Icon {
                            height: units.gu(3)
                            width: units.gu(3)
                            name: "security-alert"
                        }
                        Label {
                            text: "Read paths: " + readpaths
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    RowLayout {
                        anchors { left: parent.left; right: parent.right }
                        spacing: units.gu(1)
                        visible: writepaths.length > 0
                        Icon {
                            height: units.gu(3)
                            width: units.gu(3)
                            name: "security-alert"
                        }

                        Label {
                            text: "Write paths: " + writepaths
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
            ThinDivider { }

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
}
