import QtQuick 2.4
import Ubuntu.Components 1.3

UbuntuShape {
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
    aspect: UbuntuShape.Inset
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

        if (bC > Math.max(hC, nC, tDC, tUC)) {
            return "../Assets/buggy-full.svg";
        } else if (hC > Math.max(bC, nC, tDC, tUC)) {
            return "../Assets/happy-full.svg";
        } else if (nC > Math.max(bC, hC, tDC, tUC)) {
            return "../Assets/neutral-full.svg";
        } else if (tDC > Math.max(bC, hC, nC, tUC)) {
            return "../Assets/thumbdown-full.svg";
        } else if (tUC > Math.max(bC, hC, nC, tDC)) {
            return "../Assets/thumbup-full.svg";
        }

        //There's more than one type with the same rating
        else if (hC + tUC > bC + tDC +  nC) {
            return "../Assets/thumbup-general.svg";
        } else if (bC + tDC > hC + tUC +  nC) {
            return "../Assets/thumbdown-general.svg";
        } else

        return "../Assets/neutral.svg";
    }
}
