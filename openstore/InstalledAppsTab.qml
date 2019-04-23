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

    header: PageHeader {
        title: i18n.tr("My Apps")

        trailingActionBar {
            actions: Action {
                iconName: "settings"
                text: i18n.tr("Settings")
                onTriggered: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
        }
    }

    ScrollView {
        id: scrollView
        width: Math.min(parent.width, units.gu(80))
        anchors {
            top: parent.top
            topMargin: parent.header ? parent.header.height : 0
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }

        ListView {
            id: view
            anchors.fill: parent

            topMargin: scrollView.width == units.gu(80) ? units.gu(4) : 0
            bottomMargin: scrollView.width == units.gu(80) ? units.gu(4) : 0

            model: SortFilterModel {
                id: sortedModel
                sort.property: 'updateStatus'
                model: appModel
            }

            // WORKAROUND: Fix for wrong grid unit size
            Component.onCompleted: root.flickable_responsive_scroll_fix(view)

            section.property: 'updateStatus'
            section.delegate: Components.SectionDivider {
                height: (section == 'downgrade') ? units.gu(9) : units.gu(6)

                text: {
                    if (section == 'none') {
                        // TRANSLATORS: %1 is the number of installed apps
                        return i18n.tr("Installed apps (%1)").arg(sortedModel.count - appModel.updatesAvailableCount - appModel.downgradesAvailableCount);
                    }
                    else if (section == 'available') {
                        // TRANSLATORS: %1 is the number of available app updates
                        return i18n.tr("Available updates (%1)").arg(appModel.updatesAvailableCount);
                    }

                    // TRANSLATORS: %1 is the number of apps that can be downgraded
                    return i18n.tr('Stable version available (%1)'.arg(appModel.downgradesAvailableCount));
                }
                subtext: (section == 'downgrade') ? i18n.tr('The installed versions of these apps did not come from the OpenStore but a stable version is available.') : '';

                buttonText: (section == 'available') ? i18n.tr('Update all') : ''
                onButtonClicked: {
                    for (var i = 0; i < appModel.count; i++) {
                        var app = appModel.get(i);

                        if (app && app.updateStatus == 'available') {
                            // TODO get the app details and install the update
                            console.log(app.appId);
                        }
                    }
                }
            }

            delegate: ListItem {
                height: units.gu(6)
                divider.anchors.leftMargin: units.gu(8)

                ListItemLayout {
                    id: layout
                    anchors.centerIn: parent
                    title.text: model.name

                    UbuntuShape {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(4); height: width
                        aspect: UbuntuShape.Flat
                        image: Image {
							sourceSize.width: parent.width
                            sourceSize.height: parent.height
							source: model.icon
						}
                    }

                    // TODO show installing progress bar

                    ProgressionSlot {}
                }

                function slot_installedPackageDetailsReady(pkg) {
                    PackagesCache.packageDetailsReady.disconnect(slot_installedPackageDetailsReady)
                    bottomEdgeStack.clear()
                    bottomEdgeStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), { app: pkg })
                }

                onClicked: {
                    PackagesCache.packageDetailsReady.connect(slot_installedPackageDetailsReady)
                    PackagesCache.getPackageDetails(model.appId)
                }
            }
        }
    }

    Loader {
        anchors.centerIn: parent
        active: sortedModel.count == 0
        sourceComponent: Components.EmptyState {
            title: i18n.tr("No apps found")
            subTitle: i18n.tr("No app has been installed from OpenStore yet.")
            iconName: "ubuntu-store-symbolic"
            anchors.centerIn: parent
        }
    }
}
