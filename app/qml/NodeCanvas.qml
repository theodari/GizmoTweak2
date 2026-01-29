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
        zoomScale = Math.min(maxZoom, zoomScale * 1.1)
    }

    function zoomOut() {
        zoomScale = Math.max(minZoom, zoomScale / 1.1)
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
        flickable.clampScroll(false)
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

        // Center bounding box in view (no clamping to allow proper centering)
        var centerX = (minX + maxX) / 2
        var centerY = (minY + maxY) / 2
        flickable.contentX = centerX * zoomScale - flickable.width / 2
        flickable.contentY = centerY * zoomScale - flickable.height / 2
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
        flickable.clampScroll(false)
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
    signal nodeDoubleClicked(var node) // Double-click on a node
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

    // Snap threshold for cable alignment (in pixels)
    readonly property real cableSnapThreshold: 8

    // Compute port center position for a node at a given position
    // Returns {x, y} for each port based on its placement side
    function portAbsolutePos(nodePos, nodeW, nodeH, port) {
        // Determine port side from port properties
        var isFrame = (port.dataType === Port.DataType.Frame)
        var isInput = (port.direction === Port.Direction.In)
        var isRatio = !isFrame

        // For tweak nodes: frame in = top, frame out = bottom, ratio in = left
        // For shape/utility: inputs = left, outputs = right
        // For IO: input node output = bottom, output node input = top

        var node = port.node
        if (!node) return null

        var cat = node.category
        var type = node.type

        if (cat === Node.Category.Tweak) {
            if (isFrame && isInput)
                return Qt.point(nodePos.x + nodeW / 2, nodePos.y)  // top center
            if (isFrame && !isInput)
                return Qt.point(nodePos.x + nodeW / 2, nodePos.y + nodeH)  // bottom center
            if (isRatio && isInput)
                return Qt.point(nodePos.x, nodePos.y + nodeH / 2)  // left center
        } else if (cat === Node.Category.IO) {
            if (type === "Input" && !isInput)
                return Qt.point(nodePos.x + nodeW / 2, nodePos.y + nodeH)  // bottom
            if (type === "Output" && isInput)
                return Qt.point(nodePos.x + nodeW / 2, nodePos.y)  // top
        } else {
            // Shape, Utility
            if (isInput)
                return Qt.point(nodePos.x, nodePos.y + nodeH / 2)  // left
            if (!isInput)
                return Qt.point(nodePos.x + nodeW, nodePos.y + nodeH / 2)  // right
        }
        return null
    }

    // Snap a node position so that its connected cables align vertically or horizontally
    function snapToConnectionAlignment(nodeData, desiredPos, nodeW, nodeH) {
        if (!nodeData || !graph) return desiredPos

        var snapX = desiredPos.x
        var snapY = desiredPos.y
        var bestDx = cableSnapThreshold + 1
        var bestDy = cableSnapThreshold + 1

        var conns = graph.connections
        for (var i = 0; i < conns.length; i++) {
            var conn = conns[i]
            var srcPort = conn.sourcePort
            var tgtPort = conn.targetPort
            if (!srcPort || !tgtPort) continue

            // Determine which port belongs to the dragged node and which is remote
            var localPort = null
            var remotePort = null
            if (srcPort.node === nodeData) {
                localPort = srcPort
                remotePort = tgtPort
            } else if (tgtPort.node === nodeData) {
                localPort = tgtPort
                remotePort = srcPort
            } else {
                continue
            }

            // Get the local port position at the desired pos
            var localPos = portAbsolutePos(Qt.point(desiredPos.x, desiredPos.y), nodeW, nodeH, localPort)
            if (!localPos) continue

            // Get remote port position at remote node's current position
            var remoteNode = remotePort.node
            if (!remoteNode) continue

            // Find remote node item to get its dimensions
            var remoteItem = null
            for (var j = 0; j < nodesRepeater.count; j++) {
                var item = nodesRepeater.itemAt(j)
                if (item && item.nodeData === remoteNode) {
                    remoteItem = item
                    break
                }
            }
            if (!remoteItem) continue

            var remotePos = portAbsolutePos(remoteNode.position, remoteItem.width, remoteItem.height, remotePort)
            if (!remotePos) continue

            // Check X alignment (vertical cable)
            var dx = Math.abs(localPos.x - remotePos.x)
            if (dx < cableSnapThreshold && dx < bestDx) {
                bestDx = dx
                snapX = desiredPos.x + (remotePos.x - localPos.x)
            }

            // Check Y alignment (horizontal cable)
            var dy = Math.abs(localPos.y - remotePos.y)
            if (dy < cableSnapThreshold && dy < bestDy) {
                bestDy = dy
                snapY = desiredPos.y + (remotePos.y - localPos.y)
            }
        }

        return Qt.point(snapX, snapY)
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

    // Manual pan/zoom container (no Flickable scrolling interference)
    Item {
        id: flickable  // Keep id for compatibility
        anchors.fill: parent
        clip: true

        // Manual scroll position (replaces Flickable's contentX/Y)
        property real contentX: 0
        property real contentY: 0
        property real contentWidth: canvasWidth * zoomScale
        property real contentHeight: canvasHeight * zoomScale

        // Elastic scrolling settings
        property real borderOverflow: 3        // Pixels of border visible at rest
        property real elasticOverflow: 30      // Max elastic overflow when dragging
        property bool isInteracting: panHandler.active || wheelHandler.isWheeling

        // Animated scroll for snap-back
        Behavior on contentX {
            enabled: !flickable.isInteracting
            NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
        }
        Behavior on contentY {
            enabled: !flickable.isInteracting
            NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
        }

        // Clamp scroll to valid bounds (with optional elasticity)
        function clampScroll(elastic) {
            var minX = -borderOverflow
            var minY = -borderOverflow
            var maxX = Math.max(-borderOverflow, contentWidth - width + borderOverflow)
            var maxY = Math.max(-borderOverflow, contentHeight - height + borderOverflow)

            if (elastic) {
                // Allow elastic overflow during interaction
                minX = -borderOverflow - elasticOverflow
                minY = -borderOverflow - elasticOverflow
                maxX = Math.max(minX, contentWidth - width + borderOverflow + elasticOverflow)
                maxY = Math.max(minY, contentHeight - height + borderOverflow + elasticOverflow)
            }

            contentX = Math.max(minX, Math.min(maxX, contentX))
            contentY = Math.max(minY, Math.min(maxY, contentY))
        }

        // Snap back to proper bounds when interaction ends
        function snapToBounds() {
            var minX = -borderOverflow
            var minY = -borderOverflow
            var maxX = Math.max(-borderOverflow, contentWidth - width + borderOverflow)
            var maxY = Math.max(-borderOverflow, contentHeight - height + borderOverflow)

            contentX = Math.max(minX, Math.min(maxX, contentX))
            contentY = Math.max(minY, Math.min(maxY, contentY))
        }

        // Dark background showing out-of-bounds area
        Rectangle {
            anchors.fill: parent
            color: Qt.darker(Theme.background, 1.3)
        }

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
            target: null
            property bool isWheeling: false  // Track wheel activity for animation disable

            onWheel: function(event) {
                isWheeling = true
                wheelTimer.restart()

                var delta = event.angleDelta.y
                var scrollAmount = 60

                if (event.modifiers & Qt.ControlModifier) {
                    // Ctrl+wheel = vertical scroll
                    flickable.contentY -= delta * scrollAmount / 120
                    flickable.clampScroll(false)
                } else if (event.modifiers & Qt.ShiftModifier) {
                    // Shift+wheel = horizontal scroll
                    flickable.contentX -= delta * scrollAmount / 120
                    flickable.clampScroll(false)
                } else {
                    // Plain wheel = zoom towards mouse position
                    var oldScale = zoomScale
                    var zoomFactor = 1.033
                    if (delta > 0) {
                        zoomScale = Math.min(maxZoom, zoomScale * zoomFactor)
                    } else {
                        zoomScale = Math.max(minZoom, zoomScale / zoomFactor)
                    }

                    if (oldScale !== zoomScale) {
                        // Mouse position in viewport
                        var viewportX = event.x
                        var viewportY = event.y

                        // Canvas point under cursor (unscaled coordinates)
                        var canvasX = (viewportX + flickable.contentX) / oldScale
                        var canvasY = (viewportY + flickable.contentY) / oldScale

                        // Where that canvas point is after zoom
                        var newPosX = canvasX * zoomScale
                        var newPosY = canvasY * zoomScale

                        // Adjust scroll to keep canvas point under cursor
                        flickable.contentX = newPosX - viewportX
                        flickable.contentY = newPosY - viewportY
                        flickable.clampScroll(false)
                    }
                }
            }
        }

        // Timer for wheel activity tracking (outside WheelHandler)
        Timer {
            id: wheelTimer
            interval: 100
            onTriggered: wheelHandler.isWheeling = false
        }

        // Drag handler for panning
        DragHandler {
            id: panHandler
            target: null
            enabled: !zoomAreaMode

            onTranslationChanged: {
                flickable.contentX -= translation.x - prevTranslation.x
                flickable.contentY -= translation.y - prevTranslation.y
                prevTranslation = translation
                flickable.clampScroll(true)  // Allow elastic during drag
            }

            property point prevTranslation: Qt.point(0, 0)
            onActiveChanged: {
                if (active) {
                    prevTranslation = Qt.point(0, 0)
                } else {
                    // Snap back when drag ends
                    flickable.snapToBounds()
                }
            }
        }

        // Scaled content container - positioned manually
        Item {
            id: scaledContent
            objectName: "scaledContent"
            x: -flickable.contentX
            y: -flickable.contentY
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

                property bool wasDragged: false

                onPressed: function(mouse) {
                    wasDragged = false
                    if (mouse.button === Qt.LeftButton && !zoomAreaMode) {
                        clearAllSelections()
                        emptyCanvasClicked()
                    }
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
                    if (root.selectedConnection === conn) {
                        root.selectedConnection = null
                    }
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

    // Align/distribute: delegate to C++ (QML iteration of Node* lists is unreliable)
    function alignNodes(mode) { graph.alignSelected(mode) }
    function distributeNodes(mode) { graph.distributeSelected(mode) }

    // Delete confirmation dialog
    Dialog {
        id: deleteConfirmDialog
        title: qsTr("Confirm Delete")
        modal: true
        anchors.centerIn: parent

        property int nodeCount: 0

        Label {
            text: qsTr("Delete %1 node(s)?").arg(deleteConfirmDialog.nodeCount)
            color: Theme.text
        }

        background: Rectangle {
            color: Theme.surface
            border.color: Theme.border
            radius: 4
        }

        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: {
            var selected = graph.selectedNodes()
            for (var i = 0; i < selected.length; i++) {
                if (selected[i].type !== "Input" && selected[i].type !== "Output") {
                    graph.removeNode(selected[i].uuid)
                }
            }
        }
    }

    function confirmDeleteSelected() {
        var selected = graph.selectedNodes()
        // Filter out Input/Output
        var deletable = 0
        for (var i = 0; i < selected.length; i++) {
            if (selected[i].type !== "Input" && selected[i].type !== "Output")
                deletable++
        }
        if (deletable === 0) return
        deleteConfirmDialog.nodeCount = deletable
        deleteConfirmDialog.open()
    }

    // Delete selected items function
    function deleteSelected() {
        // Delete selected connection first (no confirmation needed)
        if (selectedConnection) {
            graph.disconnect(selectedConnection)
            selectedConnection = null
            return
        }

        // Delete selected nodes with confirmation
        confirmDeleteSelected()
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
