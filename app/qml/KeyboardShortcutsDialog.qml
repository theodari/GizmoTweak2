import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * KeyboardShortcutsDialog - QML Implementation
 *
 * Displays all keyboard shortcuts organized by category.
 * Based on Paint Alchemy's implementation.
 */
Dialog {
    id: root

    title: qsTr("Keyboard Shortcuts")
    modal: false
    width: 600
    height: 500
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    background: Rectangle {
        color: Theme.surface
        border.color: Theme.accentBright
        border.width: 1
        radius: 4
    }

    // Shortcut data organized by category
    property var shortcutCategories: [
        {
            name: qsTr("File"),
            shortcuts: [
                { key: "Ctrl+N", action: qsTr("New project") },
                { key: "Ctrl+O", action: qsTr("Open project") },
                { key: "Ctrl+S", action: qsTr("Save project") },
                { key: "Ctrl+Shift+S", action: qsTr("Save project as") },
                { key: "Ctrl+I", action: qsTr("Import ILDA patterns") },
                { key: "Ctrl+E", action: qsTr("Export ILDA patterns") }
            ]
        },
        {
            name: qsTr("Edit"),
            shortcuts: [
                { key: "Ctrl+Z", action: qsTr("Undo") },
                { key: "Ctrl+Y / Ctrl+Shift+Z", action: qsTr("Redo") },
                { key: "Ctrl+X", action: qsTr("Cut") },
                { key: "Ctrl+C", action: qsTr("Copy") },
                { key: "Ctrl+V", action: qsTr("Paste") },
                { key: "Ctrl+D", action: qsTr("Duplicate") },
                { key: "Del / Backspace", action: qsTr("Delete selected") }
            ]
        },
        {
            name: qsTr("Selection"),
            shortcuts: [
                { key: "Ctrl+A", action: qsTr("Select all nodes") },
                { key: "Ctrl+Shift+A", action: qsTr("Deselect all") }
            ]
        },
        {
            name: qsTr("Playback"),
            shortcuts: [
                { key: "Space", action: qsTr("Play / Pause") },
                { key: "L", action: qsTr("Toggle loop mode") }
            ]
        },
        {
            name: qsTr("Timeline Navigation"),
            shortcuts: [
                { key: "[", action: qsTr("Previous keyframe") },
                { key: "]", action: qsTr("Next keyframe") },
                { key: ",", action: qsTr("Step backward (100ms)") },
                { key: ".", action: qsTr("Step forward (100ms)") }
            ]
        },
        {
            name: qsTr("Mouse Modifiers"),
            shortcuts: [
                { key: "Shift + Drag keyframe", action: qsTr("Snap to beat grid") },
                { key: "Ctrl + Drag node", action: qsTr("Constrain to axis") },
                { key: "Shift + Click node", action: qsTr("Add/remove from selection") }
            ]
        },
        {
            name: qsTr("Mouse Wheel"),
            shortcuts: [
                { key: "Wheel", action: qsTr("Zoom canvas in/out") },
                { key: "Ctrl + Wheel", action: qsTr("Zoom timeline in/out") }
            ]
        },
        {
            name: qsTr("Help"),
            shortcuts: [
                { key: "F2", action: qsTr("Show this dialog") }
            ]
        }
    ]

    contentItem: ColumnLayout {
        spacing: 8

        // Search field
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: qsTr("Search:")
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSmall
            }

            TextField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: qsTr("Type to filter shortcuts...")
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.text
                placeholderTextColor: Theme.textMuted

                background: Rectangle {
                    color: Theme.backgroundLight
                    border.color: searchField.activeFocus ? Theme.accentBright : Theme.border
                    border.width: 1
                    radius: 4
                }
            }
        }

        // Shortcuts list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: shortcutsList
                spacing: 16

                model: root.shortcutCategories

                delegate: Column {
                    width: shortcutsList.width - 20
                    spacing: 4
                    visible: categoryVisible(modelData)

                    // Category header
                    Rectangle {
                        width: parent.width
                        height: 28
                        color: Theme.backgroundLight
                        radius: 4

                        Label {
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.name
                            color: Theme.accentBright
                            font.pixelSize: Theme.fontSizeNormal
                            font.bold: true
                        }
                    }

                    // Shortcuts in this category
                    Repeater {
                        model: modelData.shortcuts

                        delegate: Rectangle {
                            width: parent.width
                            height: 24
                            color: index % 2 === 0 ? Theme.surface : Theme.backgroundLight
                            radius: 2
                            visible: shortcutVisible(modelData)

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12

                                // Shortcut key
                                Rectangle {
                                    Layout.preferredWidth: 160
                                    Layout.preferredHeight: 20
                                    color: Theme.background
                                    border.color: Theme.border
                                    border.width: 1
                                    radius: 3

                                    Label {
                                        anchors.centerIn: parent
                                        text: modelData.key
                                        color: Theme.text
                                        font.pixelSize: Theme.fontSizeSmall
                                        font.family: "Consolas"
                                    }
                                }

                                // Action description
                                Label {
                                    Layout.fillWidth: true
                                    text: modelData.action
                                    color: Theme.textMuted
                                    font.pixelSize: Theme.fontSizeSmall
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    footer: DialogButtonBox {
        background: Rectangle {
            color: Theme.backgroundLight
        }

        Button {
            text: qsTr("Close")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole

            contentItem: Text {
                text: parent.text
                color: Theme.text
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                implicitWidth: 80
                implicitHeight: 28
                color: parent.pressed ? Theme.surfacePressed : (parent.hovered ? Theme.surfaceHover : Theme.surface)
                border.color: Theme.accentBright
                border.width: 1
                radius: 4
            }
        }
    }

    // Helper functions for filtering
    function categoryVisible(category) {
        if (searchField.text === "") return true
        var searchLower = searchField.text.toLowerCase()
        if (category.name.toLowerCase().indexOf(searchLower) >= 0) return true
        for (var i = 0; i < category.shortcuts.length; i++) {
            if (shortcutMatchesSearch(category.shortcuts[i], searchLower)) return true
        }
        return false
    }

    function shortcutVisible(shortcut) {
        if (searchField.text === "") return true
        return shortcutMatchesSearch(shortcut, searchField.text.toLowerCase())
    }

    function shortcutMatchesSearch(shortcut, searchLower) {
        return shortcut.key.toLowerCase().indexOf(searchLower) >= 0 ||
               shortcut.action.toLowerCase().indexOf(searchLower) >= 0
    }
}
