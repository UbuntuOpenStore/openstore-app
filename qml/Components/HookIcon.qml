/*
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
 *
 */

import QtQuick 2.4
import QtQuick.Layouts 1.1
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3

Icon {
    id: root
    implicitHeight: height
    implicitWidth: width
    AbstractButton {
        anchors.fill: parent
        onClicked: {
            PopupUtils.open(popover, parent)
        }
    }

    Component {
        id: popover
        Popover {
            Item {
                height: contentColumn.implicitHeight + units.gu(2)
                width: parent.width
                Column {
                    id: contentColumn
                    anchors { left: parent.left; top: parent.top; right: parent.right }
                    anchors.margins: units.gu(1)
                    spacing: units.gu(1)
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "stock_application"
                        }
                        Label {
                            text: i18n.tr("Application")
                            Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "stock_website"
                        }
                        Label {
                            // TRANSLATORS: This is a platform service for launching other applications (ref. https://developer.ubuntu.com/en/phone/platform/guides/url-dispatcher-guide/ )
                            text: i18n.tr("URL Handler")
                            Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "share"
                        }
                        Label {
                            // TRANSLATORS: This is a platform service for content exchange (ref. https://developer.ubuntu.com/en/phone/platform/guides/content-hub-guide/ )
                            text: i18n.tr("Content Hub Handler")
                            Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "notification"
                        }
                        Label {
                            // TRANSLATORS: This is a platform service for push notifications (ref. https://developer.ubuntu.com/en/phone/platform/guides/push-notifications-client-guide/ )
                            text: i18n.tr("Push Helper")
                            Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "contact-group"
                        }
                        Label {
                            // TRANSLATORS: i.e. Online Accounts (ref. https://developer.ubuntu.com/en/phone/platform/guides/online-accounts-developer-guide/ )
                            text: i18n.tr("Accounts provider")
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
