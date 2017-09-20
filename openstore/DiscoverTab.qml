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

import "Components" as Components

Page {
    id: rootItem

    property DiscoverModel discoverModel: root.discoverModel

    header: PageHeader {
        title: i18n.tr("Discover")
        contents: Rectangle {
            id: searchField
            anchors.centerIn: parent
            width: Math.min(parent.width, units.gu(36))
            implicitHeight: units.gu(4)

            Row {
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    margins: units.gu(1)
                }

                spacing: units.gu(1)
                Icon {
                    width: units.gu(2); height: width
                    name: "find"
                }

                Label {
                    text: i18n.tr("Search in OpenStore...")
                }
            }

            radius: units.dp(8)
            color: "transparent"
            border.width: units.dp(1)
            border.color: "#cdcdcd"

            MouseArea {
                anchors.fill: parent
                onClicked: mainPage.showSearch()
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.topMargin: rootItem.header.height

        ListView {
            id: view
            anchors.fill: parent

            // WORKAROUND: Fix for wrong grid unit size
            Component.onCompleted: root.flickable_responsive_scroll_fix(view)

            header: Column {
                width: parent.width

                AbstractButton {
                    id: highlightAppControl

                    property var appItem: discoverModel.getPackage(discoverModel.highlightAppId)

                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.min(parent.width, units.gu(80))
                    height: width * 0.5

                    onClicked: {
                        var pageProps = { app: highlightAppControl.appItem }
                        bottomEdgeStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), pageProps)
                    }

                    Image {
                        anchors.fill: parent
                        anchors.bottomMargin: units.gu(2)
                        source: discoverModel.highlightBannerUrl || highlightAppControl.appItem.icon
                        fillMode: Image.PreserveAspectCrop
                    }

                    Rectangle {
                        anchors.fill: highlightAppLabels
                        color: "black"
                        opacity: 0.45
                    }

                    ListItemLayout {
                        id: highlightAppLabels
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: units.gu(2)

                        title.text: highlightAppControl.appItem.name
                        title.textSize: Label.Large
                        title.font.weight: Font.Normal
                        title.color: "white"

                        subtitle.text: highlightAppControl.appItem.tagline || highlightAppControl.appItem.description
                        subtitle.textSize: Label.Small
                        subtitle.color: "white"

                        summary.text: {
                            if (highlightAppControl.appItem.installed)
                                return highlightAppControl.appItem.updateAvailable ? i18n.tr("Update available").toUpperCase()
                                                                                   : i18n.tr("✓ Installed").toUpperCase()

                            return ""
                        }
                        summary.textSize: Label.XSmall
                        summary.color: "white"
                    }
                }

                ListItem {
                    id: appStoreUpdateWarning
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.min(parent.width, units.gu(80))
                    divider.visible: false
                    enabled: visible    // Just for being sure everything works as expected
                    visible: appModel.appStoreUpdateAvailable

                    ListItemLayout {
                        anchors.fill: parent
                        title.text: i18n.tr("OpenStore update available")
                        subtitle.text: i18n.tr("Update OpenStore now!")
                        subtitle.wrapMode: Text.WordWrap

                        Button {
                            SlotsLayout.position: SlotsLayout.Last
                            color: UbuntuColors.green
                            text: i18n.tr("Details")

                            function slot_installedPackageDetailsReady(pkg) {
                                appModel.packageDetailsReady.disconnect(slot_installedPackageDetailsReady)
                                bottomEdgeStack.clear()
                                bottomEdgeStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), { app: pkg })
                            }

                            onClicked: {
                                appModel.packageDetailsReady.connect(slot_installedPackageDetailsReady)
                                appModel.showPackageDetails(appModel.appStoreAppId)
                            }
                        }
                    }
                }
            }

            model: discoverModel
            delegate: Components.OpenStoreCarousel {
                width: parent.width

                title: model.name
                subtitle: model.tagline
                showProgression: model.queryUrl
                onTitleClicked: if (model.queryUrl) { mainPage.showSearchQuery(model.queryUrl) }
                onAppTileClicked: bottomEdgeStack.push(Qt.resolvedUrl("../AppDetailsPage.qml"), { app: appItem })

                viewModel: model.appIds
                function packageInfoGetter(i) {
                    return discoverModel.getPackage(i)
                }
            }
        }

    }
}
