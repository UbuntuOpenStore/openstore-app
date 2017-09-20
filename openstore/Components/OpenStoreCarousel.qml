import QtQuick 2.4
import Ubuntu.Components 1.3

Column {
    id: rootItem

    property string title
    property string subtitle
    property bool showProgression: true

    signal titleClicked()
    signal appTileClicked(var appItem)

    property alias viewModel: view.model
    function packageInfoGetter(i) { return null }

    property int tilesWidth: units.gu(10)

    spacing: units.gu(1)

    ListItem {
        divider.visible: false
        onClicked: rootItem.titleClicked()
        visible: rootItem.title || rootItem.subtitle

        ListItemLayout {
            anchors.centerIn: parent
            title.text: rootItem.title
            subtitle.text: rootItem.subtitle
            subtitle.wrapMode: Text.WordWrap

            ProgressionSlot {
                visible: rootItem.showProgression
            }
        }
    }

    ListView {
        id: view
        anchors { left: parent.left; right: parent.right }
        leftMargin: units.gu(2)
        rightMargin: units.gu(2)

        clip: true

        height: count > 0 ? rootItem.tilesWidth * 1.5 + units.gu(4) : 0
        visible: count > 0

        spacing: units.gu(1.5)

        orientation: ListView.Horizontal
        delegate: PackageTile {
            id: appDel

            appItem: rootItem.packageInfoGetter(modelData ? modelData : model.index)
            height: parent.height
            width: rootItem.tilesWidth

            onClicked: rootItem.appTileClicked(appDel.appItem)
        }
    }
}
