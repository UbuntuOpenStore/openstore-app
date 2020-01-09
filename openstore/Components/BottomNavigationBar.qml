import QtQuick 2.4
import Ubuntu.Components 1.3

StyledItem {
    id: rootItem

    property var model
    property int selectedIndex: 0

    signal tabThumbClicked(var index)

    anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
    }

    height: visible ? units.gu(7) : 0

    Rectangle {
        anchors.fill: parent
        color: theme.palette.normal.background
    }

    Row {
        width: Math.min(parent.width, units.gu(80))
        anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }

        Repeater {
            id: tabsRepeater
            model: rootItem.model

            AbstractButton {
                id: tabsBtn

                property bool isSelected: model.index == rootItem.selectedIndex

                width: parent.width / tabsRepeater.count
                height: parent.height - units.dp(1)
                anchors.bottom: parent.bottom

                onClicked: rootItem.tabThumbClicked(model.index)

                Rectangle {
                    anchors.top: parent.top
                    width: parent.width
                    height: units.dp(2)
                    color: isSelected ? theme.palette.selected.focus : theme.palette.normal.foreground
                }

                Rectangle {
                    anchors.fill: parent
                    color: theme.palette.normal.base
                    visible: tabsBtn.pressed
                }

                Column {
                    anchors.centerIn: parent
                    width: tabsBtn.width - units.gu(2)
                    spacing: units.gu(0.5)

                    Icon {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: units.gu(2.5); height: width
                        color: isSelected ? theme.palette.selected.focus : theme.palette.normal.backgroundTertiaryText
                        name: modelData.iconName

                        Rectangle {
                            anchors.horizontalCenter: parent.right
                            anchors.verticalCenter: parent.top
                            width: children[0].width + units.gu(1)
                            height: children[0].height + units.gu(0.5)
                            radius: units.dp(4)
                            color: UbuntuColors.blue
                            visible: modelData.count || modelData.count > 0
                            Label {
                                anchors.centerIn: parent
                                textSize: Label.Small
                                color: "white"
                                text: modelData.count || ""
                            }
                        }
                    }

                    Label {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        textSize: Label.Small
                        text: modelData.name
                        color: isSelected ? theme.palette.selected.focus : theme.palette.normal.backgroundTertiaryText
                    }
                }
            }
        }
    }

    Rectangle {
        anchors { left: parent.left; right: parent.right }
        height: units.dp(1)
        color: theme.palette.normal.base
    }
}
