import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import GizmoTweakLib2
import GizmoTweak2

ApplicationWindow {
    id: root

    width: 1280
    height: 720
    visible: true
    title: currentFile ? "GizmoTweak2 - " + currentFile : "GizmoTweak2"
    color: Theme.background

    property NodeGraph graph: NodeGraph {}
    property bool showGrid: false
    property string currentFile: ""
    property bool showProperties: true

    // Create Input and Output at startup
    Component.onCompleted: {
        graph.createNode("Input", Qt.point(1000, 100))
        graph.createNode("Output", Qt.point(1000, 500))
        graph.clearUndoStack()  // Clear undo after creating initial nodes
    }

    // File operations
    function newGraph() {
        graph.clear()
        graph.createNode("Input", Qt.point(1000, 100))
        graph.createNode("Output", Qt.point(1000, 500))
        graph.clearUndoStack()  // Clear undo after creating initial nodes
        currentFile = ""
    }

    function saveGraph(fileUrl) {
        var json = graph.toJson()
        var jsonString = JSON.stringify(json, null, 2)
        FileIO.writeFile(fileUrl, jsonString)
        currentFile = FileIO.urlToLocalFile(fileUrl)
    }

    function loadGraph(fileUrl) {
        var jsonString = FileIO.readFile(fileUrl)
        if (jsonString) {
            var json = JSON.parse(jsonString)
            graph.fromJson(json)
            graph.clearUndoStack()  // Clear undo after loading
            currentFile = FileIO.urlToLocalFile(fileUrl)
        }
    }

    // File dialogs
    FileDialog {
        id: openDialog
        title: qsTr("Open Graph")
        nameFilters: [qsTr("GizmoTweak files (*.gt2)"), qsTr("All files (*)")]
        onAccepted: loadGraph(selectedFile)
    }

    FileDialog {
        id: saveDialog
        title: qsTr("Save Graph")
        nameFilters: [qsTr("GizmoTweak files (*.gt2)"), qsTr("All files (*)")]
        fileMode: FileDialog.SaveFile
        defaultSuffix: "gt2"
        onAccepted: saveGraph(selectedFile)
    }

    menuBar: MenuBar {
        palette.window: Theme.backgroundLight
        palette.windowText: Theme.text
        palette.base: Theme.backgroundLight
        palette.text: Theme.text
        palette.highlight: Theme.menuHighlight
        palette.highlightedText: Theme.textOnHighlight
        palette.button: Theme.backgroundLight
        palette.buttonText: Theme.text

        Menu {
            title: qsTr("&File")
            delegate: StyledMenuItem {}

            background: Rectangle {
                implicitWidth: 200
                color: Theme.surface
                border.color: Theme.border
                radius: 2
            }

            Action {
                text: qsTr("&New")
                shortcut: StandardKey.New
                onTriggered: newGraph()
            }

            Action {
                text: qsTr("&Open...")
                shortcut: StandardKey.Open
                onTriggered: openDialog.open()
            }

            Action {
                text: qsTr("&Save")
                shortcut: StandardKey.Save
                onTriggered: {
                    if (currentFile) {
                        saveGraph("file:///" + currentFile)
                    } else {
                        saveDialog.open()
                    }
                }
            }

            Action {
                text: qsTr("Save &As...")
                shortcut: StandardKey.SaveAs
                onTriggered: saveDialog.open()
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
            delegate: StyledMenuItem {}

            background: Rectangle {
                implicitWidth: 200
                color: Theme.surface
                border.color: Theme.border
                radius: 2
            }

            Action {
                text: graph.undoText ? qsTr("&Undo %1").arg(graph.undoText) : qsTr("&Undo")
                shortcut: StandardKey.Undo
                enabled: graph.canUndo
                onTriggered: graph.undo()
            }

            Action {
                text: graph.redoText ? qsTr("&Redo %1").arg(graph.redoText) : qsTr("&Redo")
                shortcut: StandardKey.Redo
                enabled: graph.canRedo
                onTriggered: graph.redo()
            }

            MenuSeparator {}

            Action {
                text: qsTr("&Delete")
                onTriggered: canvas.deleteSelected()
            }
        }

        Menu {
            title: qsTr("&View")
            delegate: StyledMenuItem {}

            background: Rectangle {
                implicitWidth: 200
                color: Theme.surface
                border.color: Theme.border
                radius: 2
            }

            Action {
                text: qsTr("Show Grid")
                checkable: true
                checked: showGrid
                onTriggered: showGrid = !showGrid
            }

            Action {
                text: qsTr("Show Properties")
                checkable: true
                checked: showProperties
                onTriggered: showProperties = !showProperties
            }

            MenuSeparator {}

            Action {
                text: qsTr("Zoom In")
                shortcut: StandardKey.ZoomIn
                onTriggered: canvas.zoomIn()
            }

            Action {
                text: qsTr("Zoom Out")
                shortcut: StandardKey.ZoomOut
                onTriggered: canvas.zoomOut()
            }

            Action {
                text: qsTr("Reset Zoom")
                shortcut: "Ctrl+0"
                onTriggered: canvas.resetZoom()
            }
        }
    }

    Row {
        anchors.fill: parent

        NodeCanvas {
            id: canvas
            width: parent.width - (propertiesPanel.visible ? propertiesPanel.width : 0)
            height: parent.height
            graph: root.graph
            showGrid: root.showGrid
        }

        PropertiesPanel {
            id: propertiesPanel
            width: 250
            height: parent.height
            visible: showProperties && canvas.selectedNode !== null
            selectedNode: canvas.selectedNode
        }
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

            Label {
                text: qsTr("Zoom: %1%").arg(Math.round(canvas.zoomScale * 100))
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
