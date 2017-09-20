import QtQuick 2.4
import Ubuntu.Components 1.3

Item {
    id: rootItem
    anchors.fill: parent

    property alias stack: internalStack

    function push(page, properties) {
        var pageObject = internalStack.push(page, properties)

        // set the back action for Page.header:
        if (pageObject && pageObject.hasOwnProperty("header") && pageObject.header &&
                pageObject.header.hasOwnProperty("navigationActions")) {

            // Page.header is an instance of PageHeader.
            pageObject.header.navigationActions = [ d.backAction ];
        }

        return pageObject
    }

    function pop() {
        internalStack.pop()
    }

    function clear() {
        internalStack.clear()
    }

    Rectangle {
        anchors.fill: parent
        color: UbuntuColors.jet
        opacity: 0.8
        visible: d.isStackVisible
    }

    MouseArea {
        // Capture all mouse/touch events beneath 'mainContainer'
        anchors.fill: parent
        enabled: internalStack.depth > 0
        onWheel: wheel.accepted = true  // wheel events are not captured by default
    }

    Item {
        id: mainContainer

        width: d.isWide ? Math.min(parent.width - units.gu(12), units.gu(100)) : parent.width
        anchors {
            top: parent.top
            topMargin: d.isWide ? rootItem.height * 0.1 : 0
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }

        states: State {
            when: !d.isStackVisible
            PropertyChanges { target: mainContainer; anchors.topMargin: rootItem.height }
        }

        Behavior on anchors.topMargin {
            NumberAnimation { easing.type: Easing.InOutQuad }
        }

    /*    InverseMouseArea {
            anchors.fill: parent
            onClicked: internalStack.clear()
            enabled: visible
        }*/

        Rectangle {
            anchors.fill: parent
            color: root.backgroundColor
        }

        PageStack {
            id: internalStack
            anchors.fill: parent
        }
    }

    QtObject {
        id: d
        property bool isStackVisible: internalStack.depth > 0
        readonly property bool isWide: rootItem.width > units.gu(90)

        property Action backAction: Action {
            iconName: internalStack.depth > 1
                      ? Qt.application.layoutDirection == Qt.RightToLeft ? "next": "back"
                      : "close"
            text: internalStack.depth > 1 ? i18n.tr("Back") : i18n.tr("Close")
            onTriggered: {
                if (internalStack.depth > 1) {
                    internalStack.pop()
                } else {
                    internalStack.clear()
                }
            }
        }
    }
}
