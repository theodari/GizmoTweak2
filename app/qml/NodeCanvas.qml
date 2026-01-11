import QtQuick
import QtQuick.Controls
import QtQuick.Shapes
import GizmoTweakLib2
import GizmoTweak2

Item {
    id: root

    required property NodeGraph graph
    property bool showGrid: false

    // Animation time for previews (0.0 to 1.0)
    property real currentTime: 0.0

    // Expose flickable for external coordinate calculations
    property alias flickable: flickable

    // Canvas properties - 2x the typical visible area
    property real canvasWidth: 2400
    property real canvasHeight: 1600
    property real gridSize: 20

    // Zoom properties
    property real zoomScale: 1.0
    property real minZoom: 0.5  // Allows seeing entire canvas
    property real maxZoom: 2.0  // 2x default zoom maximum

    // Zoom area selection mode
    property bool zoomAreaMode: false
    property point zoomAreaStart: Qt.point(0, 0)
    property point zoomAreaEnd: Qt.point(0, 0)
    property bool isSelectingZoomArea: false

    function zoomIn() {
        zoomScale = Math.min(maxZoom, zoomScale * 1.2)
    }

    function zoomOut() {
        zoomScale = Math.max(minZoom, zoomScale / 1.2)
    }

    function resetZoom() {
        zoomScale = 1.0
    }

    function centerView() {
        // Center on all nodes without changing zoom
        if (graph.nodeCount === 0) return

        var minX = Infinity, minY = Infinity
        var maxX = -Infinity, maxY = -Infinity

        for (var i = 0; i < nodesRepeater.count; i++) {
            var nodeItem = nodesRepeater.itemAt(i)
            if (!nodeItem) continue

            minX = Math.min(minX, nodeItem.x)
            minY = Math.min(minY, nodeItem.y)
            maxX = Math.max(maxX, nodeItem.x + nodeItem.width)
            maxY = Math.max(maxY, nodeItem.y + nodeItem.height)
        }

        if (minX === Infinity) return

        // Calculate center of nodes
        var centerX = (minX + maxX) / 2
        var centerY = (minY + maxY) / 2

        // Center the view on nodes
        flickable.contentX = centerX * zoomScale - flickable.width / 2
        flickable.contentY = centerY * zoomScale - flickable.height / 2
    }

    function zoomToFit() {
        // Find bounding box of all nodes
        if (graph.nodeCount === 0) return

        var minX = Infinity, minY = Infinity
        var maxX = -Infinity, maxY = -Infinity

        for (var i = 0; i < nodesRepeater.count; i++) {
            var nodeItem = nodesRepeater.itemAt(i)
            if (!nodeItem) continue

            minX = Math.min(minX, nodeItem.x)
            minY = Math.min(minY, nodeItem.y)
            maxX = Math.max(maxX, nodeItem.x + nodeItem.width)
            maxY = Math.max(maxY, nodeItem.y + nodeItem.height)
        }

        if (minX === Infinity) return

        // Add padding
        var padding = 50
        minX -= padding
        minY -= padding
        maxX += padding
        maxY += padding

        var contentW = maxX - minX
        var contentH = maxY - minY

        // Calculate zoom to fit
        var scaleX = flickable.width / contentW
        var scaleY = flickable.height / contentH
        zoomScale = Math.min(Math.min(scaleX, scaleY), maxZoom)
        zoomScale = Math.max(zoomScale, minZoom)

        // Center content
        flickable.contentX = minX * zoomScale - (flickable.width - contentW * zoomScale) / 2
        flickable.contentY = minY * zoomScale - (flickable.height - contentH * zoomScale) / 2
    }

    function zoomToArea(x1, y1, x2, y2) {
        // Normalize coordinates
        var minX = Math.min(x1, x2)
        var minY = Math.min(y1, y2)
        var maxX = Math.max(x1, x2)
        var maxY = Math.max(y1, y2)

        var areaW = maxX - minX
        var areaH = maxY - minY

        if (areaW < 10 || areaH < 10) return  // Too small

        // Calculate zoom to fit area
        var scaleX = flickable.width / areaW
        var scaleY = flickable.height / areaH
        zoomScale = Math.min(Math.min(scaleX, scaleY), maxZoom)
        zoomScale = Math.max(zoomScale, minZoom)

        // Center on area
        var centerX = (minX + maxX) / 2
        var centerY = (minY + maxY) / 2
        flickable.contentX = centerX * zoomScale - flickable.width / 2
        flickable.contentY = centerY * zoomScale - flickable.height / 2
    }

    function startZoomAreaSelection() {
        zoomAreaMode = true
    }

    function cancelZoomAreaSelection() {
        zoomAreaMode = false
        isSelectingZoomArea = false
    }

    // Connection preview
    property var dragStartPort: null
    property point dragEndPoint: Qt.point(0, 0)
    property bool isDraggingConnection: false

    // Context menu position
    property point contextMenuPosition: Qt.point(0, 0)

    // Paste position - center of visible area or context menu position
    property point pastePosition: {
        if (contextMenuPosition.x > 0 && contextMenuPosition.y > 0) {
            return contextMenuPosition
        }
        // Default: center of visible canvas area
        var centerX = (flickable.contentX + flickable.width / 2) / zoomScale
        var centerY = (flickable.contentY + flickable.height / 2) / zoomScale
        return Qt.point(centerX, centerY)
    }

    // Selected connection
    property var selectedConnection: null

    // Selected node (for properties panel)
    property var selectedNode: null

    signal selectionChanged()
    signal nodeClicked(var node)      // Any click on a node
    signal emptyCanvasClicked()       // Click on empty canvas
    signal connectionClicked()        // Click on a connection

    // Delete selected connection
    function deleteSelectedConnection() {
        if (selectedConnection) {
            graph.disconnect(selectedConnection)
            selectedConnection = null
        }
    }

    // Clear all selections
    function clearAllSelections() {
        graph.clearSelection()
        selectedConnection = null
        selectedNode = null
        selectionChanged()
    }

    // Find non-overlapping position for a node
    function findValidPosition(nodeUuid, desiredPos, nodeWidth, nodeHeight) {
        var margin = 10
        var pos = Qt.point(desiredPos.x, desiredPos.y)

        // Check collision with all other nodes
        for (var i = 0; i < nodesRepeater.count; i++) {
            var otherItem = nodesRepeater.itemAt(i)
            if (!otherItem || !otherItem.nodeData) continue
            if (otherItem.nodeData.uuid === nodeUuid) continue

            var otherX = otherItem.x
            var otherY = otherItem.y
            var otherW = otherItem.width
            var otherH = otherItem.height

            // Check for overlap
            if (pos.x < otherX + otherW + margin &&
                pos.x + nodeWidth + margin > otherX &&
                pos.y < otherY + otherH + margin &&
                pos.y + nodeHeight + margin > otherY) {

                // Collision detected - try to find closest non-overlapping position
                var dx = (pos.x + nodeWidth / 2) - (otherX + otherW / 2)
                var dy = (pos.y + nodeHeight / 2) - (otherY + otherH / 2)

                // Move in the direction we came from
                if (Math.abs(dx) > Math.abs(dy)) {
                    // Move horizontally
                    if (dx > 0) {
                        pos.x = otherX + otherW + margin
                    } else {
                        pos.x = otherX - nodeWidth - margin
                    }
                } else {
                    // Move vertically
                    if (dy > 0) {
                        pos.y = otherY + otherH + margin
                    } else {
                        pos.y = otherY - nodeHeight - margin
                    }
                }

                // Restart check with new position
                i = -1
            }
        }

        // Snap to grid
        var gridSnap = 20
        pos.x = Math.round(pos.x / gridSnap) * gridSnap
        pos.y = Math.round(pos.y / gridSnap) * gridSnap

        return pos
    }

    // Find port at position (hit-testing)
    function findPortAtPosition(pos) {
        var hitRadius = Theme.portRadius + 8

        // Iterate through NodeItem delegates
        for (var i = 0; i < nodesRepeater.count; i++) {
            var nodeItem = nodesRepeater.itemAt(i)
            if (!nodeItem || !nodeItem.nodeData) continue

            var node = nodeItem.nodeData

            // Safety check - ensure node has the required methods
            if (typeof node.inputCount !== 'function' || typeof node.outputCount !== 'function') continue

            // Check all inputs
            var inputCnt = node.inputCount()
            for (var j = 0; j < inputCnt; j++) {
                var port = node.inputAt(j)
                if (port && port !== dragStartPort) {
                    var portPos = port.scenePosition
                    var dx = pos.x - portPos.x
                    var dy = pos.y - portPos.y
                    var dist = Math.sqrt(dx * dx + dy * dy)
                    if (dist < hitRadius) {
                        return port
                    }
                }
            }

            // Check all outputs
            var outputCnt = node.outputCount()
            for (var k = 0; k < outputCnt; k++) {
                var outPort = node.outputAt(k)
                if (outPort && outPort !== dragStartPort) {
                    var outPortPos = outPort.scenePosition
                    var dxOut = pos.x - outPortPos.x
                    var dyOut = pos.y - outPortPos.y
                    var distOut = Math.sqrt(dxOut * dxOut + dyOut * dyOut)
                    if (distOut < hitRadius) {
                        return outPort
                    }
                }
            }
        }

        return null
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: canvasWidth * zoomScale
        contentHeight: canvasHeight * zoomScale
        clip: true
        interactive: !zoomAreaMode  // Disable panning when selecting zoom area

        // Drop area for nodes from toolbar
        DropArea {
            id: dropArea
            anchors.fill: parent
            keys: ["node"]

            onDropped: function(drop) {
                if (drop.keys.indexOf("node") !== -1) {
                    // Calculate canvas position from drop position
                    var canvasX = (drop.x + flickable.contentX) / zoomScale
                    var canvasY = (drop.y + flickable.contentY) / zoomScale

                    // Snap to grid
                    canvasX = Math.round(canvasX / gridSize) * gridSize
                    canvasY = Math.round(canvasY / gridSize) * gridSize

                    // Get node type from drag source
                    var nodeType = drop.source.nodeType
                    if (nodeType) {
                        console.log("Creating node:", nodeType, "at", canvasX, canvasY)
                        graph.createNode(nodeType, Qt.point(canvasX, canvasY))
                    }
                }
            }

            // Visual feedback during drag over
            Rectangle {
                anchors.fill: parent
                color: dropArea.containsDrag ? Theme.accent : "transparent"
                opacity: 0.1
                visible: dropArea.containsDrag
            }
        }

        // Mouse wheel handler for zoom and scroll
        WheelHandler {
            id: wheelHandler
            target: null  // Don't move target, handle manually
            onWheel: function(event) {
                var delta = event.angleDelta.y
                var scrollAmount = 60  // Pixels to scroll per wheel step

                if (event.modifiers & Qt.ControlModifier) {
                    // Ctrl+wheel = vertical scroll
                    flickable.contentY = Math.max(0, Math.min(
                        flickable.contentHeight - flickable.height,
                        flickable.contentY - delta * scrollAmount / 120
                    ))
                } else if (event.modifiers & Qt.ShiftModifier) {
                    // Shift+wheel = horizontal scroll
                    flickable.contentX = Math.max(0, Math.min(
                        flickable.contentWidth - flickable.width,
                        flickable.contentX - delta * scrollAmount / 120
                    ))
                } else {
                    // Plain wheel = zoom towards mouse position
                    var oldScale = zoomScale
                    if (delta > 0) {
                        zoomScale = Math.min(maxZoom, zoomScale * 1.1)
                    } else {
                        zoomScale = Math.max(minZoom, zoomScale / 1.1)
                    }

                    // Zoom towards mouse position
                    if (oldScale !== zoomScale) {
                        var mouseX = event.x + flickable.contentX
                        var mouseY = event.y + flickable.contentY
                        var factor = zoomScale / oldScale

                        flickable.contentX = mouseX * factor - event.x
                        flickable.contentY = mouseY * factor - event.y
                    }
                }
            }
        }

        // Scaled content container
        Item {
            id: scaledContent
            objectName: "scaledContent"
            width: canvasWidth
            height: canvasHeight
            scale: zoomScale
            transformOrigin: Item.TopLeft

            // Grid background
            Rectangle {
                id: gridBackground
                width: canvasWidth
                height: canvasHeight
                color: Theme.background

            // Grid pattern (conditional)
            Canvas {
                anchors.fill: parent
                visible: showGrid
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    // Minor grid
                    ctx.strokeStyle = Theme.gridLine
                    ctx.lineWidth = 1
                    ctx.beginPath()

                    for (var x = 0; x <= width; x += gridSize) {
                        ctx.moveTo(x, 0)
                        ctx.lineTo(x, height)
                    }
                    for (var y = 0; y <= height; y += gridSize) {
                        ctx.moveTo(0, y)
                        ctx.lineTo(width, y)
                    }
                    ctx.stroke()

                    // Major grid
                    ctx.strokeStyle = Theme.gridLineMajor
                    ctx.beginPath()

                    for (x = 0; x <= width; x += gridSize * 5) {
                        ctx.moveTo(x, 0)
                        ctx.lineTo(x, height)
                    }
                    for (y = 0; y <= height; y += gridSize * 5) {
                        ctx.moveTo(0, y)
                        ctx.lineTo(width, y)
                    }
                    ctx.stroke()
                }
            }

            // Mouse area for clicks, context menu, and zoom area selection
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                cursorShape: zoomAreaMode ? Qt.CrossCursor : Qt.ArrowCursor

                onClicked: function(mouse) {
                    if (mouse.button === Qt.LeftButton && !zoomAreaMode) {
                        clearAllSelections()
                        emptyCanvasClicked()
                    }
                }

                onPressed: function(mouse) {
                    if (mouse.button === Qt.RightButton) {
                        // Cancel zoom area mode on right click
                        if (zoomAreaMode) {
                            cancelZoomAreaSelection()
                            return
                        }
                        // Store position in canvas coordinates
                        contextMenuPosition = Qt.point(mouse.x, mouse.y)
                        contextMenu.popup()
                    } else if (mouse.button === Qt.LeftButton && zoomAreaMode) {
                        // Start zoom area selection
                        zoomAreaStart = Qt.point(mouse.x, mouse.y)
                        zoomAreaEnd = Qt.point(mouse.x, mouse.y)
                        isSelectingZoomArea = true
                    }
                }

                onPositionChanged: function(mouse) {
                    if (isSelectingZoomArea) {
                        zoomAreaEnd = Qt.point(mouse.x, mouse.y)
                    }
                }

                onReleased: function(mouse) {
                    if (isSelectingZoomArea && mouse.button === Qt.LeftButton) {
                        // Perform zoom to area
                        zoomToArea(zoomAreaStart.x, zoomAreaStart.y, zoomAreaEnd.x, zoomAreaEnd.y)
                        zoomAreaMode = false
                        isSelectingZoomArea = false
                    }
                }
            }

            // Zoom area selection rectangle
            Rectangle {
                visible: isSelectingZoomArea
                x: Math.min(zoomAreaStart.x, zoomAreaEnd.x)
                y: Math.min(zoomAreaStart.y, zoomAreaEnd.y)
                width: Math.abs(zoomAreaEnd.x - zoomAreaStart.x)
                height: Math.abs(zoomAreaEnd.y - zoomAreaStart.y)
                color: "transparent"
                border.color: Theme.accent
                border.width: 2
                opacity: 0.8

                Rectangle {
                    anchors.fill: parent
                    color: Theme.accent
                    opacity: 0.1
                }
            }
        }

            // Connections layer - use ListModel for proper add/remove handling
            ListModel {
                id: connectionsModel
            }

            Repeater {
                id: connectionsRepeater
                model: connectionsModel

                delegate: ConnectionItem {
                    connection: model.connectionObj
                    selected: root.selectedConnection === model.connectionObj

                    onClicked: {
                        graph.clearSelection()
                        root.selectedNode = null
                        root.selectedConnection = model.connectionObj
                        root.connectionClicked()
                    }

                    onDeleteRequested: {
                        graph.disconnect(model.connectionObj)
                        if (root.selectedConnection === model.connectionObj) {
                            root.selectedConnection = null
                        }
                    }
                }
            }

            // Sync connections model with C++ graph
            Connections {
                target: graph
                function onConnectionAdded(conn) {
                    connectionsModel.append({ "connectionObj": conn })
                }
                function onConnectionRemoved(conn) {
                    for (var i = 0; i < connectionsModel.count; i++) {
                        if (connectionsModel.get(i).connectionObj === conn) {
                            connectionsModel.remove(i)
                            break
                        }
                    }
                }
            }

            // Nodes layer
            Repeater {
                id: nodesRepeater
                model: graph

                delegate: NodeItem {
                    required property var node
                    nodeData: node
                    graph: root.graph
                    canvas: root
                    currentTime: root.currentTime
                    connectionDragging: root.isDraggingConnection
                    dragPosition: root.dragEndPoint

                    onConnectionDragStarted: function(port, mousePos) {
                        root.dragStartPort = port
                        root.dragEndPoint = mousePos
                        root.isDraggingConnection = true
                    }

                    onConnectionDragUpdated: function(mousePos) {
                        root.dragEndPoint = mousePos
                    }

                    onConnectionDragEnded: function(mousePos) {
                        // Find target port at mouse position
                        var targetPort = findPortAtPosition(mousePos)
                        if (targetPort && root.dragStartPort) {
                            graph.connect(root.dragStartPort, targetPort)
                        }
                        root.isDraggingConnection = false
                        root.dragStartPort = null
                    }

                }
            }

            // Connection preview while dragging
            ConnectionPreview {
                id: connectionPreview
                visible: isDraggingConnection
                startPort: dragStartPort
                endPoint: dragEndPoint
                width: canvasWidth
                height: canvasHeight
            }
        }  // Close scaledContent
    }

    // Context menu
    Menu {
        id: contextMenu
        delegate: StyledMenuItem {}

        background: Rectangle {
            implicitWidth: 150
            color: Theme.surface
            border.color: Theme.border
            radius: 2
        }

        Action {
            text: qsTr("Paste")
            enabled: graph.canPaste
            onTriggered: graph.pasteAtPosition(contextMenuPosition)
        }
    }

    // Initialize connections model from existing graph connections
    Component.onCompleted: {
        var conns = graph.connections
        for (var i = 0; i < conns.length; i++) {
            connectionsModel.append({ "connectionObj": conns[i] })
        }
    }

    // Delete selected items function
    function deleteSelected() {
        // Delete selected connection first
        if (selectedConnection) {
            graph.disconnect(selectedConnection)
            selectedConnection = null
            return
        }

        // Then delete selected nodes (except Input/Output)
        var selected = graph.selectedNodes()
        for (var i = 0; i < selected.length; ++i) {
            if (selected[i].type !== "Input" && selected[i].type !== "Output") {
                graph.removeNode(selected[i].uuid)
            }
        }
    }

    // Keyboard shortcuts
    Shortcut {
        sequence: "Delete"
        onActivated: deleteSelected()
    }

    Shortcut {
        sequence: "Backspace"
        onActivated: deleteSelected()
    }
}
