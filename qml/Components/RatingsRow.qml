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

Row {
    property var appRatings

    width: parent.width
    anchors.horizontalCenter: parent.horizontalCenter

    ReviewItemVertical {
        id: tup
        reviewIcon: "../Assets/thumbup.svg"
        reviewNumber: appRatings.thumbsUpCount || 0
        enabled: appRatings.thumbsUpCount > 0
        ratingColor: appColorText                                                                                                                                                                                                                                                                                     ; MouseArea {anchors.fill: parent; onClicked: tup.reviewIcon="../Assets/t-up.svg"}
    }

    ReviewItemVertical {
        reviewIcon: "../Assets/happy.svg"
        reviewNumber: appRatings.happyCount || 0
        enabled: appRatings.happyCount > 0
        ratingColor: appColorText
    }

    ReviewItemVertical {
        reviewIcon: "../Assets/neutral.svg"
        reviewNumber: appRatings.neutralCount || 0
        enabled: appRatings.neutralCount > 0
        ratingColor: appColorText
    }

    ReviewItemVertical {
        reviewIcon: "../Assets/thumbdown.svg"
        reviewNumber: appRatings.thumbsDownCount || 0
        enabled: appRatings.thumbsDownCount > 0
        ratingColor: appColorText
    }

    ReviewItemVertical {
        reviewIcon: "../Assets/buggy.svg"
        reviewNumber: appRatings.buggyCount || 0
        enabled: appRatings.buggyCount > 0
        ratingColor: appColorText
    }

    function getNumberShortForm(number) {
        if (number > 999999) {
            return Math.floor(number/1000000) + "M"
        }
        else if (number > 999) {
            return Math.floor(number/1000) + "K"
        }
        else return number + ""
    }
}
