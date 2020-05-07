import QtQuick 2.4
import QtGraphicalEffects 1.0
import Ubuntu.Components 1.3

Row {
    id: ratingsRow
    width: parent.width
    height: units.gu(0.4)

    // TODO explicity pass this info in
    readonly property int ratingCount: appItem.ratings.thumbsUpCount +
        appItem.ratings.happyCount +
        appItem.ratings.thumbsDownCount +
        appItem.ratings.buggyCount
    readonly property var relativPositive: ratingCount > 0 ?
        (appItem.ratings.thumbsUpCount +
        appItem.ratings.happyCount) /
        ratingCount
        : 0.5
    visible: ratingCount !== 0

    Rectangle {
        height: parent.height
        width: parent.width * ratingsRow.relativPositive
        color: UbuntuColors.green
    }

    Rectangle {
        height: parent.height
        width: parent.width * (1-ratingsRow.relativPositive)
        color: UbuntuColors.red
    }
}
