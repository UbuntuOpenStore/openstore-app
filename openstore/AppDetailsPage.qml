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
import Ubuntu.Components.ListItems 1.3
import QtQuick.Layouts 1.1
import OpenStore 1.0


Page {
    header: PageHeader {
        title: app ? app.name : "App details"
    }

    property var app: null


    Flickable {
        anchors.fill: parent
        anchors.topMargin: parent.header.height
        contentHeight: mainColumn.height + units.gu(2)
        interactive: contentHeight > height - topMargin

        Column {
            id: mainColumn
            anchors { left: parent.left; top: parent.top; right: parent.right }
            anchors.margins: units.gu(2)
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
                    Label {
                        text: app.author
                        Layout.fillWidth: true
                    }
                }
            }
            ThinDivider { }
            Label {
                anchors { left: parent.left; right: parent.right }
                text: app.tagline
                wrapMode: Text.WordWrap
            }

            ListView {
                anchors { left: parent.left; right: parent.right }
                height: count > 0 ? units.gu(20) : 0
                visible: count > 0
                spacing: units.gu(1)
                orientation: ListView.Horizontal
                model: app.screenshots
                delegate: UbuntuShape {
                    height: parent.height
                    // sh : lv.h = sw : x
                    width: screenshot.sourceSize.width * height / screenshot.sourceSize.height
                    sourceFillMode: UbuntuShape.PreserveAspectFit
                    source: Image {
                        id: screenshot
                        source: modelData
                    }

                    AbstractButton {
                        id: screenShotButton
                        anchors.fill: parent
                        onClicked: {
                            print("opening at:", screenShotButton.mapToItem(root, 0, 0).x)
                            zoomIn.createObject(root, {x: screenShotButton.mapToItem(root, 0, 0).x, y: screenShotButton.mapToItem(root, 0, 0).y, itemScale: screenShotButton.height / root.height, imageSource: modelData});
//                            zoomIn.createObject(root, {x: 100, y: 100});
                        }
                    }

                    Component {
                        id: zoomIn
                        Rectangle {
                            id: zI
                            width: parent.width
                            height: parent.height
                            color: "black"

                            property real itemScale: 1
                            property string imageSource
                            transform: Scale {
                                origin.x: 0
                                origin.y: 0
                                xScale: zI.itemScale
                                yScale: zI.itemScale
                            }

                            ParallelAnimation {
                                id: scaleInAnimation
                                onStarted: {
                                    hideAnimation.initialScale = itemScale;
                                    hideAnimation.initialX = x;
                                    hideAnimation.initialY = y;
                                }

                                UbuntuNumberAnimation { target: zI; property: "itemScale"; to: 1 }
                                UbuntuNumberAnimation { target: zI; properties: "x,y"; to: 0 }
                            }


                            Component.onCompleted: {
                                scaleInAnimation.start();
                            }

                            Image {
                                anchors.fill: parent
                                source: zI.imageSource
                                fillMode: Image.PreserveAspectFit
                            }

                            AbstractButton {
                                anchors.fill: parent
                                onClicked: {
                                    hideAnimation.start()
                                }
                            }

                            ParallelAnimation {
                                id: hideAnimation
                                property real initialScale: 1
                                property int initialX: 0
                                property int initialY: 0


                                UbuntuNumberAnimation { target: zI; property: "itemScale"; to: hideAnimation.initialScale }
                                UbuntuNumberAnimation { target: zI; property: "x"; to: hideAnimation.initialX }
                                UbuntuNumberAnimation { target: zI; property: "y"; to: hideAnimation.initialY }
                                onStopped: {
                                    script: zI.destroy()
                                }

                            }
                        }
                    }
                }
            }

            Label {
                anchors { left: parent.left; right: parent.right }
                text: app.description
                wrapMode: Text.WordWrap
            }

            ThinDivider { }

            Label {
                text: "<b>Packager/Publisher:</b> " + (app.maintainer ? app.maintainer : "Openstore team")
            }

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
                text: "Changelog:"
                wrapMode: Text.WordWrap
                visible: app.changelog
            }

            Label {
                anchors { left: parent.left; right: parent.right }
                text: app.changelog
                wrapMode: Text.WordWrap
                visible: app.changelog
            }

            ThinDivider {}

            ProgressBar {
                Layout.fillWidth: true
                maximumValue: app ? app.fileSize : 0
                value: appModel.installer.downloadProgress
                visible: appModel.installer.busy
                indeterminate: appModel.installer.downloadProgress == 0
            }

            RowLayout {
                width: parent.width
                spacing: units.gu(1)
                visible: !appModel.installer.busy

                Button {
                    text: app.installed ? "Upgrade" : "Install"
                    visible: !app.installed || (app.installed && app.installedVersion < app.version)
                    color: "#DD4814"
                    onClicked: {
                        appModel.installer.installPackage(app.packageUrl)
                    }
                }

                Button {
                    text: "Remove"
                    visible: app.installed
                    color: UbuntuColors.red
                    onClicked: {
                        appModel.installer.removePackage(app.appId, app.version)
                    }
                }
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
                            Layout.preferredHeight: units.gu(3)
                            Layout.preferredWidth: units.gu(3)
                            implicitHeight: height
                            implicitWidth: width
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
                            Layout.preferredHeight: units.gu(3)
                            Layout.preferredWidth: units.gu(3)
                            implicitHeight: height
                            implicitWidth: width
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
                            Layout.preferredHeight: units.gu(3)
                            Layout.preferredWidth: units.gu(3)
                            implicitHeight: height
                            implicitWidth: width
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
                            Layout.preferredHeight: units.gu(3)
                            Layout.preferredWidth: units.gu(3)
                            implicitHeight: height
                            implicitWidth: width
                            name: "security-alert"
                        }

                        Label {
                            text: "Write paths: " + writepaths
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Button {
                        anchors { left: parent.left }
                        text: "Open"
                        color: UbuntuColors.green
                        visible: app.installed &&  (hooks & ApplicationItem.HookDesktop)
                        onClicked: Qt.openUrlExternally("appid://" + app.appId + "/" + hookName + "/" + app.installedVersion)
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
