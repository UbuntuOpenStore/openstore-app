/*
 * Copyright (C) 2020 - Brian Douglass
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

import QtQuick 2.9
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import Ubuntu.Components.Popups 1.3
import Ubuntu.Connectivity 1.0
import QtQuick.Layouts 1.1
import OpenStore 1.0

import "Components" as Components

Page {
    id: appLocalDetailsPage

    property var app: null

    header: Components.HeaderBase {
        title: app ? app.name : i18n.tr("App details")
        enabled: !PlatformIntegration.clickInstaller.busy

        trailingActionBar {
            numberOfSlots: 1
            actions: Action {
                iconName: "delete"
                text: i18n.tr("Remove")

                onTriggered: {
                    // TODO share removeQuestion popup
                    var popup = PopupUtils.open(removeQuestion, root, {pkgName: app.name || app.appId});
                    popup.accepted.connect(function() {
                        PlatformIntegration.clickInstaller.removePackage(app.appId, app.version);
                        bottomEdgeStack.pop();
                    })
                }
            }
        }
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.topMargin: parent.header.height

        // WORKAROUND: Fix for wrong grid unit size
        Component.onCompleted: root.flickable_responsive_scroll_fix(scrollView.flickableItem)

        Column {
            id: mainColumn
            width: scrollView.width

            ListItem {
                height: units.gu(16)

                ListItemLayout {
                    anchors.fill: parent
                    title.text: app.name

                    UbuntuShape {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(12); height: width
                        aspect: UbuntuShape.Flat

                        image: Image {
                            height: parent.height
                            width: parent.width
                            sourceSize.width: parent.width
                            sourceSize.height: parent.height
                            source: app ? app.icon : ""
                        }
                    }
                }
            }

            ListItem {
                height: messageLayout.height
                ListItemLayout {
                    id: messageLayout
                    subtitle.text: {
                        if (Connectivity.online) {
                            return i18n.tr("This app could not found in the OpenStore");
                        }

                        return i18n.tr("You are currently offline and the app details could not be fetched from the OpenStore");
                    }
                    subtitle.maximumLineCount: Number.MAX_VALUE
                    subtitle.wrapMode: Text.WordWrap

                    Icon {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(4); height: width
                        name: "info"
                    }
                }
            }

            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Installed version")
                    subtitle.text: app.version
                }
            }
        }
    }

    Components.UninstallPopup {
        id: removeQuestion
    }
}
