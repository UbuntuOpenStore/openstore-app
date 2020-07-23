/*
 * Copyright (C) 2019 - Stefan Weng <stefwe@mailbox.org>
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

Item {
    property bool enabled: true

    width: units.gu(2.5)
    height: width
    opacity: enabled ? 1 : 0.4

    property string reviewIcon
    property string ratingColor
    property int reviewNumber

    Column {
        width: units.gu(2.5)
        height: width
        spacing: units.gu(0.5)

        Icon {
            width: units.gu(2.5)
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
            asynchronous: true
            color: ratingColor || theme.palette.normal.baseText
            source: reviewIcon
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            width: units.gu(2)
            text: getNumberShortForm(reviewNumber)
            color: ratingColor || theme.palette.normal.baseText
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
