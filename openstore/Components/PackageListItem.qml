import QtQuick 2.4
import Ubuntu.Components 1.3

ListItem {
    id: rootItem
    property var appItem
    property bool showTicks: true

    height: layout.height

    ListItemLayout {
        id: layout

        title.text: rootItem.appItem.name
        summary.text: {
            var translations = {
                'app': i18n.tr("App"),
                'scope': i18n.tr("Scope"),
                'webapp': i18n.tr("Web App"),
                'webapp+': i18n.tr("Web App+"),
            };

            var types = [];
            for (var i = 0; i < rootItem.appItem.types.length; i++) {
                if (translations[rootItem.appItem.types[i]]) {
                    types.push(translations[rootItem.appItem.types[i]]);
                }
            }

            return types.join(', ') + '\n' + rootItem.appItem.tagline
        }
        summary.wrapMode: Text.WrapAtWordBoundaryOrAnywhere

        UbuntuShape {
            SlotsLayout.position: SlotsLayout.Leading
            aspect: UbuntuShape.Flat
            image: Image {
                source: rootItem.appItem.icon
                sourceSize.width: parent.width
                sourceSize.height: parent.height
                height: parent.height
                width: parent.width
            }
        }

        Icon {
            SlotsLayout.position: SlotsLayout.Trailing
            height: units.gu(2)
            width: height
            implicitHeight: height
            implicitWidth: width
            visible: rootItem.appItem.installed && rootItem.showTicks
            name: "tick"
            color: rootItem.appItem.updateAvailable ? theme.palette.selected.focus : theme.palette.normal.positive
        }

        ProgressionSlot {}
    }
}
