import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import GizmoTweakLib2
import GizmoTweak2

ApplicationWindow {
    id: root

    width: 1600
    height: 900
    minimumWidth: 1024
    minimumHeight: 600
    visible: true
    title: currentFile ? "GizmoTweak2 - " + currentFile : "GizmoTweak2"
    color: Theme.background

    // Window geometry persistence
    Settings {
        id: windowSettings
        category: "Window"
        property alias x: root.x
        property alias y: root.y
        property alias width: root.width
        property alias height: root.height
    }

    property NodeGraph graph: NodeGraph {}
    property bool showGrid: false
    property string currentFile: ""
    property bool showProperties: propertiesDrawer.visible  // Bound to drawer state
    property bool showTimeline: true  // Timeline panel visibility
    property string statusHint: ""  // Hover instructions for status bar

    // Laser output control (laserEngine is injected from C++)
    property bool laserEnabled: false
    property int currentZoneIndex: 0
    property var availableZones: laserEngine ? laserEngine.zones : []

    // Create Input and Output at startup, or load last file if enabled
    Component.onCompleted: {
        // Try to reload last file if option is enabled
        if (recentFiles.reloadLastFileOnStartup && recentFiles.lastFile) {
            if (recentFiles.fileExists(recentFiles.lastFile)) {
                loadGraphFromPath(recentFiles.lastFile)
                return
            }
        }
        // Otherwise create default nodes
        graph.createNode("Input", Qt.point(800, 80))
        graph.createNode("Output", Qt.point(800, 500))
        graph.clearUndoStack()  // Clear undo after creating initial nodes
    }

    // File operations
    function newGraph() {
        graph.clear()
        graph.createNode("Input", Qt.point(800, 80))
        graph.createNode("Output", Qt.point(800, 500))
        graph.clearUndoStack()  // Clear undo after creating initial nodes
        currentFile = ""
    }

    function saveGraph(fileUrl) {
        var json = graph.toJson()
        var jsonString = JSON.stringify(json, null, 2)
        FileIO.writeFile(fileUrl, jsonString)
        currentFile = FileIO.urlToLocalFile(fileUrl)
        recentFiles.addRecentFile(currentFile)
    }

    function loadGraph(fileUrl) {
        var localPath = FileIO.urlToLocalFile(fileUrl)
        loadGraphFromPath(localPath)
    }

    function loadGraphFromPath(filePath) {
        var fileUrl = "file:///" + filePath
        var jsonString = FileIO.readFile(fileUrl)
        if (jsonString) {
            var json = JSON.parse(jsonString)
            graph.fromJson(json)
            graph.clearUndoStack()  // Clear undo after loading
            currentFile = filePath
            recentFiles.addRecentFile(filePath)
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

            Menu {
                id: recentMenu
                title: qsTr("Open &Recent")
                enabled: recentFiles.recentFiles.length > 0

                delegate: StyledMenuItem {}

                background: Rectangle {
                    implicitWidth: 300
                    color: Theme.surface
                    border.color: Theme.border
                    radius: 2
                }

                Instantiator {
                    model: recentFiles.recentFiles
                    delegate: Action {
                        text: recentFiles.displayName(modelData)
                        onTriggered: {
                            if (recentFiles.fileExists(modelData)) {
                                loadGraphFromPath(modelData)
                            } else {
                                recentFiles.removeRecentFile(modelData)
                            }
                        }
                    }
                    onObjectAdded: (index, object) => recentMenu.insertAction(index, object)
                    onObjectRemoved: (index, object) => recentMenu.removeAction(object)
                }

                MenuSeparator {}

                Action {
                    text: qsTr("&Clear Recent Files")
                    onTriggered: recentFiles.clearRecentFiles()
                }
            }

            MenuSeparator {}

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
                text: qsTr("Cu&t")
                shortcut: StandardKey.Cut
                enabled: graph.hasSelection
                onTriggered: graph.cutSelected()
            }

            Action {
                text: qsTr("&Copy")
                shortcut: StandardKey.Copy
                enabled: graph.hasSelection
                onTriggered: {
                    console.log("Copy triggered, hasSelection:", graph.hasSelection)
                    graph.copySelected()
                    console.log("After copy, canPaste:", graph.canPaste)
                }
            }

            Action {
                text: qsTr("&Paste")
                shortcut: StandardKey.Paste
                enabled: graph.canPaste
                onTriggered: {
                    console.log("Paste triggered at position:", canvas.pastePosition)
                    graph.pasteAtPosition(canvas.pastePosition)
                }
            }

            MenuSeparator {}

            Action {
                text: qsTr("Select &All")
                shortcut: StandardKey.SelectAll
                onTriggered: graph.selectAll()
            }

            Action {
                text: qsTr("&Duplicate")
                shortcut: "Ctrl+D"
                enabled: graph.hasSelection
                onTriggered: graph.duplicateSelected()
            }

            MenuSeparator {}

            Action {
                text: qsTr("&Delete")
                shortcut: "Delete"
                enabled: graph.hasSelection || canvas.selectedConnection
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
                text: qsTr("Show &Grid")
                shortcut: "G"
                checkable: true
                checked: showGrid
                onTriggered: showGrid = !showGrid
            }

            Action {
                text: qsTr("Show &Properties")
                shortcut: "P"
                checkable: true
                checked: showProperties
                onTriggered: {
                    if (propertiesDrawer.visible) {
                        propertiesDrawer.close()
                    } else {
                        propertiesDrawer.open()
                    }
                }
            }

            Action {
                text: qsTr("Show &Timeline")
                shortcut: "T"
                checkable: true
                checked: showTimeline
                onTriggered: showTimeline = !showTimeline
            }

            MenuSeparator {}

            Action {
                text: qsTr("Reload Last File on Startup")
                checkable: true
                checked: recentFiles.reloadLastFileOnStartup
                onTriggered: recentFiles.reloadLastFileOnStartup = !recentFiles.reloadLastFileOnStartup
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
                text: qsTr("Zoom to &Fit")
                shortcut: "F"
                onTriggered: canvas.zoomToFit()
            }

            Action {
                text: qsTr("&Reset Zoom")
                shortcut: "Ctrl+0"
                onTriggered: canvas.resetZoom()
            }

            Action {
                text: qsTr("Center View")
                shortcut: "Home"
                onTriggered: canvas.centerView()
            }
        }
    }

    Column {
        anchors.fill: parent

        // Node toolbar for drag & drop creation
        NodeToolbar {
            id: nodeToolbar
            width: parent.width
            zoomAreaActive: canvas.zoomAreaMode
            laserEnabled: root.laserEnabled
            zonesModel: root.availableZones
            currentZoneIndex: root.currentZoneIndex
            onHoverChanged: function(message) { root.statusHint = message }

            onDragStarted: function(nodeType, nodeLabel, mouseX, mouseY) {
                dragProxy.nodeType = nodeType
                dragProxy.nodeLabel = nodeLabel
                dragProxy.x = mouseX - dragProxy.width / 2
                dragProxy.y = mouseY - dragProxy.height / 2
                dragProxy.visible = true
            }

            onDragMoved: function(mouseX, mouseY) {
                dragProxy.x = mouseX - dragProxy.width / 2
                dragProxy.y = mouseY - dragProxy.height / 2
            }

            onDragEnded: {
                // Check if dropped on canvas
                var canvasPos = canvas.mapFromItem(null, dragProxy.x + dragProxy.width / 2, dragProxy.y + dragProxy.height / 2)
                if (canvasPos.x >= 0 && canvasPos.x <= canvas.width && canvasPos.y >= 0 && canvasPos.y <= canvas.height) {
                    // Calculate position in canvas coordinates
                    var contentX = canvas.flickable ? canvas.flickable.contentX : 0
                    var contentY = canvas.flickable ? canvas.flickable.contentY : 0
                    var zoomScale = canvas.zoomScale
                    var nodeX = (canvasPos.x + contentX) / zoomScale
                    var nodeY = (canvasPos.y + contentY) / zoomScale

                    // Snap to grid
                    var gridSize = 20
                    nodeX = Math.round(nodeX / gridSize) * gridSize
                    nodeY = Math.round(nodeY / gridSize) * gridSize

                    graph.createNode(dragProxy.nodeType, Qt.point(nodeX, nodeY))
                }
                dragProxy.visible = false
            }

            // Zoom tool signals
            onZoomInRequested: canvas.zoomIn()
            onZoomOutRequested: canvas.zoomOut()
            onResetZoomRequested: canvas.resetZoom()
            onZoomToFitRequested: canvas.zoomToFit()
            onZoomAreaRequested: {
                if (canvas.zoomAreaMode) {
                    canvas.cancelZoomAreaSelection()
                } else {
                    canvas.startZoomAreaSelection()
                }
            }

            // Laser control signals
            onLaserToggled: function(enabled) {
                root.laserEnabled = enabled
                if (laserEngine) {
                    laserEngine.setLaserEnabled(root.currentZoneIndex, enabled)
                }
            }

            onZoneSelected: function(index) {
                // Disable laser on old zone, then switch
                if (laserEngine && root.laserEnabled) {
                    laserEngine.setLaserEnabled(root.currentZoneIndex, false)
                }
                root.currentZoneIndex = index
                if (laserEngine && root.laserEnabled) {
                    laserEngine.setLaserEnabled(index, true)
                }
            }
        }

        // Vertical splitter: top = canvas/preview, bottom = timeline
        SplitView {
            width: parent.width
            height: parent.height - nodeToolbar.height
            orientation: Qt.Vertical

            handle: Rectangle {
                implicitHeight: Theme.splitterHandleWidth
                implicitWidth: Theme.splitterHandleWidth
                color: SplitHandle.pressed ? Theme.splitterHandlePressed
                     : SplitHandle.hovered ? Theme.splitterHandleHover
                     : Theme.splitterHandle

                // Visual feedback line in center
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width * 0.15
                    height: 2
                    radius: 1
                    color: Qt.lighter(parent.color, 1.3)
                    opacity: SplitHandle.hovered || SplitHandle.pressed ? 1.0 : 0.6
                }
            }

            // Top area: horizontal splitter for canvas + preview
            SplitView {
                SplitView.fillHeight: true
                SplitView.minimumHeight: 200
                orientation: Qt.Horizontal

                handle: Rectangle {
                    implicitHeight: Theme.splitterHandleWidth
                    implicitWidth: Theme.splitterHandleWidth
                    color: SplitHandle.pressed ? Theme.splitterHandlePressed
                         : SplitHandle.hovered ? Theme.splitterHandleHover
                         : Theme.splitterHandle

                    // Visual feedback line in center
                    Rectangle {
                        anchors.centerIn: parent
                        width: 2
                        height: parent.height * 0.15
                        radius: 1
                        color: Qt.lighter(parent.color, 1.3)
                        opacity: SplitHandle.hovered || SplitHandle.pressed ? 1.0 : 0.6
                    }
                }

                // Node canvas (takes remaining space)
                NodeCanvas {
                    id: canvas
                    SplitView.fillWidth: true
                    SplitView.minimumWidth: 400
                    graph: root.graph
                    showGrid: root.showGrid
                    currentTime: previewPanel.currentTime

                    // Open drawer when clicking on a node (if not already open)
                    onNodeClicked: function(node) {
                        if (!propertiesDrawer.opened) {
                            propertiesDrawer.open()
                        }
                    }

                    // Close drawer when clicking on empty canvas
                    onEmptyCanvasClicked: {
                        if (propertiesDrawer.opened) {
                            propertiesDrawer.close()
                        }
                    }

                    // Close drawer when clicking on a connection
                    onConnectionClicked: {
                        if (propertiesDrawer.opened) {
                            propertiesDrawer.close()
                        }
                    }
                }

                // Preview panel on RIGHT (resizable via splitter)
                PreviewPanel {
                    id: previewPanel
                    SplitView.preferredWidth: 350
                    SplitView.minimumWidth: 200
                    SplitView.maximumWidth: 600
                    graph: root.graph
                }
            }

            // Bottom: Timeline panel
            TimelinePanel {
                id: timelinePanel
                visible: root.showTimeline
                SplitView.preferredHeight: root.showTimeline ? 150 : 0
                SplitView.minimumHeight: root.showTimeline ? 80 : 0
                SplitView.maximumHeight: 400
                graph: root.graph
            }
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

            // Graph validation indicator
            Label {
                visible: !graph.isGraphComplete
                text: qsTr("Graph incomplete")
                color: Theme.error
                font.pixelSize: Theme.fontSizeSmall
                font.bold: true

                // Pulsing animation
                SequentialAnimation on opacity {
                    running: !graph.isGraphComplete
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 0.5; duration: 600; easing.type: Easing.InOutSine }
                    NumberAnimation { from: 0.5; to: 1.0; duration: 600; easing.type: Easing.InOutSine }
                }
            }

            // Hover instructions (center)
            Label {
                text: root.statusHint
                color: Theme.textHighlight
                font.pixelSize: Theme.fontSizeSmall
                font.italic: true
            }

            Item { Layout.fillWidth: true }

            Label {
                text: "GizmoTweak2 v" + Qt.application.version
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSmall
            }

            // Laser indicator
            RowLayout {
                spacing: 4

                // Laser status dot
                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: root.laserEnabled ? "#FF0000" : Theme.textMuted

                    // Pulsing glow when laser is on
                    SequentialAnimation on opacity {
                        running: root.laserEnabled
                        loops: Animation.Infinite
                        NumberAnimation { from: 1.0; to: 0.4; duration: 400 }
                        NumberAnimation { from: 0.4; to: 1.0; duration: 400 }
                    }

                    // Reset opacity when laser is off
                    opacity: root.laserEnabled ? 1.0 : 0.5
                }

                Label {
                    text: root.laserEnabled
                        ? qsTr("LASER ON - %1").arg(root.availableZones[root.currentZoneIndex] || "?")
                        : qsTr("Laser off")
                    color: root.laserEnabled ? "#FF4444" : Theme.textMuted
                    font.pixelSize: Theme.fontSizeSmall
                    font.bold: root.laserEnabled
                }
            }
        }
    }

    // Drag proxy - floats over everything during node creation drag
    Rectangle {
        id: dragProxy
        width: 100
        height: 50
        radius: Theme.nodeRadius
        visible: false
        opacity: 0.9

        property string nodeType: ""
        property string nodeLabel: ""

        color: {
            switch (nodeType) {
                case "Gizmo": return Theme.nodeGizmo
                case "Transform": return Theme.nodeTransform
                case "SurfaceFactory": return Theme.nodeSurface
                case "PositionTweak":
                case "ScaleTweak":
                case "RotationTweak":
                case "ColorTweak":
                case "PolarTweak":
                case "WaveTweak":
                case "SqueezeTweak":
                case "SparkleTweak": return Theme.nodeTweak
                case "TimeShift":
                case "Mirror": return Theme.nodeUtility
                default: return Theme.surface
            }
        }
        border.color: Theme.accent
        border.width: 2

        Text {
            anchors.centerIn: parent
            text: dragProxy.nodeLabel
            color: Theme.text
            font.pixelSize: Theme.fontSizeNormal
            font.bold: true
        }
    }

    // Properties drawer (slides in from left)
    PropertiesPanel {
        id: propertiesDrawer
        selectedNode: canvas.selectedNode
        parent: Overlay.overlay
    }

    // Global keyboard shortcuts
    Shortcut {
        sequence: "Escape"
        onActivated: {
            // If drawer is open, close it
            if (propertiesDrawer.opened) {
                propertiesDrawer.close()
            } else {
                // Otherwise, deselect all
                graph.clearSelection()
                canvas.selectedConnection = null
            }
        }
    }
}
