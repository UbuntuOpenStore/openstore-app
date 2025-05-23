/*
 * Copyright (C) 2015 - Michael Zanetti <michael.zanetti@ubuntu.com>
 * Copyright (C) 2020 Brian Douglass
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
import Lomiri.Components 1.3
import Lomiri.Components.Popups 1.3
import QtQuick.Layouts 1.1
import OpenStore 1.0

import "Components" as Components
import "Dialogs" as Dialogs

Page {
    id: appDetailsPage
    anchors.fill: parent

    property var app: null
    property var rating: null
    property var restrictedPermissions: [
        'bluetooth',
        'calendar',
        'contacts',
        'debug',
        'history',
        'music_files',
        'music_files_read',
        'picture_files',
        'picture_files_read',
        'video_files',
        'video_files_read',
    ]

    property bool isTrustedApp: {
        if (app && app.appId) {
            if (app.appId.startsWith('com.ubuntu.') && !app.appId.startsWith('com.ubuntu.developer.')) {
                return true;
            }

            if (app.appId.startsWith('com.canonical.')) {
                return true;
            }

            if (app.appId.startsWith('ubports.')) {
                return true;
            }

            if (app.appId.startsWith('openstore.')) {
                return true;
            }
        }

        return false;
    }

    property bool canAccessUnconfinedLocations: {
        for (var i=0; i<app.hooksCount; ++i) {
            if (includesUnconfinedLocations(app.readPaths(i)))
                return true
            if (includesUnconfinedLocations(app.writePaths(i)))
                return true
        }
        return false
    }

    property bool isUnconfined: {
        for (var i=0; i<app.hooksCount; ++i) {
            if (app.apparmorTemplate(i).indexOf("unconfined") >= 0)
                return true
        }
        return false
    }

    function getNumberShortForm(number) {
        if (number > 999999) {
            return Math.floor(number/1000000) + "M"
        }
        else if (number > 999) {
            return Math.floor(number/1000) + "K"
        }
        else return number + ""
    }

    function appReloaded(pkg) {
        PackagesCache.packageDetailsReady.disconnect(appReloaded);
        app = pkg;
    }

    function reload() {
        PackagesCache.packageDetailsReady.connect(appReloaded);
        PackagesCache.getPackageDetails(app.appId, true);
    }

    header: Components.HeaderBase {
        title: app ? app.name : i18n.tr("App details")
        enabled: !PlatformIntegration.clickInstaller.busy

        trailingActionBar {
            numberOfSlots: 1
            actions: Action {
                iconName: "delete"
                text: i18n.tr("Remove")
                visible: app.installed && app.appId != "openstore.openstore-team"

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
                divider.visible: false

                ListItemLayout {
                    anchors.fill: parent
                    title.text: app.name
                    subtitle.text: app.publisher
                    summary.maximumLineCount: 3
                    summary.text: {
                        var translations = {
                            'app': i18n.tr("App"),
                            'webapp': i18n.tr("Bookmark"),
                            'webapp+': i18n.tr("Web App"),
                        };

                        var types = [];
                        for (var i = 0; i < app.types.length; i++) {
                            if (translations[app.types[i]]) {
                                types.push(translations[app.types[i]]);
                            }
                        }

                        var installedSize = app.installedSize ? '\n' + i18n.tr('Install Size: ') + printSize(app.installedSize) : '';
                        var downloadSize = app.downloadSize ? '\n' + i18n.tr('Download Size: ') + printSize(app.downloadSize) : '';
                        return types.join(', ') + installedSize + downloadSize;
                    }

                    LomiriShape {
                        SlotsLayout.position: SlotsLayout.Leading
                        width: units.gu(12); height: width
                        aspect: LomiriShape.Flat

                        image: Image {
                            height: parent.height
                            width: parent.width
                            sourceSize.width: parent.width
                            sourceSize.height: parent.height
                            source: app ? app.icon : ""
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: easterEggAnimation.start()
                        }
                    }
                }
            }

            // Review
            ListItem {
                height: units.gu(6)
                visible: app.ratings.totalCount > 0

                Row {
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                    spacing: units.gu(5)

                    Components.ReviewItem {
                        reviewIcon: rating === 0 ? "../Assets/thumbup-full.svg" : "../Assets/thumbup.svg"
                        reviewNumber: app.ratings.thumbsUpCount
                        enabled: app.ratings.thumbsUpCount > 0
                    }

                    Components.ReviewItem {
                        reviewIcon: rating === 3 ? "../Assets/happy-full.svg" : "../Assets/happy.svg"
                        reviewNumber: app.ratings.happyCount
                        enabled: app.ratings.happyCount > 0
                    }

                    Components.ReviewItem {
                        reviewIcon: rating === 2 ? "../Assets/neutral-full.svg" : "../Assets/neutral.svg"
                        reviewNumber: app.ratings.neutralCount
                        enabled: app.ratings.neutralCount > 0
                    }

                    Components.ReviewItem {
                        reviewIcon: rating === 1 ? "../Assets/thumbdown-full.svg" : "../Assets/thumbdown.svg"
                        reviewNumber: app.ratings.thumbsDownCount
                        enabled: app.ratings.thumbsDownCount > 0
                    }

                    Components.ReviewItem {
                        reviewIcon: rating === 4 ? "../Assets/buggy-full.svg" : "../Assets/buggy.svg"
                        reviewNumber: app.ratings.buggyCount
                        enabled: app.ratings.buggyCount > 0
                    }
                }
            }

            ListItem {
                visible: app.isLocalVersionSideloaded && app.frameworkSupported
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
                    visible: !PlatformIntegration.clickInstaller.busy && !PackagesCache.updatingCache && app.frameworkSupported

                    Button {
                        Layout.fillWidth: true
                        Layout.maximumWidth: buttonsRow.width > units.gu(60) ? units.gu(24) : buttonsRow.width
                        text: i18n.tr("Open")
                        visible: app.installed && app.containsApp && app.appId!="openstore.openstore-team"
                        color: installUpgradeButton.visible
                            ? theme.name == "Lomiri.Components.Themes.Ambiance"
                                ? LomiriColors.graphite
                                : LomiriColors.ash
                            : theme.palette.normal.positive

                        onClicked: Qt.openUrlExternally(app.appLaunchUrl())
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: app.installed && app.containsApp && app.appId == "openstore.openstore-team" && !app.isLocalVersionSideloaded && !app.updateAvailable
                        horizontalAlignment: Text.AlignHCenter

                        text: "🎉 " + i18n.tr("The OpenStore is installed!") + " ❤️"
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
                            if (isUnconfined && !isTrustedApp && !app.installed) {
                                var popup = PopupUtils.open(unconfinedWarningDialog)
                                popup.accepted.connect(function() {
                                    app.install();
                                });
                            }
                            else if (app.donateUrl && !app.installed) {
                                var popup = PopupUtils.open(donationDialog)
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
                        if (app && !app.installed && !app.frameworkSupported) {
                            pageStack.pop();
                        }
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        maximumValue: app ? app.downloadSize : 0
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
                            enabled: PlatformIntegration.clickInstaller.downloadProgress < app.downloadSize
                        }
                        Rectangle {
                            color: "#cdcdcd"
                            anchors.fill: parent
                            visible: abortBtn.enabled && abortBtn.pressed
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
                    visible: !app.frameworkSupported && !PlatformIntegration.clickInstaller.busy && !PackagesCache.updatingCache

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
                visible: (canAccessUnconfinedLocations || isUnconfined) && !isTrustedApp
                ListItemLayout {
                    anchors.centerIn: parent
                    subtitle.text: {
                        if (isUnconfined) {
                            return i18n.tr("This app has access to restricted parts of the system and all of your data, see below for details.");
                        }

                        return i18n.tr("This app has access to restricted system data, see below for details.");
                    }
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
                    title.wrapMode: Text.WordWrap
                    title.maximumLineCount: 2
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

            Components.ReviewPreview {
                visible: app.frameworkSupported
                reviews: app.reviews

                onRatingUpdated: {
                    appDetailsPage.rating = rating;
                }

                onReviewPosted: {
                    reload();
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
                    subtitle.text: app.maintainer || "None"
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
                visible: app.versionString
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

            Components.LineDivider {}

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
                visible: !!app.translationUrl
                onClicked: Qt.openUrlExternally(app.translationUrl)
                ListItemLayout {
                    anchors.centerIn: parent

                    title.text: i18n.tr("Help translate this app")
                    subtitle.text: app.translationUrl

                    ProgressionSlot {}
                }
            }

            ListItem {
                enabled: !PlatformIntegration.clickInstaller.busy
                onClicked: {
                    bottomEdgeStack.clear()
                    root.showSearch('publisher:' + app.publisher)
                }
                ListItemLayout {
                    anchors.centerIn: parent
                    // TRANSLATORS: This is the button that shows a list of all the packages from the same publisher. %1 is the name of the publisher.
                    title.text: i18n.tr("More from %1").arg(app.publisher)
                    ProgressionSlot {}
                }
            }

            ListItem {
                enabled: !PlatformIntegration.clickInstaller.busy
                onClicked: {
                    bottomEdgeStack.clear()
                    root.showCategory(localCat(app.category), app.category)
                }
                ListItemLayout {
                    anchors.centerIn: parent
                    // FIXME: app.category is not localized.
                    // TRANSLATORS: This is the button that shows a list of all the other packages in the same category. %1 is the name of the category.
                    title.text: i18n.tr("Other apps in %1").arg(localCat(app.category))
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
                    property var permissions: app.permissions(index)
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

                            Icon {
                                SlotsLayout.position: SlotsLayout.Leading
                                width: units.gu(4); height: width
                                name: "security-alert"
                                visible: {
                                    if (apparmorTemplate.indexOf("unconfined") >= 0) {
                                        return true;
                                    }

                                    var length = restrictedPermissions.length;
                                    while(length--) {
                                        if (permissions.indexOf(restrictedPermissions[length]) > -1) {
                                           return true;
                                        }
                                    }

                                    return false;
                                }
                            }

                            title.text: i18n.tr("Permissions")
                            subtitle.maximumLineCount: Number.MAX_VALUE
                            subtitle.wrapMode: Text.WordWrap
                            subtitle.text: {
                                var translations = {
                                    accounts: i18n.tr("Accounts"),
                                    audio: i18n.tr("Audio"),
                                    bluetooth: i18n.tr("Bluetooth"),
                                    calendar: i18n.tr("Calendar"),
                                    camera: i18n.tr("Camera"),
                                    connectivity: i18n.tr("Connectivity"),
                                    contacts: i18n.tr("Contacts"),
                                    content_exchange_source: i18n.tr("Content Exchange Source"),
                                    content_exchange: i18n.tr("Content Exchange"),
                                    debug: i18n.tr("Debug"),
                                    history: i18n.tr("History"),
                                    'in-app-purchases': i18n.tr("In App Purchases"),
                                    'keep-display-on': i18n.tr("Keep Display On"),
                                    location: i18n.tr("Location"),
                                    microphone: i18n.tr("Microphone"),
                                    music_files_read: i18n.tr("Read Music Files"),
                                    music_files: i18n.tr("Music Files"),
                                    networking: i18n.tr("Networking"),
                                    picture_files_read: i18n.tr("Read Picture Files"),
                                    picture_files: i18n.tr("Picture Files"),
                                    'push-notification-client': i18n.tr("Push Notifications"),
                                    sensors: i18n.tr("Sensors"),
                                    usermetrics: i18n.tr("User Metrics"),
                                    video_files_read: i18n.tr("Read Video Files"),
                                    video_files: i18n.tr("Video Files"),
                                    video: i18n.tr("Video"),
                                    webview: i18n.tr("Webview"),
                                };

                                if (apparmorTemplate.indexOf("unconfined") >= 0) {
                                    return '<font color=\"#ED3146\">' + i18n.tr("Full System Access") + '</font>';
                                }

                                if (permissions.length === 0) {
                                    // TRANSLATORS: this will show when an app doesn't need any special permissions
                                    return "<i>" + i18n.tr("none required") + "</i>"
                                }

                                var translated = [];
                                for (var i = 0; i < permissions.length; i++) {
                                    var permission = permissions[i];
                                    var isRestricted = restrictedPermissions.indexOf(permission) > -1;

                                    if (translations[permission]) {
                                        permission = translations[permission];
                                    }

                                    if (isRestricted) {
                                        translated.push('<font color=\"#ED3146\">' + permission + '</font>');
                                    }
                                    else {
                                        translated.push(permission);
                                    }
                                }

                                return translated.join(', ');
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

    Item {
        width: units.gu(6)

        x: parent.width + width
        y: units.gu(7)

        NumberAnimation on x {
            id: easterEggAnimation

            from: parent.width
            to: 0 - width
            duration: 4000

            running: false
            alwaysRunToEnd: true
        }

        Icon {
            width: units.gu(3)
            height: width
            source: '../Assets/t-up.svg'

            anchors {
                top: parent.top
                left: parent.left
            }
        }

        Icon {
            width: units.gu(3)
            height: width
            source: '../Assets/t-up.svg'

            anchors {
                top: parent.top
                right: parent.right
            }
        }
    }



    Dialogs.DonationDialog {
        id: donationDialog
    }

    Dialogs.UnconfinedWarningDialog {
        id: unconfinedWarningDialog
    }

    Dialogs.UninstallDialog {
        id: removeQuestion
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

    function localCat(id) {
        var localName = id;
        for (var i=0; i < categoriesModel.rowCount(); i++) {
            if (categoriesModel.data(categoriesModel.index(i,0),0) === id) {
                localName = categoriesModel.data(categoriesModel.index(i,0),1)
            }
        }

        return localName;
    }
}
