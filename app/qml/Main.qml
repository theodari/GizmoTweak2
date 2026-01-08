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

    // Créer Input et Output au démarrage
    Component.onCompleted: {
        graph.createNode("Input", Qt.point(1000, 100))
        graph.createNode("Output", Qt.point(1000, 500))
    }

    // Common menu palette
    property var menuPalette: ({
        window: Theme.surface,
        windowText: Theme.text,
        base: Theme.surface,
        text: Theme.text,
        highlight: Theme.menuHighlight,
        highlightedText: Theme.text,
        button: Theme.surface,
        buttonText: Theme.text,
        mid: Theme.border
    })

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
            title: qsTr("&Fichier")
            palette: root.menuPalette

            Action {
                text: qsTr("&Nouveau")
                shortcut: StandardKey.New
                onTriggered: {
                    graph.clear()
                    graph.createNode("Input", Qt.point(1000, 100))
                    graph.createNode("Output", Qt.point(1000, 500))
                }
            }

            MenuSeparator {}

            Action {
                text: qsTr("&Quitter")
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        }

        Menu {
            title: qsTr("&Edition")
            palette: root.menuPalette

            Action {
                text: qsTr("&Supprimer")
                shortcut: StandardKey.Delete
                onTriggered: {
                    var selected = graph.selectedNodes()
                    for (var i = 0; i < selected.length; ++i) {
                        // Ne pas supprimer Input/Output
                        if (selected[i].type !== "Input" && selected[i].type !== "Output") {
                            graph.removeNode(selected[i].uuid)
                        }
                    }
                }
            }
        }

        Menu {
            title: qsTr("&Affichage")
            palette: root.menuPalette

            Action {
                text: qsTr("Afficher la grille")
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
                text: qsTr("Connexions: %1").arg(graph.connectionCount)
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
