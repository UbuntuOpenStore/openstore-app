/*
 * Copyright (C) 2017 - Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
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
import OpenStore 1.0

ScrollView {
    id: rootItem
    anchors.fill: parent
    anchors.topMargin: parent.header ? parent.header.height : 0

    property var discoverData
    property string discoverApiEndPoint: "https://open.uappexplorer.com/api/v1/apps/discover"
    property AppModel storeModel

    signal appDetailsRequired(var appId)
    signal categoryViewRequired(var name, var categoryCode)

    Component.onCompleted: {
        var doc = new XMLHttpRequest();
        doc.onreadystatechange = function() {
            if (doc.readyState == 4 && doc.status == 200) {
                var reply = JSON.parse(doc.responseText)

                if (reply.success) {
                    rootItem.discoverData = reply.data
                } else {
                    console.log("Unable to fetch discover data from server (success = false).")
                }
            }
        }

        doc.open("GET", discoverApiEndPoint, true);
        doc.send();
    }

    ListView {
        id: view
        anchors.fill: parent

        header: AbstractButton {
            id: highlightAppControl
            property var appItem: storeModel.app(storeModel.findApp(discoverData.highlighted_id))
            width: parent.width
            height: Math.min(units.gu(28), width * 9 / 16)

            onClicked: rootItem.appDetailsRequired(discoverData.highlighted_id)

            Image {
                anchors.fill: parent
                anchors.bottomMargin: units.gu(4)
                source: highlightAppControl.appItem.icon
                fillMode: Image.PreserveAspectCrop
            }

            ListItemLayout {
                anchors.centerIn: parent

                title.text: highlightAppControl.appItem.name
                title.font.pixelSize: units.gu(3)
                title.color: "white"

                subtitle.text: highlightAppControl.appItem.tagline || highlightAppControl.appItem.description
                subtitle.font.pixelSize: units.gu(1.5)
                subtitle.color: "white"
            }
        }

        model: discoverData ? rootItem.discoverData.categories : null
        delegate: Column {
            width: parent.width
            spacing: units.gu(1)

            ListItem {
                divider.visible: false
                onClicked: {
                    if (modelData.referral) {
                        rootItem.categoryViewRequired(modelData.name, modelData.referral)
                    }
                }

                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: modelData.name
                    subtitle.text: modelData.tagline

                    ProgressionSlot {
                        visible: modelData.referral != ""
                    }
                }
            }

            ListView {
                anchors { left: parent.left; right: parent.right }
                leftMargin: units.gu(2)
                rightMargin: units.gu(2)
                clip: true
                height: count > 0 ? units.gu(24) : 0
                visible: count > 0
                spacing: units.gu(2)
                orientation: ListView.Horizontal
                model: modelData.ids
                delegate: AbstractButton {
                    id: appDel
                    property var appItem: storeModel.app(storeModel.findApp(modelData))
                    height: parent.height
                    width: units.gu(16)

                    onClicked: rootItem.appDetailsRequired(modelData)

                    Column {
                        anchors.fill: parent

                        UbuntuShape {
                            width: parent.width
                            height: width
                            aspect: UbuntuShape.Flat
                            sourceFillMode: UbuntuShape.PreserveAspectFit
                            source: Image {
                                source: appDel.appItem.icon
                            }
                        }

                        ListItemLayout {
                            anchors {
                                left: parent.left; leftMargin: units.gu(-1)
                                right: parent.right
                            }

                            height: units.gu(4)
                            title {
                                text: appDel.appItem.name
                                textSize: Label.Small
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                maximumLineCount: 2
                            }

                            subtitle {
                                text: appDel.appItem.author
                                textSize: Label.XSmall
                            }

                            summary.text: appDel.appItem.installed ? appDel.appItem.updateAvailable ? i18n.tr("Update available") : i18n.tr("Installed") : ""
                            summary.textSize: Label.XSmall
                        }
                    }
                }
            }
        }
    }
}
