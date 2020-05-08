/*
 * Copyright (C) 2017 Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
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
import Morph.Web 0.1

import "Components" as Components

Page {
    id: rootItem

    header: Components.HeaderBase {
        title: i18n.tr("Settings")
    }

    AuthenticationHandler {
        id: authHandler
        serviceName: root.applicationName

        onAuthenticationSucceeded: {
            settings.hideNsfw = nsfwSwitch.checked
        }

        onAuthenticationAborted: {
            nsfwSwitch.checked = !nsfwSwitch.checked
        }
    }

    Column {
        id: secondaryCol
        width: units.gu(50)
        visible: isBigScreen

        anchors {
            top: parent.top
            topMargin: parent.header.height + units.gu(2)
            right: parent.right
        }

        Loader {
            active: isBigScreen
            visible: active
            width: parent.width
            sourceComponent: textAboutComponent
        }
    }

    ScrollView {
        id: scrollView
        height: parent.height
        width: isBigScreen
            ? parent.width - secondaryCol.width
            : parent.width

        anchors {
            top: parent.top
            topMargin: parent.header.height
            left: parent.left
        }

        Item {
            width: scrollView.width
            height: layout.height + units.gu(8)

            Column {
                id: layout
                width: parent.width

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: parent.top
                    topMargin: units.gu(2)
                }

                Column {
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: units.gu(18)

                    UbuntuShape {
                        width: units.gu(16)
                        height: width
                        anchors.horizontalCenter: parent.horizontalCenter
                        radius: "medium"

                        source: Image {
                            source: Qt.resolvedUrl("openstore.svg")
                        }
                    }

                    /* TODO: This need to be exported in CMake or ¿pro? files
                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        //TRANSLATORS: This is the name of the app (%1) + version number (v%2)
                        text: i18n.tr("%1 v%2").arg("OpenStore").arg(Qt.application.version)
                    }
                    */
                }

                Components.SectionDivider {
                    text: i18n.tr("OpenStore Account")
                }

                ListItem {
                    visible: root.apiKey === ""
                    onClicked: bottomEdgeStack.push(Qt.resolvedUrl("SignInWebView.qml"))
                    ListItemLayout {
                        anchors.centerIn: parent
                        title.text: i18n.tr("Sign in")

                        Icon {
                            name: "account"
                            width: units.gu(2)
                            height: width
                        }

                        ProgressionSlot {}
                    }
                }

                ListItem {
                    visible: root.apiKey !== ""
                    onClicked: root.apiKey = ""
                    ListItemLayout {
                        anchors.centerIn: parent
                        title.text: i18n.tr("Sign out")

                        Icon {
                            name: "edit-clear"
                            width: units.gu(2)
                            height: width
                        }

                        ProgressionSlot {}
                    }
                }

                /*
                Components.SectionDivider {
                    text: i18n.tr("Development")
                }

                ListItem {
                    onClicked: Qt.openUrlExternally("https://open-store.io/manage")
                    ListItemLayout {
                        anchors.centerIn: parent
                        title.text: i18n.tr("Manage your apps on OpenStore")
                        ProgressionSlot {}
                    }
                }
                */

                Components.SectionDivider {
                    text: i18n.tr("Parental control")
                }

                ListItem {
                    ListItemLayout {
                        anchors.centerIn: parent
                        title.text: i18n.tr("Hide adult-oriented content")

                        Switch {
                            id: nsfwSwitch
                            SlotsLayout.position: SlotsLayout.Last
                            checked: settings.hideNsfw

                            onClicked: {
                                if (!checked) {
                                    // Ask authentication only if user is trying to enable NSFW
                                    authHandler.authenticate(i18n.tr("By typing your password you take full responsibility for showing NSFW content."))
                                } else {
                                    settings.hideNsfw = checked
                                }
                            }
                        }
                    }
                }

                Loader {
                    id: textAbout
                    active: !isBigScreen
                    visible: active
                    width: parent.width
                    sourceComponent: textAboutComponent
                }
            }
        }
    } //ScrollView

    Component {
        id: textAboutComponent

        Column {
            Components.SectionDivider {
                text: i18n.tr("About OpenStore")
            }

            ListItem {
                onClicked: Qt.openUrlExternally("https://gitlab.com/theopenstore/openstore-app")
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Source Code")
                    ProgressionSlot {}
                }
            }

            ListItem {
                onClicked: Qt.openUrlExternally("https://gitlab.com/theopenstore/openstore-meta/issues/new")
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Report an issue")
                    ProgressionSlot {}
                }
            }
        }
    }
}
