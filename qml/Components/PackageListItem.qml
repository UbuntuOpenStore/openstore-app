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

ListItem {
    id: rootItem
    property var appItem
    property bool showTicks: true

    height: layout.height

    ListItemLayout {
        id: layout

        title.text: rootItem.appItem.name
        summary.text: {
            var translations = {
                'app': i18n.tr("App"),
                'scope': i18n.tr("Scope"),
                'webapp': i18n.tr("Web App"),
                'webapp+': i18n.tr("Web App+"),
            };

            var types = [];
            for (var i = 0; i < rootItem.appItem.types.length; i++) {
                if (translations[rootItem.appItem.types[i]]) {
                    types.push(translations[rootItem.appItem.types[i]]);
                }
            }

            return types.join(', ') + '\n' + rootItem.appItem.tagline
        }
        summary.wrapMode: Text.WrapAtWordBoundaryOrAnywhere

        UbuntuShape {
            id: appIconShape
            aspect: UbuntuShape.Flat
            SlotsLayout.position: SlotsLayout.Leading

            image: Image {
                source: rootItem.appItem.icon
                sourceSize.width: parent.width
                sourceSize.height: parent.height
                height: parent.height
                width: parent.width
            }

            MostRated {}
        }

        Icon {
            SlotsLayout.position: SlotsLayout.Trailing
            height: units.gu(2)
            width: height
            implicitHeight: height
            implicitWidth: width
            visible: rootItem.appItem.installed && rootItem.showTicks
            name: "tick"
            color: rootItem.appItem.updateAvailable ? theme.palette.selected.focus : theme.palette.normal.positive
        }

        ProgressionSlot {}
    }
}
