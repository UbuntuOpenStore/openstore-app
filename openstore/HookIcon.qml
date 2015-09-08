import QtQuick 2.0
import QtQuick.Layouts 1.1
import Ubuntu.Components 1.0
import Ubuntu.Components.Popups 1.0

Icon {
    id: root
    AbstractButton {
        anchors.fill: parent
        onClicked: {
            PopupUtils.open(popover, parent)
        }
    }

    Component {
        id: popover
        Popover {
            Item {
                height: contentColumn.implicitHeight + units.gu(2)
                width: parent.width
                Column {
                    id: contentColumn
                    anchors { left: parent.left; top: parent.top; right: parent.right }
                    anchors.margins: units.gu(1)
                    spacing: units.gu(1)
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "stock_application"
                        }
                        Label {
                            text: "Application"
                            Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "search"
                        }
                        Label {
                            text: "Scope"
                            Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "stock_website"
                        }
                        Label {
                            text: "URL Handler"
                            Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "share"
                        }
                        Label {
                            text: "Content Hub Handler"
                            Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "notification"
                        }
                        Label {
                            text: "Push Helper"
                            Layout.fillWidth: true
                        }
                    }
                    RowLayout {
                        width: parent.width
                        spacing: units.gu(1)
                        Icon {
                            Layout.preferredHeight: units.gu(4)
                            Layout.preferredWidth: units.gu(4)
                            name: "contact-group"
                        }
                        Label {
                            text: "Accounts provider"
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
