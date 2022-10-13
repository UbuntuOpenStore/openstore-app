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
import Ubuntu.Components 1.3
import Ubuntu.Connectivity 1.0
import OpenStore 1.0
import QtQuick.Layouts 1.1

import "Components" as Components

Page {
    id: rootItem

    property var currentApp: null
    property var appsUpdating: []

    property bool updating: (PlatformIntegration.clickInstaller.busy && !PlatformIntegration.clickInstaller.isLocalInstall) || PackagesCache.updatingCache

    function updateNextPackage() {
        //console.log('updateNextPackage', JSON.stringify(appsUpdating));

        if (appsUpdating.length > 0) {
            var nextApp = appsUpdating.shift();
            currentApp = nextApp.appId;

            PlatformIntegration.clickInstaller.installPackage(nextApp.packageUrl);
        }
    }

    Connections {
        target: PlatformIntegration.clickInstaller
        onPackageInstalled: updateNextPackage()
        onPackageInstallationFailed: updateNextPackage()
    }

    header: Components.HeaderMain {
        title: i18n.tr("Installed Apps")
        flickable: view
    }

    ScrollView {
        id: scrollView
        width: parent.width
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        ListView {
            id: view
            anchors.fill: parent

            model: localAppModel

            // WORKAROUND: Fix for wrong grid unit size
            Component.onCompleted: root.flickable_responsive_scroll_fix(view)

            section.property: 'updateStatus'
            section.delegate: Components.SectionDivider {
                height: (section == 'downgrade') ? units.gu(9) : units.gu(6)

                text: {
                    if (section == 'none') {
                        // TRANSLATORS: %1 is the number of installed apps
                        return i18n.tr("Installed apps (%1)").arg(localAppModel.count - localAppModel.updatesAvailableCount - localAppModel.downgradesAvailableCount);
                    }
                    else if (section == 'available') {
                        // TRANSLATORS: %1 is the number of available app updates
                        return i18n.tr("Available updates (%1)").arg(localAppModel.updatesAvailableCount);
                    }

                    // TRANSLATORS: %1 is the number of apps that can be downgraded
                    return i18n.tr("Stable version available (%1)".arg(localAppModel.downgradesAvailableCount));
                }

                subtext: (section == 'downgrade') ? i18n.tr("The installed versions of these apps did not come from the OpenStore but a stable version is available.") : '';
                buttonText: (section == 'available') ? i18n.tr("Update all") : ''
                buttonEnabled: !PlatformIntegration.clickInstaller.busy

                onButtonClicked: {
                    var updates = [];
                    for (var i = 0; i < localAppModel.count; i++) {
                        var app = localAppModel.get(i);

                        if (app && app.updateStatus == 'available' && app.packageUrl) {
                            updates.push(app);
                        }
                    }

                    appsUpdating = updates;
                    updateNextPackage();
                }
            }

            delegate: ListItem {
                height: units.gu(7)
                divider.anchors.leftMargin: units.gu(8)

                ListItemLayout {
                    id: layout

                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        top: parent.top
                    }

                    title.text: model.name

                    Item {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(4); height: width

                        UbuntuShape {
                            id: appIcon
                            anchors.fill: parent
                            opacity: updateSpinner.visible ? .2 : 1
                            aspect: UbuntuShape.Flat
                            image: Image {
                                sourceSize.width: parent.width
                                sourceSize.height: parent.height
                                source: model.icon
                            }
                        }

                        ActivityIndicator {
                            id: updateSpinner
                            width: parent.width - units.gu(1)
                            height: width
                            anchors.centerIn: parent
                            visible: updating && currentApp == model.appId
                            running: visible
                        }
                    }

                    ProgressionSlot {
                        name: updating && currentApp == model.appId ? "close" : "next"
                    }
                }

                function slot_packageFetchError(appId) {
                    PackagesCache.packageDetailsReady.disconnect(slot_installedPackageDetailsReady);
                    PackagesCache.packageFetchError.disconnect(slot_packageFetchError);

                    bottomEdgeStack.clear();
                    bottomEdgeStack.push(Qt.resolvedUrl("AppLocalDetailsPage.qml"), { app: localAppModel.getByAppId(appId) });
                }

                function slot_installedPackageDetailsReady(pkg) {
                    PackagesCache.packageDetailsReady.disconnect(slot_installedPackageDetailsReady);
                    PackagesCache.packageFetchError.disconnect(slot_packageFetchError);

                    bottomEdgeStack.clear();
                    bottomEdgeStack.push(Qt.resolvedUrl("AppDetailsPage.qml"), { app: pkg });
                }

                onClicked: {
                    if (updating && currentApp == model.appId) {
                        PlatformIntegration.clickInstaller.abortInstallation()
                    } else {
                        if (Connectivity.online) {
                            PackagesCache.packageDetailsReady.connect(slot_installedPackageDetailsReady);
                            PackagesCache.packageFetchError.connect(slot_packageFetchError);
                            PackagesCache.getPackageDetails(model.appId);
                        }
                        else {
                            bottomEdgeStack.clear();
                            bottomEdgeStack.push(Qt.resolvedUrl("AppLocalDetailsPage.qml"), { app: localAppModel.getByAppId(model.appId) });
                        }
                    }
                }
            }
        }
    }

    Loader {
        anchors.centerIn: parent
        active: localAppModel.count == 0
        sourceComponent: Components.EmptyState {
            title: i18n.tr("No apps found")
            subTitle: i18n.tr("No app has been installed from OpenStore yet.")
            iconName: "ubuntu-store-symbolic"
            anchors.centerIn: parent
        }
    }
}
