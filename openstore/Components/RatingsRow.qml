import QtQuick 2.4
import Ubuntu.Components 1.3

Row {
    property var appRatings

    width: parent.width
    anchors.horizontalCenter: parent.horizontalCenter

    ReviewItemVertical {
        id: tup
        reviewIcon: "../Assets/thumbup.svg"
        reviewNumber: appRatings.thumbsUpCount
        enabled: appRatings.thumbsUpCount > 0
        ratingColor: appColorText                                                                                                                                                                                                                                                                                     ; MouseArea {anchors.fill: parent; onClicked: tup.reviewIcon="../Assets/t-up.svg"}
    }

    ReviewItemVertical {
        reviewIcon: "../Assets/happy.svg"
        reviewNumber: appRatings.happyCount
        enabled: appRatings.happyCount > 0
        ratingColor: appColorText
    }

    ReviewItemVertical {
        reviewIcon: "../Assets/neutral.svg"
        reviewNumber: appRatings.neutralCount
        enabled: appRatings.neutralCount > 0
        ratingColor: appColorText
    }

    ReviewItemVertical {
        reviewIcon: "../Assets/thumbdown.svg"
        reviewNumber: appRatings.thumbsDownCount
        enabled: appRatings.thumbsDownCount > 0
        ratingColor: appColorText
    }

    ReviewItemVertical {
        reviewIcon: "../Assets/buggy.svg"
        reviewNumber: appRatings.buggyCount
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
