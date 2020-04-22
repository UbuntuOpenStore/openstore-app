import QtQuick 2.4
import Ubuntu.Components 1.3

Item {
    id: rootItem

    property alias text: sectionLabel.text
    property alias subtext: sectionSublabel.text
    property alias buttonText: button.text
    property alias buttonEnabled: button.enabled

    signal buttonClicked();

    anchors {
        left: parent.left
        right: parent.right
        margins: units.gu(2)
    }
    height: units.gu(4)

    Label {
        id: sectionLabel
        anchors {
            top: sectionSublabel.text ? parent.top : undefined
            topMargin: sectionSublabel.text ? units.gu(2) : 0
            left: parent.left
            verticalCenter: sectionSublabel.text ? undefined : parent.verticalCenter
        }

        textSize: Label.Small
        color: theme.palette.normal.backgroundTertiaryText
    }

    Label {
        id: sectionSublabel
        visible: !!text
        width: parent.width

        anchors {
            top: sectionLabel.bottom
            topMargin: units.gu(1)
        }

        textSize: Label.XSmall
        wrapMode: Label.WordWrap
        color: theme.palette.normal.backgroundTertiaryText
    }

    Button {
        id: button
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        visible: !!text
        color: theme.palette.normal.positive
        onClicked: buttonClicked()
    }
}
