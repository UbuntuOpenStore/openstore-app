/*
 * Copyright (C) 2019 - Stefan Weng <stefwe@mailbox.org>
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
    width: units.gu(3); height: width

    property string reviewIcon
    property int reviewNumber

    Row {
        width: units.gu(3)
        height: width
        spacing: units.gu(0.5)

        Label {
            width: units.gu(3); height: width
            textSize: Label.Large
            text: reviewIcon
        }

        Label {
            anchors.verticalCenter: parent.verticalCenter
            width: units.gu(3)
            text: getNumberShortForm(reviewNumber)
        }
    }
}
