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
    title: {
        var fileName = currentFile ? currentFile : qsTr("Untitled")
        var base = "GizmoTweak2 - " + fileName
        return graph.isModified ? base + " *" : base
    }
    color: Theme.background

    // Handle window close with unsaved changes check
    onClosing: function(close) {
        if (forceClose) {
            close.accepted = true
            return
        }
        if (graph.isModified) {
            close.accepted = false
            checkUnsavedChanges(function() { Qt.quit() })
        }
    }

    // Window geometry persistence
    Settings {
        id: windowSettings
        category: "Window"
        property alias x: root.x
        property alias y: root.y
        property alias width: root.width
        property alias height: root.height
    }

    // Node usage tracking for favorites
    Settings {
        id: nodeUsageSettings
        category: "NodeUsage"
    }

    // Get usage count for a node type
    function getNodeUsageCount(nodeType) {
        return nodeUsageSettings.value(nodeType, 0)
    }

    // Increment usage count and get updated favorites list
    function trackNodeUsage(nodeType) {
        var count = getNodeUsageCount(nodeType)
        nodeUsageSettings.setValue(nodeType, count + 1)
        updateFavoriteNodes()
    }

    // Get list of favorite nodes (top 4 most used)
    property var favoriteNodes: []

    function updateFavoriteNodes() {
        var nodeTypes = ["Gizmo", "Transform", "SurfaceFactory", "Mirror", "TimeShift",
                         "PositionTweak", "ScaleTweak", "RotationTweak", "ColorTweak",
                         "PolarTweak", "WaveTweak", "SqueezeTweak", "SparkleTweak"]
        var usageCounts = []

        for (var i = 0; i < nodeTypes.length; i++) {
            var count = getNodeUsageCount(nodeTypes[i])
            if (count > 0) {
                usageCounts.push({ type: nodeTypes[i], count: count })
            }
        }

        // Sort by count descending
        usageCounts.sort(function(a, b) { return b.count - a.count })

        // Take top 4
        favoriteNodes = usageCounts.slice(0, 4).map(function(item) { return item.type })
    }

    property NodeGraph graph: NodeGraph {}
    property string currentFile: ""
    property bool showProperties: true  // Left panel visibility
    property bool showTimeline: true  // Timeline panel visibility
    property string statusHint: ""  // Hover instructions for status bar

    // Laser output control (laserEngine is injected from C++)
    property bool laserEnabled: false
    property int currentZoneIndex: 0
    property var availableZones: laserEngine ? laserEngine.zones : []

    // Connect to laser engine error signals
    Connections {
        target: laserEngine
        function onErrorOccurred(message) {
            toast.showError(qsTr("Laser: ") + message)
        }
    }

    // Create Input and Output at startup, or load last file if enabled
    Component.onCompleted: {
        // Initialize favorite nodes
        updateFavoriteNodes()

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
        selectInputNode()
    }

    // Select the InputNode and show its properties panel
    function selectInputNode() {
        for (var i = 0; i < graph.rowCount(); i++) {
            var node = graph.nodeAt(i)
            if (node && node.type === "Input") {
                graph.clearSelection()
                node.selected = true
                canvas.selectedNode = node
                return
            }
        }
    }

    // File operations
    function newGraph() {
        graph.clear()
        graph.createNode("Input", Qt.point(800, 80))
        graph.createNode("Output", Qt.point(800, 500))
        graph.clearUndoStack()  // Clear undo after creating initial nodes
        graph.setClean()  // Mark as saved (fresh document)
        currentFile = ""
        selectInputNode()
    }

    function saveGraph(fileUrl) {
        try {
            var json = graph.toJson()
            var jsonString = JSON.stringify(json, null, 2)
            if (FileIO.writeFile(fileUrl, jsonString)) {
                currentFile = FileIO.urlToLocalFile(fileUrl)
                recentFiles.addRecentFile(currentFile)
                graph.setClean()  // Mark as saved
                toast.showSuccess(qsTr("Graph saved successfully"))
            } else {
                toast.showError(qsTr("Failed to save file"))
            }
        } catch (e) {
            toast.showError(qsTr("Error saving file: ") + e.message)
        }
    }

    function loadGraph(fileUrl) {
        var localPath = FileIO.urlToLocalFile(fileUrl)
        loadGraphFromPath(localPath)
    }

    function loadGraphFromPath(filePath) {
        try {
            var fileUrl = "file:///" + filePath
            var jsonString = FileIO.readFile(fileUrl)
            if (jsonString) {
                var json = JSON.parse(jsonString)
                if (graph.fromJson(json)) {
                    graph.clearUndoStack()  // Clear undo after loading
                    graph.setClean()  // Mark as saved
                    currentFile = filePath
                    recentFiles.addRecentFile(filePath)
                    selectInputNode()
                    toast.showSuccess(qsTr("Graph loaded successfully"))
                } else {
                    toast.showError(qsTr("Failed to parse graph file"))
                }
            } else {
                toast.showError(qsTr("Failed to read file: ") + filePath)
            }
        } catch (e) {
            toast.showError(qsTr("Error loading file: ") + e.message)
        }
    }

    function loadTemplate(templateName) {
        try {
            // Templates are bundled in qrc:/resources/templates/
            var templateUrl = "qrc:/resources/templates/" + templateName
            var jsonString = FileIO.readFile(templateUrl)
            if (jsonString) {
                var json = JSON.parse(jsonString)
                if (graph.fromJson(json)) {
                    graph.clearUndoStack()
                    graph.setClean()
                    currentFile = ""  // Template creates untitled document
                    toast.showSuccess(qsTr("Template loaded: ") + templateName.replace(".gt2", ""))
                } else {
                    toast.showError(qsTr("Failed to parse template"))
                }
            } else {
                toast.showError(qsTr("Template not found: ") + templateName)
            }
        } catch (e) {
            toast.showError(qsTr("Error loading template: ") + e.message)
        }
    }

    // Pending action after save prompt
    property var pendingAction: null
    property bool forceClose: false  // Flag to bypass unsaved changes check

    // Check for unsaved changes and show prompt if needed
    function checkUnsavedChanges(action) {
        forceClose = false  // Reset force close flag
        if (graph.isModified) {
            pendingAction = action
            savePromptDialog.open()
        } else {
            action()
        }
    }

    // Save prompt dialog
    Dialog {
        id: savePromptDialog
        title: qsTr("Unsaved Changes")
        modal: true
        anchors.centerIn: parent

        Label {
            text: qsTr("Do you want to save the changes to your document?")
            color: Theme.text
        }

        background: Rectangle {
            color: Theme.surface
            border.color: Theme.border
            radius: 4
        }

        footer: Rectangle {
            color: "transparent"
            implicitHeight: footerRow.implicitHeight + 16
            implicitWidth: footerRow.implicitWidth + 16

            RowLayout {
                id: footerRow
                anchors.centerIn: parent
                anchors.margins: 8
                spacing: 8

                Button {
                    text: qsTr("Save")
                    onClicked: {
                        savePromptDialog.close()
                        // Save first, then do the action
                        if (currentFile) {
                            saveGraph("file:///" + currentFile)
                            if (pendingAction) pendingAction()
                        } else {
                            // Need to use Save As dialog
                            saveDialogForPending.open()
                        }
                        pendingAction = null
                    }
                }
                Button {
                    text: qsTr("Discard")
                    onClicked: {
                        savePromptDialog.close()
                        // Discard changes, just do the action
                        if (pendingAction) {
                            var action = pendingAction
                            pendingAction = null
                            // For quit actions, set forceClose flag then close window
                            forceClose = true
                            action()
                        }
                    }
                }
                Button {
                    text: qsTr("Cancel")
                    onClicked: {
                        savePromptDialog.close()
                        pendingAction = null
                    }
                }
            }
        }
    }

    // Save dialog for pending action (when no current file)
    FileDialog {
        id: saveDialogForPending
        title: qsTr("Save Graph")
        nameFilters: ["GizmoTweak2 files (*.gt2)", "All files (*)"]
        fileMode: FileDialog.SaveFile
        defaultSuffix: "gt2"
        onAccepted: {
            saveGraph(selectedFile)
            if (pendingAction) pendingAction()
            pendingAction = null
        }
        onRejected: {
            pendingAction = null
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

    // ILDA file dialogs
    FileDialog {
        id: importIldaDialog
        title: qsTr("Import ILDA Patterns")
        nameFilters: [qsTr("ILDA files (*.ild *.ilda)"), qsTr("All files (*)")]
        onAccepted: {
            var inputNode = findInputNode()
            if (inputNode) {
                var localPath = FileIO.urlToLocalFile(selectedFile)
                if (inputNode.loadIldaFile(localPath)) {
                    graph.markAsModified()
                    toast.showSuccess(qsTr("ILDA patterns imported successfully"))
                } else {
                    toast.showError(qsTr("Failed to import ILDA file"))
                }
            } else {
                toast.showError(qsTr("No Input node found in graph"))
            }
        }
    }

    FileDialog {
        id: exportIldaDialog
        title: qsTr("Export ILDA Patterns")
        nameFilters: [qsTr("ILDA files (*.ild)"), qsTr("All files (*)")]
        fileMode: FileDialog.SaveFile
        defaultSuffix: "ild"
        onAccepted: {
            var inputNode = findInputNode()
            if (inputNode) {
                var localPath = FileIO.urlToLocalFile(selectedFile)
                if (inputNode.saveIldaFile(localPath)) {
                    toast.showSuccess(qsTr("ILDA patterns exported successfully"))
                } else {
                    toast.showError(qsTr("Failed to export ILDA file"))
                }
            } else {
                toast.showError(qsTr("No Input node found in graph"))
            }
        }
    }

    // Keyboard shortcuts help dialog
    KeyboardShortcutsDialog {
        id: keyboardShortcutsDialog
        anchors.centerIn: parent
    }

    // Find the Input node in the graph
    function findInputNode() {
        for (var i = 0; i < graph.nodeCount; i++) {
            var node = graph.nodeAt(i)
            if (node && node.type === "Input") {
                return node
            }
        }
        return null
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
                onTriggered: checkUnsavedChanges(function() { newGraph() })
            }

            Action {
                text: qsTr("&Open...")
                shortcut: StandardKey.Open
                onTriggered: checkUnsavedChanges(function() { openDialog.open() })
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
                            var filePath = modelData
                            if (recentFiles.fileExists(filePath)) {
                                checkUnsavedChanges(function() { loadGraphFromPath(filePath) })
                            } else {
                                recentFiles.removeRecentFile(filePath)
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

            Menu {
                id: templatesMenu
                title: qsTr("New from &Template")

                delegate: StyledMenuItem {}

                background: Rectangle {
                    implicitWidth: 250
                    color: Theme.surface
                    border.color: Theme.border
                    radius: 2
                }

                Action {
                    text: qsTr("Basic Chain")
                    onTriggered: checkUnsavedChanges(function() { loadTemplate("basic_chain.gt2") })
                }

                Action {
                    text: qsTr("Rotating Circle")
                    onTriggered: checkUnsavedChanges(function() { loadTemplate("rotating_circle.gt2") })
                }

                Action {
                    text: qsTr("Wave Effect")
                    onTriggered: checkUnsavedChanges(function() { loadTemplate("wave_effect.gt2") })
                }

                Action {
                    text: qsTr("Multi Gizmo")
                    onTriggered: checkUnsavedChanges(function() { loadTemplate("multi_gizmo.gt2") })
                }

                Action {
                    text: qsTr("Mirror Effect")
                    onTriggered: checkUnsavedChanges(function() { loadTemplate("mirror_effect.gt2") })
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
                text: qsTr("&Import ILDA...")
                shortcut: "Ctrl+I"
                onTriggered: importIldaDialog.open()
            }

            Action {
                text: qsTr("&Export ILDA...")
                shortcut: "Ctrl+E"
                onTriggered: exportIldaDialog.open()
            }

            MenuSeparator {}

            Action {
                text: qsTr("&Quit")
                shortcut: StandardKey.Quit
                onTriggered: checkUnsavedChanges(function() { Qt.quit() })
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
                    graph.copySelected()
                }
            }

            Action {
                text: qsTr("&Paste")
                shortcut: StandardKey.Paste
                enabled: graph.canPaste
                onTriggered: {
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
                text: qsTr("Deselect All")
                shortcut: "Ctrl+Shift+A"
                onTriggered: graph.clearSelection()
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
                text: qsTr("Show &Properties")
                shortcut: "P"
                checkable: true
                checked: showProperties
                onTriggered: showProperties = !showProperties
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
            favoriteNodeTypes: root.favoriteNodes

            // Transport state from PreviewPanel
            isPlaying: previewPanel.isPlaying
            isLooping: previewPanel.isLooping
            playLocatorMs: previewPanel.playLocatorMs
            animationDurationMs: previewPanel.animationDurationMs

            onHoverChanged: function(message) { root.statusHint = message }

            // Transport signal wiring
            onPlayToggled: previewPanel.togglePlayback()
            onRewindRequested: previewPanel.rewind()
            onStopRequested: previewPanel.stop()
            onLoopToggled: previewPanel.toggleLoop()
            onScrubChanged: function(timeMs) {
                previewPanel.playLocatorMs = timeMs
                previewPanel.playLocatorChanged(timeMs)
            }

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
                    trackNodeUsage(dragProxy.nodeType)
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

                // Properties panel on LEFT (collapsible)
                PropertiesPanel {
                    id: propertiesDrawer
                    visible: root.showProperties
                    SplitView.preferredWidth: root.showProperties ? 400 : 0
                    SplitView.minimumWidth: root.showProperties ? 320 : 0
                    SplitView.maximumWidth: 500
                    selectedNode: canvas.selectedNode
                    graph: root.graph

                    onCollapseRequested: root.showProperties = false
                }

                // Node canvas (takes remaining space)
                NodeCanvas {
                    id: canvas
                    SplitView.fillWidth: true
                    SplitView.minimumWidth: 400
                    graph: root.graph
                    showGrid: previewPanel.showGrid
                    currentTime: previewPanel.currentTime

                    // Toggle panel on double-click
                    onNodeDoubleClicked: function(node) {
                        if (!root.showProperties) {
                            root.showProperties = true
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
                    currentZoneIndex: root.currentZoneIndex

                    // Keyframe editing context from timeline
                    currentTrack: timelinePanel.currentTrack
                    currentKeyFrameMs: timelinePanel.currentKeyFrameMs
                    scrollLocatorMs: timelinePanel.scrollLocatorMs
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
                playLocatorMs: previewPanel.playLocatorMs
                animationDurationMs: previewPanel.animationDurationMs
            }

        }
    }

    // Sync timeline scroll locator with preview (playLocatorMs is already bound)
    Connections {
        target: previewPanel
        function onPlayLocatorChanged(timeMs) {
            timelinePanel.scrollLocatorMs = timeMs
        }
        function onRewindRequested() {
            timelinePanel.scrollLocatorMs = 0
        }
        function onStopRequested() {
            timelinePanel.scrollLocatorMs = 0
        }
    }

    // Sync preview with timeline (when clicking on timeline)
    Connections {
        target: timelinePanel
        function onPlayLocatorChanged(timeMs) {
            previewPanel.playLocatorMs = timeMs
        }
        function onKeyFrameDataModified() {
            graph.markAsModified()
        }
    }

    // Keyframe edits from preview panel
    Connections {
        target: previewPanel
        function onKeyFrameDataModified() {
            graph.markAsModified()
        }
    }

    // Sync preview with Input node settings
    Connections {
        target: root.graph
        function onNodeAdded() {
            root.syncWithInputNode()
        }
    }

    function syncWithInputNode() {
        for (var i = 0; i < root.graph.nodeCount; i++) {
            var node = root.graph.nodeAt(i)
            if (node && node.type === "Input") {
                previewPanel.animationDurationMs = node.duration
                timelinePanel.animationDurationMs = node.duration
                timelinePanel.beatsPerMeasure = node.beatsPerMeasure
                timelinePanel.measureCount = node.measures
                return
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

    // Global keyboard shortcuts
    Shortcut {
        sequence: "Escape"
        onActivated: {
            // If properties panel is open, close it
            if (root.showProperties) {
                root.showProperties = false
            } else {
                // Otherwise, deselect all
                graph.clearSelection()
                canvas.selectedConnection = null
            }
        }
    }

    // Additional delete shortcut (Backspace)
    Shortcut {
        sequence: "Backspace"
        onActivated: {
            if (graph.hasSelection || canvas.selectedConnection) {
                canvas.deleteSelected()
            }
        }
    }

    // Additional redo shortcut (Ctrl+Shift+Z)
    Shortcut {
        sequence: "Ctrl+Shift+Z"
        onActivated: {
            if (graph.canRedo) graph.redo()
        }
    }

    // Play/Pause preview (Space)
    Shortcut {
        sequence: "Space"
        onActivated: {
            previewPanel.togglePlayback()
        }
    }

    // Toggle loop mode (L)
    Shortcut {
        sequence: "L"
        onActivated: {
            previewPanel.toggleLoop()
        }
    }

    // Show keyboard shortcuts dialog (F2)
    Shortcut {
        sequence: "F2"
        onActivated: {
            keyboardShortcutsDialog.open()
        }
    }

    // Previous keyframe ([)
    Shortcut {
        sequence: "["
        onActivated: {
            if (timelinePanel.currentTrack && timelinePanel.currentKeyFrameMs >= 0) {
                var times = timelinePanel.currentTrack.keyFrameTimes()
                for (var i = times.length - 1; i >= 0; i--) {
                    if (times[i] < timelinePanel.currentKeyFrameMs) {
                        timelinePanel.currentKeyFrameMs = times[i]
                        timelinePanel.scrollLocatorMs = times[i]
                        break
                    }
                }
            }
        }
    }

    // Next keyframe (])
    Shortcut {
        sequence: "]"
        onActivated: {
            if (timelinePanel.currentTrack && timelinePanel.currentKeyFrameMs >= 0) {
                var times = timelinePanel.currentTrack.keyFrameTimes()
                for (var i = 0; i < times.length; i++) {
                    if (times[i] > timelinePanel.currentKeyFrameMs) {
                        timelinePanel.currentKeyFrameMs = times[i]
                        timelinePanel.scrollLocatorMs = times[i]
                        break
                    }
                }
            }
        }
    }

    // Alternative previous keyframe (,)
    Shortcut {
        sequence: ","
        onActivated: {
            if (timelinePanel.currentTrack && timelinePanel.currentKeyFrameMs >= 0) {
                var times = timelinePanel.currentTrack.keyFrameTimes()
                for (var i = times.length - 1; i >= 0; i--) {
                    if (times[i] < timelinePanel.currentKeyFrameMs) {
                        timelinePanel.currentKeyFrameMs = times[i]
                        timelinePanel.scrollLocatorMs = times[i]
                        break
                    }
                }
            }
        }
    }

    // Alternative next keyframe (.)
    Shortcut {
        sequence: "."
        onActivated: {
            if (timelinePanel.currentTrack && timelinePanel.currentKeyFrameMs >= 0) {
                var times = timelinePanel.currentTrack.keyFrameTimes()
                for (var i = 0; i < times.length; i++) {
                    if (times[i] > timelinePanel.currentKeyFrameMs) {
                        timelinePanel.currentKeyFrameMs = times[i]
                        timelinePanel.scrollLocatorMs = times[i]
                        break
                    }
                }
            }
        }
    }

    // Toast notification component
    Toast {
        id: toast
        parent: Overlay.overlay
    }
}
