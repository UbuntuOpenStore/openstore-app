import QtQuick 2.4
import Ubuntu.Components 1.3

StyledItem {
    id: rootItem

    property var model
    property int selectedIndex: 0

    signal tabThumbClicked(var index)

    anchors {
        left: parent.left
        top: parent.top
        bottom: parent.bottom
    }

    width: visible ? units.gu(10) : 0

    Rectangle {
        anchors.fill: parent
        color: rootItem.theme.palette.normal.background
    }

    Column {
        anchors.fill: parent
        Repeater {
            id: tabsRepeater
            model: rootItem.model

            AbstractButton {
                id: tabsBtn

                property bool isSelected: model.index == rootItem.selectedIndex

                width: parent.width - units.dp(1)
                height: units.gu(8)

                onClicked: rootItem.tabThumbClicked(model.index)

                Rectangle {
                    anchors.right: parent.right
                    width: units.dp(2)
                    height: parent.height
                    color: isSelected ? theme.palette.selected.focus : theme.palette.normal.foreground
                }

                Rectangle {
                    anchors.fill: parent
                    color: rootItem.theme.palette.normal.base
                    visible: tabsBtn.pressed
                }

                Column {
                    anchors.centerIn: parent
                    width: tabsBtn.width - units.gu(2)
                    spacing: units.gu(0.5)

                    Icon {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: units.gu(2.5)
                        height: width
                        color: isSelected ? theme.palette.selected.focus : theme.palette.normal.backgroundTertiaryText
                        name: modelData.iconName

                        Rectangle {
                            anchors.horizontalCenter: parent.right
                            anchors.verticalCenter: parent.top
                            width: children[0].width + units.gu(1)
                            height: children[0].height + units.gu(0.5)
                            color: UbuntuColors.blue
                            radius: units.dp(4)
                            visible: modelData.count || modelData.count > 0
                            Label {
                                anchors.centerIn: parent
                                textSize: Label.XSmall
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
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        width: units.dp(1)
        color: rootItem.theme.palette.normal.base
    }
}
