import QtQuick 2.4
import Ubuntu.Components 1.3

Item {
    id: zI
    width: parent.width
    height: parent.height
    z: Number.MAX_VALUE

    Rectangle {
        id: zIbg
        parent: zI.parent
        anchors.fill: parent
        color: "black"
        opacity: 0
    }

    property real itemScale: 1
    property var model
    property int currentIndex

    transform: Scale {
        origin.x: 0
        origin.y: 0
        xScale: zI.itemScale
        yScale: zI.itemScale
    }

    ParallelAnimation {
        id: scaleInAnimation
        onStarted: {
            hideAnimation.initialScale = itemScale;
            hideAnimation.initialX = x;
            hideAnimation.initialY = y;
        }

        UbuntuNumberAnimation { target: zI; property: "itemScale"; to: 1 }
        UbuntuNumberAnimation { target: zI; properties: "x,y"; to: 0 }
        UbuntuNumberAnimation { target: zIbg; property: "opacity"; to: 1 }
    }


    Component.onCompleted: {
        scaleInAnimation.start();
    }

    MouseArea {
        // Capture all mouse/touch events below
        anchors.fill: parent
        onWheel: wheel.accepted = true  // wheel events are not captured by default
    }

    ListView {
        anchors.fill: parent
        clip: true

        model: zI.model
        currentIndex: zI.currentIndex
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        delegate: Image {
            width: zI.width
            height: zI.height
            source: modelData
            fillMode: Image.PreserveAspectFit
       }
    }

    AbstractButton {
        id: closeBtn
        anchors.top: parent.top
        anchors.left: parent.left

        width: units.gu(8)
        height: width

        styleName: "IconButtonStyle"
        StyleHints {
            foregroundColor: "white"
            backgroundColor: closeBtn.pressed ? "#666666" : "transparent"
        }

        action: Action {
            iconName: "close"
            onTriggered: {
                hideAnimation.start()
            }
        }
    }

    ParallelAnimation {
        id: hideAnimation
        property real initialScale: 1
        property int initialX: 0
        property int initialY: 0


        UbuntuNumberAnimation { target: zI; property: "itemScale"; to: hideAnimation.initialScale }
        UbuntuNumberAnimation { target: zI; property: "x"; to: hideAnimation.initialX }
        UbuntuNumberAnimation { target: zI; property: "y"; to: hideAnimation.initialY }
        UbuntuNumberAnimation { target: zIbg; property: "opacity"; to: 0 }
        onStopped: {
            script: zI.destroy()
        }

    }
}
