/*
 * Copyright (C) 2014, 2015, 2016 Canonical Ltd
 * Copyright (C) 2020 Brian Douglass
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
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

/*
 Component which displays an empty state (approved by design). It offers an
 icon, title and subtitle to describe the empty state.
*/

Column {
    id: emptyState
    spacing: units.gu(2)
    width: units.gu(36)

    // Public APIs
    default property alias iconPlaceholder: iconContainer.data
    property alias iconName: emptyIcon.name
    property alias title: emptyLabel.text
    property alias subTitle: emptySublabel.text

    property alias controlComponent: controlLoader.sourceComponent

    Item {
        width: parent.width
        height: childrenRect.height
        Icon {
            id: emptyIcon
            height: visible ? units.gu(10) : 0
            width: visible ? height : 0
            color: theme.palette.normal.base
            visible: name || source
        }
        Row {
            id: iconContainer
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Label {
        id: emptyLabel
        width: parent.width
        horizontalAlignment: Text.AlignLeft
        textSize: Label.XLarge

        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        maximumLineCount: 2
    }

    Label {
        id: emptySublabel
        width: parent.width
        horizontalAlignment: Text.AlignLeft
        textSize: Label.Medium

        elide: Text.ElideRight
        wrapMode: Text.WordWrap
    }

    Loader {
        id: controlLoader
    }
}
