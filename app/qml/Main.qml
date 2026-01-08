import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

ApplicationWindow {
    id: root

    width: 1280
    height: 720
    visible: true
    title: "GizmoTweak2"
    color: Theme.background

    property NodeGraph graph: NodeGraph {}
    property bool showGrid: false

    // Create Input and Output at startup
    Component.onCompleted: {
        graph.createNode("Input", Qt.point(1000, 100))
        graph.createNode("Output", Qt.point(1000, 500))
    }

    menuBar: MenuBar {
        palette.window: Theme.backgroundLight
        palette.windowText: Theme.text
        palette.base: Theme.backgroundLight
        palette.text: Theme.text
        palette.highlight: Theme.menuHighlight
        palette.highlightedText: Theme.text
        palette.button: Theme.backgroundLight
        palette.buttonText: Theme.text

        Menu {
            title: qsTr("&File")
            palette.window: Theme.surface
            palette.windowText: Theme.text
            palette.base: Theme.surface
            palette.text: Theme.text
            palette.highlight: Theme.menuHighlight
            palette.highlightedText: Theme.text
            palette.button: Theme.surface
            palette.buttonText: Theme.text
            palette.mid: Theme.border

            Action {
                text: qsTr("&New")
                shortcut: StandardKey.New
                onTriggered: {
                    graph.clear()
                    graph.createNode("Input", Qt.point(1000, 100))
                    graph.createNode("Output", Qt.point(1000, 500))
                }
            }

            MenuSeparator {}

            Action {
                text: qsTr("&Quit")
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        }

        Menu {
            title: qsTr("&Edit")
            palette.window: Theme.surface
            palette.windowText: Theme.text
            palette.base: Theme.surface
            palette.text: Theme.text
            palette.highlight: Theme.menuHighlight
            palette.highlightedText: Theme.text
            palette.button: Theme.surface
            palette.buttonText: Theme.text
            palette.mid: Theme.border

            Action {
                text: qsTr("&Delete")
                onTriggered: canvas.deleteSelected()
            }
        }

        Menu {
            title: qsTr("&View")
            palette.window: Theme.surface
            palette.windowText: Theme.text
            palette.base: Theme.surface
            palette.text: Theme.text
            palette.highlight: Theme.menuHighlight
            palette.highlightedText: Theme.text
            palette.button: Theme.surface
            palette.buttonText: Theme.text
            palette.mid: Theme.border

            Action {
                text: qsTr("Show Grid")
                checkable: true
                checked: showGrid
                onTriggered: showGrid = !showGrid
            }
        }
    }

    NodeCanvas {
        id: canvas
        anchors.fill: parent
        graph: root.graph
        showGrid: root.showGrid
    }

    // Status bar
    footer: Rectangle {
        height: 24
        color: Theme.backgroundLight

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8

            Label {
                text: qsTr("Nodes: %1").arg(graph.nodeCount)
                color: Theme.text
                font.pixelSize: Theme.fontSizeSmall
            }

            Label {
                text: qsTr("Connections: %1").arg(graph.connectionCount)
                color: Theme.text
                font.pixelSize: Theme.fontSizeSmall
            }

            Item { Layout.fillWidth: true }

            Label {
                text: "GizmoTweak2 v" + Qt.application.version
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSmall
            }
        }
    }
}
