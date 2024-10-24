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
import Lomiri.Components 1.3
import OpenStore 1.0

LomiriShape {
    // TODO explicity pass this info in
    readonly property int ratingCount: appItem.ratings.totalCount

    property alias mostVotedIcon: icon.source
    property alias mostRatedVisible: icon.visible

    visible: ratingCount !== 0

    anchors {
        centerIn: parent

        horizontalCenterOffset: parent.width * .4
        verticalCenterOffset: - parent.height * .4
    }

    width: units.gu(3)
    height: width
    aspect: LomiriShape.Inset
    backgroundColor: theme.palette.normal.background

    Icon {
        id: icon
        width: units.gu(2.5)
        height: width
        visible: parent.visible
        anchors.centerIn: parent
        asynchronous: true
        color: theme.palette.normal.baseText
        source: getMostRated()
    }

    function getMostRated() {
        //There's no rating
        if (ratingCount == 0) {
            return "";
        }

        //One type is bigger than the rest
        var bC = appItem.ratings.buggyCount;
        var hC = appItem.ratings.happyCount;
        var nC = appItem.ratings.neutralCount;
        var tDC = appItem.ratings.thumbsDownCount;
        var tUC = appItem.ratings.thumbsUpCount;

        //NEUTRAL: if neutral > positive && neutral > negative
        //         or positive > negative && positive < (negative + 20%):
        if (nC > Math.max(bC + tDC, hC + tUC) || ((hC + tUC >= bC + tDC) && (hC + tUC < (bC + tDC) * 1.2))) {
            return "../Assets/neutral-full.svg";

        //NEGATIVE: negative > positive
        } else if (bC + tDC > hC + tUC) {
            if (bC > tDC) return "../Assets/buggy-full.svg";
            else return "../Assets/thumbdown-full.svg";

        //POSITIVE: positive > negative
        } else if (hC > tUC) return "../Assets/happy-full.svg";
          else return "../Assets/thumbup-full.svg";
    }
}
