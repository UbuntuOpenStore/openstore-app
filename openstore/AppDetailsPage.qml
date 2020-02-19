/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import Ubuntu.Components.Popups 1.3
import QtQuick.Layouts 1.1
import OpenStore 1.0

import "Components" as Components

Page {
    id: appDetailsPage

    property var app: null

    header: PageHeader {
        title: app ? app.name : i18n.tr("App details")
        enabled: !PlatformIntegration.clickInstaller.busy

        trailingActionBar {
            numberOfSlots: 1
            actions: Action {
                iconName: "delete"
                text: i18n.tr("Remove")
                visible: app.installed

                onTriggered: {
                    var popup = PopupUtils.open(removeQuestion, root, {pkgName: app.name || app.id});
                    popup.accepted.connect(function() {
                        app.remove()
                    })
                }
            }
        }
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.topMargin: parent.header.height

        // WORKAROUND: Fix for wrong grid unit size
        Component.onCompleted: root.flickable_responsive_scroll_fix(scrollView.flickableItem)

        Column {
            id: mainColumn
            width: scrollView.width
            //spacing: units.gu(1)

            ListItem {
                height: units.gu(16)

                ListItemLayout {
                    anchors.fill: parent
                    title.text: app.name
                    subtitle.text: app.author
                    summary.text: {
                        var translations = {
                            'app': i18n.tr("App"),
                            'scope': i18n.tr("Scope"),
                            'webapp': i18n.tr("Web App"),
                            'webapp+': i18n.tr("Web App+"),
                        };

                        var types = [];
                        for (var i = 0; i < app.types.length; i++) {
                            if (translations[app.types[i]]) {
                                types.push(translations[app.types[i]]);
                            }
                        }

                        var filesize = app.fileSize ? '\n' + printSize(app.fileSize) : '';
                        return types.join(', ') + filesize;
                    }

                    UbuntuShape {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(12); height: width
                        aspect: UbuntuShape.Flat

                        image: Image {
                            height: parent.height
                            width: parent.width
                            sourceSize.width: parent.width
                            sourceSize.height: parent.height
                            source: app ? app.icon : ""
                        }
                    }
                }
            }

            ListItem {
                visible: app.isLocalVersionSideloaded && app.channelMatchesOS && app.frameworkMatchesOS
                height: sideloadedLayout.height
                ListItemLayout {
                    id: sideloadedLayout
                    subtitle.text: i18n.tr("The installed version of this app doesn't come from the OpenStore server. You can install the latest stable update by tapping the button below.")
                    subtitle.maximumLineCount: Number.MAX_VALUE
                    subtitle.wrapMode: Text.WordWrap

                    Icon {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(4); height: width
                        name: "info"
                    }
                }
            }

            ListItem {
                height: units.gu(8)

                RowLayout {
                    id: buttonsRow
                    anchors.fill: parent
                    anchors.margins: units.gu(2)
                    spacing: units.gu(2)
                    visible: !PlatformIntegration.clickInstaller.busy && !PackagesCache.updatingCache && app.channelMatchesOS

                    Button {
                        Layout.fillWidth: true
                        Layout.maximumWidth: buttonsRow.width > units.gu(60) ? units.gu(24) : buttonsRow.width
                        text: i18n.tr("Open")
                        visible: app.installed && app.containsApp && app.appId!="openstore.openstore-team"
                        color: installUpgradeButton.visible
                            ? theme.name == "Ubuntu.Components.Themes.Ambiance"
                                ? UbuntuColors.graphite
                                : UbuntuColors.ash
                            : theme.palette.normal.positive

                        onClicked: Qt.openUrlExternally(app.appLaunchUrl())
                    }

                    Button {
                        id: installUpgradeButton
                        Layout.fillWidth: true
                        Layout.maximumWidth: buttonsRow.width > units.gu(60) ? units.gu(24) : buttonsRow.width
                        text: {
                            if (app.isLocalVersionSideloaded) {
                                return i18n.tr("Install stable version");
                            }
                            else if (app.installed) {
                                return i18n.tr("Upgrade");
                            }

                            return i18n.tr("Install");
                        }
                        visible: !app.installed || (app.installed && app.updateAvailable) || app.isLocalVersionSideloaded
                        color: app.isLocalVersionSideloaded ? theme.palette.selected.focus : theme.palette.normal.positive
                        onClicked: {
                            var isUnconfined = false;
                            for (var i=0; i<app.hooksCount; ++i) {
                                if (app.apparmorTemplate(i).indexOf("unconfined") >= 0) {
                                    isUnconfined = true
                                }
                            }

                            if (isUnconfined && !app.installed) {
                                var popup = PopupUtils.open(unconfinedWarningPopup)
                                popup.accepted.connect(function() {
                                    app.install();
                                });
                            }
                            else if (app.donateUrl && !app.installed) {
                                var popup = PopupUtils.open(donatingPopup)
                                popup.accepted.connect(function() {
                                    app.install();
                                    Qt.openUrlExternally(app.donateUrl);
                                });
                                popup.rejected.connect(function() {
                                    app.install();
                                });
                            }
                            else {
                                app.install();
                            }
                        }
                    }
                }

                RowLayout {
                    id: installationRow
                    anchors.fill: parent
                    anchors.margins: units.gu(2)
                    spacing: units.gu(2)
                    visible: (PlatformIntegration.clickInstaller.busy && !PlatformIntegration.clickInstaller.isLocalInstall) || PackagesCache.updatingCache

                    onVisibleChanged: {
                        // The page is automatically closed for channel-incompatible apps when they are removed.
                        if (app && !app.installed && !app.channelMatchesOS) {
                            pageStack.pop();
                        }
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        maximumValue: app ? app.fileSize : 0
                        value: PlatformIntegration.clickInstaller.downloadProgress
                        indeterminate: PlatformIntegration.clickInstaller.downloadProgress == 0 || PackagesCache.updatingCache
                    }

                    AbstractButton {
                        id: abortBtn
                        Layout.preferredWidth: units.gu(4)
                        Layout.fillHeight: true
                        action: Action {
                            iconName: "close"
                            onTriggered: PlatformIntegration.clickInstaller.abortInstallation()
                            enabled: PlatformIntegration.clickInstaller.downloadProgress < app.fileSize
                        }
                        Rectangle {
                            color: "#cdcdcd"
                            anchors.fill: parent
                            visible: abortBtn.pressed
                        }
                        Icon {
                            anchors.centerIn: parent
                            width: units.gu(2); height: width
                            name: abortBtn.action.iconName
                            source: abortBtn.action.iconSource
                            color: "#CDCDCD"
                        }
                    }
                }

                ListItemLayout {
                    // Layout shown when the app is not compatible with the system.
                    id: mismatchLayout
                    subtitle.text: i18n.tr("This app is not compatible with your system.")
                    subtitle.maximumLineCount: Number.MAX_VALUE
                    subtitle.wrapMode: Text.WordWrap
                    visible: !app.channelMatchesOS && !PlatformIntegration.clickInstaller.busy && !PackagesCache.updatingCache

                    Icon {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(4); height: width
                        name: 'dialog-warning-symbolic'
                    }

                    Button {
                        Layout.fillWidth: true
                        Layout.maximumWidth: buttonsRow.width > units.gu(60) ? units.gu(24) : buttonsRow.width
                        text: i18n.tr("Remove")
                        visible: app.installed && !PackagesCache.updatingCache
                        color: theme.palette.normal.negative
                        onClicked: {
                            var popup = PopupUtils.open(removeQuestion, root, {pkgName: app.name || app.id});
                            popup.accepted.connect(function() {
                                app.remove();
                            })
                        }
                    }
                }
            }

            ListItem {
                visible: {
                    for (var i=0; i<app.hooksCount; ++i) {
                        if (includesUnconfinedLocations(app.readPaths(i)))
                            return true
                        if (includesUnconfinedLocations(app.writePaths(i)))
                            return true
                        if (app.apparmorTemplate(i).indexOf("unconfined") >= 0)
                            return true
                    }
                    return false
                }
                ListItemLayout {
                    anchors.centerIn: parent
                    subtitle.text: i18n.tr("This app has access to restricted system data, see below for details.")
                    subtitle.color: theme.palette.normal.negative
                    subtitle.maximumLineCount: Number.MAX_VALUE
                    subtitle.wrapMode: Text.WordWrap

                    Icon {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(4); height: width
                        name: "security-alert"
                    }
                }
            }

            Components.ScreenshotsCarousel {
                height: units.gu(32)
                model: app.screenshots
            }

            ListItem {
                height: descLayout.height
                onClicked: descLayout.showAll = !descLayout.showAll
                ListItemLayout {
                    id: descLayout
                    property bool showAll: false
                    title.text: app.tagline || i18n.tr("Description")
                    subtitle.text: app.description
                    subtitle.textSize: Label.Small
                    subtitle.wrapMode: Text.WordWrap
                    subtitle.maximumLineCount: showAll ? Number.MAX_VALUE : 5

                    Icon {
                        width: units.gu(2); height: width
                        SlotsLayout.position: SlotsLayout.Last
                        visible: descLayout.subtitle.truncated
                        name: descLayout.showAll ? "go-up" : "go-down"
                    }
                }
            }

            ListItem {
                height: changelogLayout.height
                visible: app.changelog
                onClicked: changelogLayout.showAll = !changelogLayout.showAll
                ListItemLayout {
                    id: changelogLayout
                    property bool showAll: false
                    // TRANSLATORS: Title of the changelog section
                    title.text: i18n.tr("What's New")
                    subtitle.text: app.changelog
                    subtitle.textSize: Label.Small
                    subtitle.wrapMode: Text.WordWrap
                    subtitle.maximumLineCount: showAll ? Number.MAX_VALUE : 5

                    Icon {
                        width: units.gu(2); height: width
                        SlotsLayout.position: SlotsLayout.Last
                        visible: changelogLayout.subtitle.truncated
                        name: changelogLayout.showAll ? "go-up" : "go-down"
                    }
                }
            }

            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Packager/Publisher")
                    subtitle.text: app.maintainer || i18n.tr("OpenStore team")
                }
            }

            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Installed version")
                    subtitle.text: app.installedVersionString || "None"
                }
            }

            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Latest available version")
                    subtitle.text: "%1 (%2)".arg(app.versionString).arg(app.updatedDate.toLocaleDateString(Locale.ShortFormat))
                }
            }

            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("First released")
                    subtitle.text: app.publishedDate.toLocaleDateString()
                }
            }

            ListItem {
                id: latestVersionDownloads
                divider.visible: false
                visible: (app.latestDownloads > 0)
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Downloads of the latest version")
                    subtitle.text: app.latestDownloads
                }
            }

            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("Total downloads")
                    subtitle.text: app.totalDownloads || "<i>0</i>"
                }
            }

            ListItem {
                divider.visible: false
                ListItemLayout {
                    anchors.centerIn: parent
                    title.text: i18n.tr("License")
                    subtitle.text: app.license || "<i>" + i18n.tr("N/A") + "</i>"
                }
            }

            ListItem {
                visible: !!app.source
                onClicked: Qt.openUrlExternally(app.source)
                ListItemLayout {
                    anchors.centerIn: parent

                    title.text: i18n.tr("Source Code")
                    subtitle.text: app.source

                    ProgressionSlot {}
                }
            }

            ListItem {
                visible: !!app.supportUrl
                onClicked: Qt.openUrlExternally(app.supportUrl)
                ListItemLayout {
                    anchors.centerIn: parent

                    title.text: i18n.tr("Get support for this app")
                    subtitle.text: app.supportUrl

                    ProgressionSlot {}
                }
            }

            ListItem {
                visible: !!app.donateUrl
                onClicked: Qt.openUrlExternally(app.donateUrl)
                ListItemLayout {
                    anchors.centerIn: parent

                    title.text: i18n.tr("Donate to support this app")
                    subtitle.text: app.donateUrl

                    ProgressionSlot {}
                }
            }

            ListItem {
                enabled: !PlatformIntegration.clickInstaller.busy
                onClicked: {
                    bottomEdgeStack.clear()
                    mainPage.showSearch('author:' + app.author)
                }
                ListItemLayout {
                    anchors.centerIn: parent
                    // TRANSLATORS: This is the button that shows a list of all the packages from the same author. %1 is the name of the author.
                    title.text: i18n.tr("More from %1").arg(app.author)
                    ProgressionSlot {}
                }
            }

            ListItem {
                enabled: !PlatformIntegration.clickInstaller.busy
                onClicked: {
                    bottomEdgeStack.clear()
                    mainPage.showCategory(app.category, app.category)
                }
                ListItemLayout {
                    anchors.centerIn: parent
                    // FIXME: app.category is not localized.
                    // TRANSLATORS: This is the button that shows a list of all the other packages in the same category. %1 is the name of the category.
                    title.text: i18n.tr("Other apps in %1").arg(app.category)
                    ProgressionSlot {}
                }
            }

            Components.SectionDivider {
                text: i18n.tr("Package contents")
            }

            Repeater {
                model: app.hooksCount

                delegate: ListItem {
                    height: hookDelLayout.height + units.gu(3)

                    property var hooks: app.hooks(index)
                    property string permissions: app.permissions(index)
                    property string readpaths: app.readPaths(index)
                    property string writepaths: app.writePaths(index)
                    property string hookName: app.hookName(index)
                    property string apparmorTemplate: app.apparmorTemplate(index)

                    Column {
                        id: hookDelLayout
                        anchors { left: parent.left; right: parent.right; margins: units.gu(2) }
                        y: units.gu(1)
                        spacing: units.gu(1)

                        RowLayout {
                            width: parent.width
                            height: units.gu(4)

                            Label {
                                text: hookName
                                Layout.fillWidth: true
                            }

                            Components.HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "stock_application"
                                visible: (hooks & PackageItem.HookDesktop)
                            }
                            Components.HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "search"
                                visible: (hooks & PackageItem.HookScope)
                            }
                            Components.HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "stock_website"
                                visible: (hooks & PackageItem.HookUrls)
                            }
                            Components.HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "share"
                                visible: (hooks & PackageItem.HookContentHub)
                            }
                            Components.HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "notification"
                                visible: (hooks & PackageItem.HookPushHelper)
                            }
                            Components.HookIcon {
                                Layout.preferredHeight: units.gu(4)
                                Layout.preferredWidth: units.gu(4)
                                name: "contact-group"
                                visible: (hooks & PackageItem.HookAccountService)
                            }
                        }

                        ListItemLayout {
                            anchors { left: parent.left; right: parent.right }
                            anchors.leftMargin: units.gu(-2)
                            height: units.gu(6)
                            Icon {
                                SlotsLayout.position: SlotsLayout.Leading
                                width: units.gu(4); height: width
                                name: "security-alert"
                                visible: apparmorTemplate.indexOf("unconfined") >= 0
                            }

                            title.text: i18n.tr("AppArmor profile")
                            subtitle.text: apparmorTemplate || "Ubuntu confined app"
                            subtitle.color: apparmorTemplate.indexOf("unconfined") >= 0
                                ? theme.palette.normal.negative
                                : theme.palette.normal.backgroundSecondaryText
                            subtitle.maximumLineCount: Number.MAX_VALUE
                        }


                        ListItemLayout {
                            anchors { left: parent.left; right: parent.right }
                            anchors.leftMargin: units.gu(-2)
                            visible: permissions.length > 0

                            Icon {
                                property var restrictedPerms: ["bluetooth", "calendar", "contacts", "debug", "history", "music_files", "picture_files", "video_files"]
                                SlotsLayout.position: SlotsLayout.Leading
                                width: units.gu(4); height: width
                                name: "security-alert"
                                visible: {
                                    var length = restrictedPerms.length;
                                    while(length--) {
                                       if (permissions.indexOf(restrictedPerms[length]) > -1)
                                           return true
                                    }
                                    return false
                                }
                            }

                            title.text: i18n.tr("Permissions")
                            subtitle.maximumLineCount: Number.MAX_VALUE
                            subtitle.wrapMode: Text.WordWrap
                            subtitle.text: {
                                if (permissions) {
                                    return permissions.replace("bluetooth", "<font color=\"#ED3146\">bluetooth</font>")
                                                      .replace("calendar", "<font color=\"#ED3146\">calendar</font>")
                                                      .replace("contacts", "<font color=\"#ED3146\">contacts</font>")
                                                      .replace("debug", "<font color=\"#ED3146\">debug</font>")
                                                      .replace("history", "<font color=\"#ED3146\">history</font>")
                                                      .replace("music_files_read", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("picture_files_read", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("video_files_read", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("music_files", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("picture_files", "<font color=\"#ED3146\">music_files_read</font>")
                                                      .replace("video_files", "<font color=\"#ED3146\">music_files_read</font>")
                                }

                                // TRANSLATORS: this will show when an app doesn't need any special permissions
                                return "<i>" + i18n.tr("none required") + "</i>"
                            }
                        }

                        ListItemLayout {
                            anchors { left: parent.left; right: parent.right }
                            anchors.leftMargin: units.gu(-2)
                            visible: readpaths.length > 0

                            Icon {
                                SlotsLayout.position: SlotsLayout.Leading
                                width: units.gu(4); height: width
                                name: "security-alert"
                                visible: includesUnconfinedLocations(readpaths)
                            }

                            title.text: i18n.tr("Read paths")
                            subtitle.text: readpaths
                            subtitle.maximumLineCount: Number.MAX_VALUE
                            subtitle.wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }

                        ListItemLayout {
                            anchors { left: parent.left; right: parent.right }
                            anchors.leftMargin: units.gu(-2)
                            visible: writepaths.length > 0
                            Icon {
                                SlotsLayout.position: SlotsLayout.Leading
                                width: units.gu(4); height: width
                                name: "security-alert"
                                visible: includesUnconfinedLocations(writepaths)
                            }

                            title.text: i18n.tr("Write paths")
                            subtitle.text: writepaths
                            subtitle.maximumLineCount: Number.MAX_VALUE
                            subtitle.wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }
                    }
                }
            }
        }
    }

    Component {
        id: donatingPopup
        Dialog {
            id: donatingDialog
            title: i18n.tr("Donating")
            text: i18n.tr("Would you like to support this app with a donation to the developer?")

            signal accepted()
            signal rejected()

            Button {
                text: i18n.tr("Donate now")
                color: theme.palette.normal.positive
                onClicked: {
                    donatingDialog.accepted()
                    PopupUtils.close(donatingDialog)
                }
            }
            Button {
                text: i18n.tr("Maybe later")
                onClicked: {
                    donatingDialog.rejected();
                    PopupUtils.close(donatingDialog)
                }
            }
        }
    }

    Component {
        id: unconfinedWarningPopup
        Dialog {
            id: unconfinedWarningDialog
            title: i18n.tr("Warning")
            text: i18n.tr("This app has access to restricted parts of the system and all of your data. While the OpenStore maintainers have reviewed the code for this app for safety, they are not responsible for anything bad that might happen to your device or data from installing this app.")

            signal accepted()
            signal rejected()

            Button {
                text: i18n.tr("Install")
                color: theme.palette.normal.positive
                onClicked: {
                    unconfinedWarningDialog.accepted()
                    PopupUtils.close(unconfinedWarningDialog)
                }
            }
            Button {
                text: i18n.tr("Cancel")
                onClicked: {
                    unconfinedWarningDialog.rejected();
                    PopupUtils.close(unconfinedWarningDialog)
                }
            }
        }
    }

    Component {
        id: removeQuestion
        Dialog {
            id: removeQuestionDialog
            title: i18n.tr("Remove package")
            text: i18n.tr("Do you want to remove %1?").arg(pkgName)

            property string pkgName
            signal accepted();
            signal rejected();

            Button {
                text: i18n.tr("Remove")
                color: theme.palette.normal.negative
                onClicked: {
                    removeQuestionDialog.accepted();
                    PopupUtils.close(removeQuestionDialog)
                }
            }

            Button {
                text: i18n.tr("Cancel")
                onClicked: {
                    removeQuestionDialog.rejected();
                    PopupUtils.close(removeQuestionDialog)
                }

            }
        }
    }


    function printSize(size) {
        var s

        s = 1024 * 1024 * 1024
        if (size >= s)
            // TRANSLATORS: %1 is the size of a file, expressed in GB
            return i18n.tr("%1 GB").arg((size / s).toFixed(2));

        s = 1024 * 1024
        if (size >= s)
            // TRANSLATORS: %1 is the size of a file, expressed in MB
            return i18n.tr("%1 MB").arg((size / s).toFixed(2));

        s = 1024
        if (size >= s)
            // TRANSLATORS: %1 is the size of a file, expressed in kB
            return i18n.tr("%1 kB").arg(parseInt(size / s));

        // TRANSLATORS: %1 is the size of a file, expressed in bytes
        return i18n.tr("%1 bytes").arg(size);
    }

    function includesUnconfinedLocations(paths) {
        var p = paths.split(",")
        var j = 0

        for (var i=0; i < p.length; ++i) {
            var x = p[i]
            if (x.match(/[^\w\s]/)) {
                if (x.indexOf("/home/phablet/.cache/" + app.appId) == -1 && x.indexOf("/home/phablet/.config/" + app.appId) == -1) {
                    ++j
                }
            }
        }

        return (j > 0)
    }
}
