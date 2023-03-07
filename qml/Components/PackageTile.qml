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
import Ubuntu.Components 1.3

AbstractButton {
    id: rootItem
    property var appItem

    signal imageLoaded()

    Column {
        anchors.fill: parent

        UbuntuShape {
            width: parent.width
            height: width
            aspect: UbuntuShape.Flat
            sourceFillMode: UbuntuShape.PreserveAspectFit

            source: Image {
                source: rootItem.appItem.icon
                sourceSize.width: parent.width
                sourceSize.height: parent.height

                visible: false

                onStatusChanged: {
                    // WORKAROUND: Since we use PackageTile in ListView
                    // delegates, we need to delay delegates destruction
                    // until the image is loaded.
                    if (status == Image.Ready) {
                        rootItem.imageLoaded()
                    }
                }
            }

            MostRated {}
        }

        ListItemLayout {
            anchors {
                left: parent.left; leftMargin: units.gu(-2)
                right: parent.right; rightMargin: units.gu(-2)
            }

            height: units.gu(4)
            title {
                text: rootItem.appItem.name
                textSize: Label.Small
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                maximumLineCount: 2
            }

            subtitle {
                text: {
                    // Show the author (like in the discover page) or the types if author isn't available (like in the filtered app list)

                    if (rootItem.appItem.author) {
                        return rootItem.appItem.author;
                    }

                    var translations = {
                        'app': i18n.tr("App"),
                        'webapp': i18n.tr("Bookmark"),
                        'webapp+': i18n.tr("Web App"),
                    };

                    var types = [];
                    for (var i = 0; i < rootItem.appItem.types.length; i++) {
                        if (translations[rootItem.appItem.types[i]]) {
                            types.push(translations[rootItem.appItem.types[i]]);
                        }
                    }

                    return types.join(', ');
                }

                textSize: Label.XSmall
            }

            summary.text: rootItem.appItem.installed ? rootItem.appItem.updateAvailable ? i18n.tr("Update available").toUpperCase() : i18n.tr("✓ Installed").toUpperCase() : ""
            summary.textSize: Label.XSmall
        }
    }
}
