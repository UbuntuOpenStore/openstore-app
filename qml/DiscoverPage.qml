/*
 * Copyright (C) 2017 - Stefano Verzegnassi <verzegnassi.stefano@gmail.com>
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
 */

import QtQuick 2.4
import Lomiri.Components 1.3
import OpenStore 1.0
import Lomiri.Connectivity 1.0

import "Components" as Components

Page {
    id: discoverPage
    objectName: "discoverPage"

    property DiscoverModel discoverModel: root.discoverModel
    anchors.fill: parent

    function slot_packageDetailsReady(pkg) {
        PackagesCache.packageDetailsReady.disconnect(slot_packageDetailsReady)
        bottomEdgeStack.clear()
        bottomEdgeStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), { app: pkg })
    }

    header: Components.HeaderMain {
        id: mainHeader
        title: i18n.tr('OpenStore')
        flickable: view
    }

    ScrollView {
        anchors.fill: parent

        ListView {
            id: view
            anchors.fill: parent

            // WORKAROUND: Fix for wrong grid unit size
            Component.onCompleted: root.flickable_responsive_scroll_fix(view)

            header: Column {
                width: parent.width

                Components.HighlightedApp {
                    id: highlightAppControl

                    property var appItem: discoverModel.getPackage(discoverModel.highlightAppId)

                    // Hide when discover is loaded, but there is no highlighted app
                    visible: !discoverModel.ready || (discoverModel.ready && highlightAppControl.appItem !== null)

                    appStatus: {
                        //TODO: More elegant way of doing this?
                        if (highlightAppControl.appItem && highlightAppControl.appItem.installed)
                            return highlightAppControl.appItem.updateAvailable ? 1 //Update available
                                                                               : 2 //Installed

                        return 0 //Not installed / not available
                    }

                    imageUrl: discoverModel.highlightBannerUrl || highlightAppControl.appItem.icon
                    appName: highlightAppControl.appItem ? highlightAppControl.appItem.name : ''
                    appPublisher: highlightAppControl.appItem ? i18n.tr("by %1").arg(highlightAppControl.appItem.publisher) : ''
                    appDesc: highlightAppControl.appItem ? (highlightAppControl.appItem.tagline + " " + highlightAppControl.appItem.description) : ''
                    appRatings: highlightAppControl.appItem ? highlightAppControl.appItem.ratings : ''

                    ActivityIndicator {
                        anchors.centerIn: parent
                        //visible: !highlightAppControl.ready && Connectivity.online
                        visible: {
                            if (!Connectivity.online) {
                                return false;
                            }
                            if (highlightAppControl.ready) {
                                return false;
                            }

                            // Discover was loaded, but there is no highlighted app
                            if (discoverModel.ready && !highlightAppControl.appItem) {
                                return false;
                            }

                            return true;
                        }
                        running: visible
                    }

                    onButtonClicked: {
                        //If we hide the whole Component we don't need to check if it's ready
                        if (appStatus == 2) {
                            Qt.openUrlExternally(highlightAppControl.appItem.appLaunchUrl());
                        } else {
                            PackagesCache.packageDetailsReady.connect(slot_packageDetailsReady)
                            PackagesCache.getPackageDetails(highlightAppControl.appItem.appId)
                        }
                    }

                    Item {
                        visible: !Connectivity.online && !highlightAppControl.ready
                        anchors.fill: parent

                        Icon {
                            id: offlineIcon
                            name: 'wifi-none'
                            anchors.centerIn: parent
                            width: units.gu(4)
                            height: width
                        }

                        Label {
                            anchors {
                                horizontalCenter: offlineIcon.horizontalCenter
                                top: offlineIcon.bottom
                                topMargin: units.gu(1)
                            }

                            text: i18n.tr("You are currently offline")
                        }
                    }
                }

                ListItem {
                    id: appStoreUpdateAlert
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    divider.visible: false
                    enabled: visible    // Just for being sure everything works as expected
                    visible: localAppModel.appStoreUpdateAvailable
                    color: theme.palette.normal.activity

                    ListItemLayout {
                        anchors.fill: parent
                        title.text: i18n.tr("OpenStore update available")
                        title.color: theme.palette.normal.activityText
                        ProgressionSlot {}
                    }

                    onClicked: {
                        PackagesCache.packageDetailsReady.connect(slot_packageDetailsReady)
                        PackagesCache.getPackageDetails(localAppModel.appStoreAppId)
                    }

                    // WORKAROUND: appStoreUpdateAlert visibility is toggled after the whole page is layouted.
                    // This may result in the "Discover" tab being slightly scrolled down at start-up.
            /*        Connections {
                        target: appStoreUpdateAlert
                        onVisibleChanged: {
                            view.positionViewAtBeginning()
                        }
                    }
            */
                }

                ListItem {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    //Used opacity to hide this until the model is ready. If visible is used, the ListView moves up on start
                    opacity: highlightAppControl.ready || (discoverModel.ready && highlightAppControl.appItem === null) || !Connectivity.online ? 1 : 0

                    ListItemLayout {
                        anchors.fill: parent
                        title.text: localAppModel.updatesAvailableCount > 0
                            ? i18n.tr("Installed Apps") + i18n.tr(" (%1 update available)", " (%1 updates available)", localAppModel.updatesAvailableCount).arg(localAppModel.updatesAvailableCount)
                            : i18n.tr("Installed Apps")
                        title.color: theme.palette.normal.backgroundText
                        ProgressionSlot {}

                        Icon {
                            name: "ubuntu-store-symbolic"
                            SlotsLayout.position: SlotsLayout.Leading;
                            width: units.gu(3)
                            height: width
                        }
                    }

                    onClicked: pageStack.push(Qt.resolvedUrl("InstalledAppsPage.qml"), {})
                }

                ListItem {
                    id: footerItem
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    //Used opacity to hide this until the model is ready. If visible is used, the ListView moves up on start
                    opacity: highlightAppControl.ready || (discoverModel.ready && highlightAppControl.appItem === null) ? 1 : 0

                    ListItemLayout {
                        anchors.fill: parent
                        title.text: i18n.tr("Browse Apps by Category")
                        title.color: theme.palette.normal.backgroundText
                        ProgressionSlot {}

                        Icon {
                            name: "view-list-symbolic"
                            SlotsLayout.position: SlotsLayout.Leading;
                            width: units.gu(3)
                            height: width
                        }
                    }

                    onClicked: pageStack.push(Qt.resolvedUrl("CategoriesPage.qml"))
                }
            }

            model: discoverModel
            delegate: Components.OpenStoreCarousel {
                width: parent.width

                title: model.name
                subtitle: model.tagline
                showProgression: model.queryUrl

                onTitleClicked: if (model.queryUrl) { root.showSearchQuery(model.queryUrl) }
                onAppTileClicked: {
                    PackagesCache.packageDetailsReady.connect(slot_packageDetailsReady)
                    PackagesCache.getPackageDetails(appItem.appId)
                }

                viewModel: model.appIds
                function packageInfoGetter(i) {
                    return discoverModel.getPackage(i)
                }
            }

            //If we didn't get the app list on start, let's get it when we reconnect
            Connections {
                target: OpenStoreNetworkManager

                onNetworkAccessibleChanged: {
                    if (OpenStoreNetworkManager.networkAccessible && discoverModel.rowCount() == 0) {
                        console.log("Reconnected and App Model is empty. Try populate model again")
                        discoverModel.refresh();
                    }
                }
            }
        }
    }
}
