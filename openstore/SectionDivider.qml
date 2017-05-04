import QtQuick 2.4
import Ubuntu.Components 1.3

Rectangle {
    id: rootItem

    property alias text: sectionLabel.text
    property alias iconName: icon.name

    anchors { left: parent.left; right: parent.right }
    height: units.gu(4)

    color: theme.palette.normal.foreground

    Row {
        anchors {
            left: parent.left
            right: parent.right
            margins: units.gu(2)
            verticalCenter: parent.verticalCenter
        }
        spacing: units.gu(1)

        Icon {
            id: icon
            height: units.gu(2)
            width: name ? units.gu(2) : 0
            anchors.verticalCenter: parent.verticalCenter
        }

        Label {
            id: sectionLabel
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        height: units.dp(2)
        gradient: Gradient {
            GradientStop { position: 0.0;  color: Qt.rgba(0, 0, 0, 0.1) }
            GradientStop { position: 0.49; color: Qt.rgba(0, 0, 0, 0.1) }
            GradientStop { position: 0.5;  color: Qt.rgba(1, 1, 1, 0.4) }
            GradientStop { position: 1.0;  color: Qt.rgba(1, 1, 1, 0.4) }
        }
    }
}
