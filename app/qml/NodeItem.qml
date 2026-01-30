import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

Rectangle {
    id: root

    property Node nodeData: null
    required property NodeGraph graph
    property var canvas: null  // Reference to NodeCanvas for collision detection
    property real currentTime: 0.0  // Animation time for previews

    // Counter to force re-evaluation of port-related properties when node properties change
    property int portUpdateTrigger: 0
    Connections {
        target: nodeData
        function onPropertyChanged() { root.portUpdateTrigger++ }
    }

    // Connection dragging state (passed from NodeCanvas)
    property bool connectionDragging: false
    property point dragPosition: Qt.point(0, 0)

    signal connectionDragStarted(var port, point mousePos)
    signal connectionDragUpdated(point mousePos)
    signal connectionDragEnded(point mousePos)  // Now passes position for hit-testing
    signal doubleClicked()  // For showing properties panel

    // Check if this is an Input node (for mini-preview)
    readonly property bool isInputNode: nodeData && nodeData.type === "Input"

    // Check if this node needs a C++ preview (shapes, utilities, tweaks - but not Input/Output)
    readonly property bool needsCppPreview: nodeData && nodeData.type !== "Input" && nodeData.type !== "Output"

    // Position is driven by nodeData.position via explicit sync (not binding,
    // because drag.target breaks bindings). See onPositionChanged below.
    x: nodeData ? nodeData.position.x : 0
    y: nodeData ? nodeData.position.y : 0
    width: 112
    height: 78

    radius: Theme.nodeRadius


    // Hover state
    property bool isHovering: false

    // Visual properties based on state
    border.color: nodeData && nodeData.selected ? Theme.accent : (isHovering ? Qt.lighter(Theme.border, 1.3) : Theme.border)
    border.width: nodeData && nodeData.selected ? 3 : (isHovering ? 2 : 1)

    // Base color based on category and type
    property color baseColor: {
        if (!nodeData) return Theme.surface
        if (nodeData.category === Node.Category.IO) return Theme.nodeIO
        if (nodeData.category === Node.Category.Shape) {
            if (nodeData.type === "Transform") return Theme.nodeTransform
            if (nodeData.type === "SurfaceFactory") return Theme.nodeSurface
            return Theme.nodeGizmo  // Gizmo
        }
        if (nodeData.category === Node.Category.Utility) return Theme.nodeUtility
        if (nodeData.category === Node.Category.Tweak) return Theme.nodeTweak
        return Theme.surface
    }

    // Node background color - lighter when selected or hovered
    color: nodeData && nodeData.selected ? Qt.lighter(baseColor, 1.2) : (isHovering ? Qt.lighter(baseColor, 1.1) : baseColor)

    // Determine layout type based on category
    readonly property bool isIONode: nodeData && (nodeData.category === Node.Category.IO)
    readonly property bool isShapeOrUtility: nodeData && (nodeData.category === Node.Category.Shape || nodeData.category === Node.Category.Utility)
    readonly property bool isTweak: nodeData && (nodeData.category === Node.Category.Tweak)

    // Determine which sides have ports
    readonly property bool hasTopPorts: nodeData && typeof nodeData.inputCount === 'function' ? (nodeData.type === "Output" ? nodeData.inputCount() > 0 : (isTweak ? getFrameInput() !== null : false)) : false
    readonly property bool hasBottomPorts: nodeData && typeof nodeData.outputCount === 'function' ? (nodeData.type === "Input" ? nodeData.outputCount() > 0 : (isTweak ? getFrameOutput() !== null : false)) : false
    readonly property bool hasLeftPorts: portUpdateTrigger >= 0 && nodeData && typeof nodeData.inputCount === 'function' ? (isShapeOrUtility ? nodeData.inputCount() > 0 : (isTweak ? getRatioInput() !== null : false)) : false
    readonly property bool hasRightPorts: nodeData && isShapeOrUtility && typeof nodeData.outputCount === 'function' ? nodeData.outputCount() > 0 : false

    // Helper functions to get specific ports for tweaks
    function getFrameInput() {
        if (!nodeData || typeof nodeData.inputCount !== 'function') return null
        var count = nodeData.inputCount()
        for (var i = 0; i < count; i++) {
            var port = nodeData.inputAt(i)
            if (port && port.dataType === Port.DataType.Frame) return port
        }
        return null
    }

    function getFrameOutput() {
        if (!nodeData || typeof nodeData.outputCount !== 'function') return null
        var count = nodeData.outputCount()
        for (var i = 0; i < count; i++) {
            var port = nodeData.outputAt(i)
            if (port && port.dataType === Port.DataType.Frame) return port
        }
        return null
    }

    function getRatioInput() {
        if (!nodeData || typeof nodeData.inputCount !== 'function') return null
        var count = nodeData.inputCount()
        for (var i = 0; i < count; i++) {
            var port = nodeData.inputAt(i)
            if (port && port.visible && (port.dataType === Port.DataType.Ratio2D ||
                         port.dataType === Port.DataType.Ratio1D ||
                         port.dataType === Port.DataType.RatioAny)) return port
        }
        return null
    }

    function getPositionInput() {
        if (!nodeData || typeof nodeData.inputCount !== 'function') return null
        var count = nodeData.inputCount()
        for (var i = 0; i < count; i++) {
            var port = nodeData.inputAt(i)
            if (port && port.dataType === Port.DataType.Position) return port
        }
        return null
    }

    function getPositionOutput() {
        if (!nodeData || typeof nodeData.outputCount !== 'function') return null
        var count = nodeData.outputCount()
        for (var i = 0; i < count; i++) {
            var port = nodeData.outputAt(i)
            if (port && port.dataType === Port.DataType.Position) return port
        }
        return null
    }

    // Drag handling for the node itself (declared first so ports are on top)
    // NOTE: We do NOT use drag.target because it breaks QML bindings on x/y.
    // Instead we handle drag manually via mouse deltas → nodeData.position.
    MouseArea {
        id: dragArea
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        property bool isDragging: false
        property bool isCtrlClick: false
        // Mouse press position in canvas coordinates
        property real pressMouseX: 0
        property real pressMouseY: 0
        // Start positions of all selected nodes (uuid → {x, y})
        property var dragStartPositions: ({})

        onPressed: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                // Right-click: if node not selected, select it alone first
                if (nodeData && !nodeData.selected) {
                    graph.clearSelection()
                    nodeData.selected = true
                    if (canvas) {
                        canvas.selectedNode = nodeData
                        canvas.selectionChanged()
                    }
                } else if (canvas) {
                    var selCount = graph.selectedNodes().length
                    canvas.selectedNode = (selCount === 1) ? nodeData : null
                }
                nodeContextMenu.popup()
                return
            }

            // Left click: clear connection selection
            if (canvas) canvas.selectedConnection = null

            isCtrlClick = (mouse.modifiers & Qt.ControlModifier)

            if (isCtrlClick) {
                // Ctrl+Click: toggle selection
                if (nodeData) {
                    nodeData.selected = !nodeData.selected
                    if (canvas) {
                        var sc = graph.selectedNodes().length
                        canvas.selectedNode = (sc === 1) ? graph.selectedNodes()[0] : null
                        canvas.selectionChanged()
                    }
                }
                return
            }

            // Plain click: clear and select
            if (nodeData && !nodeData.selected) {
                graph.clearSelection()
            }
            if (nodeData) {
                nodeData.selected = true
                if (canvas) {
                    canvas.selectedNode = nodeData
                    canvas.selectionChanged()
                    canvas.nodeClicked(nodeData)
                }
            }

            // Store mouse position in parent (canvas) coordinates
            var posInParent = mapToItem(root.parent, mouse.x, mouse.y)
            pressMouseX = posInParent.x
            pressMouseY = posInParent.y

            // Store start positions for multi-drag
            dragStartPositions = {}
            var selected = graph.selectedNodes()
            for (var i = 0; i < selected.length; i++) {
                var n = selected[i]
                dragStartPositions[n.uuid] = Qt.point(n.position.x, n.position.y)
                graph.beginMoveNode(n.uuid)
            }
        }

        onPositionChanged: function(mouse) {
            if (!pressed || isCtrlClick || mouse.button === Qt.RightButton) return

            isDragging = true
            var posInParent = mapToItem(root.parent, mouse.x, mouse.y)
            var dx = posInParent.x - pressMouseX
            var dy = posInParent.y - pressMouseY

            // Apply snap for the primary node
            if (nodeData && canvas && typeof canvas.snapToConnectionAlignment === 'function') {
                var startSelf = dragStartPositions[nodeData.uuid]
                if (startSelf) {
                    var desiredPos = Qt.point(startSelf.x + dx, startSelf.y + dy)
                    var snapped = canvas.snapToConnectionAlignment(nodeData, desiredPos, root.width, root.height)
                    dx = snapped.x - startSelf.x
                    dy = snapped.y - startSelf.y
                }
            }

            // Move all selected nodes by delta (sets nodeData.position → bindings update x/y)
            for (var uuid in dragStartPositions) {
                var startPos = dragStartPositions[uuid]
                var node = graph.nodeByUuid(uuid)
                if (node) {
                    node.position = Qt.point(startPos.x + dx, startPos.y + dy)
                }
            }
        }

        onReleased: function(mouse) {
            if (mouse.button === Qt.RightButton) return

            if (isDragging) {
                // Finalize positions for all selected nodes
                for (var uuid in dragStartPositions) {
                    var node = graph.nodeByUuid(uuid)
                    if (node) {
                        graph.endMoveNode(uuid, node.position)
                    }
                }
                isDragging = false
            }
            dragStartPositions = {}
            isCtrlClick = false
        }

        onEntered: root.isHovering = true
        onExited: root.isHovering = false

        onDoubleClicked: {
            root.doubleClicked()
            if (nodeData && canvas) {
                canvas.nodeDoubleClicked(nodeData)
            }
        }
    }

    // Selection count (updated when context menu opens)
    property int _selCount: 0

    // Node context menu (right-click)
    Menu {
        id: nodeContextMenu
        delegate: StyledMenuItem {}

        background: Rectangle {
            implicitWidth: 180
            color: Theme.surface
            border.color: Theme.border
            radius: 2
        }

        onAboutToShow: {
            root._selCount = graph.selectedNodes().length
        }

        // Multi-selection items (only visible when 2+ nodes selected)
        Menu {
            title: qsTr("Align")
            enabled: root._selCount > 1
            visible: root._selCount > 1
            delegate: StyledMenuItem {}

            background: Rectangle {
                implicitWidth: 160
                color: Theme.surface
                border.color: Theme.border
                radius: 2
            }

            Action { text: qsTr("Left"); onTriggered: canvas.alignNodes("left") }
            Action { text: qsTr("Center"); onTriggered: canvas.alignNodes("center") }
            Action { text: qsTr("Right"); onTriggered: canvas.alignNodes("right") }
            MenuSeparator {}
            Action { text: qsTr("Top"); onTriggered: canvas.alignNodes("top") }
            Action { text: qsTr("Middle"); onTriggered: canvas.alignNodes("middle") }
            Action { text: qsTr("Bottom"); onTriggered: canvas.alignNodes("bottom") }
        }

        Menu {
            title: qsTr("Distribute")
            enabled: root._selCount > 2
            visible: root._selCount > 2
            delegate: StyledMenuItem {}

            background: Rectangle {
                implicitWidth: 160
                color: Theme.surface
                border.color: Theme.border
                radius: 2
            }

            Action { text: qsTr("Horizontally"); onTriggered: canvas.distributeNodes("horizontal") }
            Action { text: qsTr("Vertically"); onTriggered: canvas.distributeNodes("vertical") }
        }

        MenuSeparator {
            visible: root._selCount > 1
        }

        Action {
            text: qsTr("Delete")
            enabled: nodeData && nodeData.type !== "Input" && nodeData.type !== "Output"
            onTriggered: canvas.confirmDeleteSelected()
        }
    }

    // Icon for Output node (target/bullseye)
    Canvas {
        id: outputIcon
        visible: nodeData && nodeData.type === "Output"
        anchors.centerIn: parent
        width: Math.min(root.height - 16, root.width - 16)
        height: width

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var cx = width / 2, cy = height / 2
            var r = Math.min(width, height) / 2 - 4

            // Outer circle
            ctx.strokeStyle = Qt.darker(Theme.nodeIO, 1.4)
            ctx.lineWidth = 3
            ctx.beginPath()
            ctx.arc(cx, cy, r, 0, 2 * Math.PI)
            ctx.stroke()

            // Middle circle
            ctx.beginPath()
            ctx.arc(cx, cy, r * 0.6, 0, 2 * Math.PI)
            ctx.stroke()

            // Inner filled circle (bullseye)
            ctx.fillStyle = Qt.darker(Theme.nodeIO, 1.4)
            ctx.beginPath()
            ctx.arc(cx, cy, r * 0.25, 0, 2 * Math.PI)
            ctx.fill()

            // Arrow pointing in
            ctx.beginPath()
            ctx.moveTo(cx - r - 8, cy)
            ctx.lineTo(cx - r * 0.7, cy)
            ctx.stroke()

            // Arrowhead
            ctx.beginPath()
            ctx.moveTo(cx - r * 0.7, cy)
            ctx.lineTo(cx - r * 0.7 - 6, cy - 5)
            ctx.lineTo(cx - r * 0.7 - 6, cy + 5)
            ctx.closePath()
            ctx.fill()
        }
    }

    // Mini-preview for Input node - shows current pattern with play icon overlay
    Rectangle {
        id: miniPreview
        visible: isInputNode

        // Fixed preview size
        property real previewSize: 70

        width: previewSize
        height: previewSize
        anchors.left: parent.left
        anchors.leftMargin: 4
        anchors.verticalCenter: parent.verticalCenter

        color: "#000000"  // Black background like laser display
        radius: 4
        border.color: Theme.border
        border.width: 1

        // Use FramePreviewItem for C++ QPainter rendering
        // FramePreviewItem observes the node directly (no Frame* passed from QML)
        FramePreviewItem {
            id: inputFramePreview
            anchors.fill: parent
            anchors.margins: 2
            node: nodeData  // Pass the node, not the frame
            showGrid: false
            backgroundColor: "#000000"
            lineWidth: 1.5
        }

        // Play icon overlay (top-left corner)
        Canvas {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 4
            width: 16
            height: 16

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                // Play triangle
                ctx.fillStyle = Qt.darker(Theme.nodeIO, 1.3)
                ctx.beginPath()
                ctx.moveTo(2, 2)
                ctx.lineTo(14, 8)
                ctx.lineTo(2, 14)
                ctx.closePath()
                ctx.fill()
            }
        }

        // Pattern name overlay
        Text {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 2
            text: nodeData && nodeData.patternNames && nodeData.patternIndex >= 0
                  ? nodeData.patternNames[nodeData.patternIndex] || ""
                  : ""
            color: Theme.textMuted
            font.pixelSize: 9
            visible: text !== ""
        }
    }

    // C++ mini-preview for Shape, Utility, and Tweak nodes - positioned LEFT
    Rectangle {
        id: cppPreviewContainer
        visible: needsCppPreview

        // Fixed preview size
        property real previewSize: 70

        width: previewSize
        height: previewSize
        anchors.left: parent.left
        anchors.leftMargin: 4
        anchors.verticalCenter: parent.verticalCenter

        color: "#000000"
        radius: 4
        border.color: Theme.border
        border.width: 1

        NodePreviewItem {
            id: nodePreview
            anchors.fill: parent
            anchors.margins: 2
            node: nodeData
            graph: root.graph
            currentTime: root.currentTime
            resolution: 32
        }
    }

    // Icon on the RIGHT of preview for Shape, Utility, and Tweak nodes
    Canvas {
        id: nodeTypeIcon
        visible: needsCppPreview
        width: 30
        height: 30
        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.verticalCenter: parent.verticalCenter

        property string nodeType: nodeData ? nodeData.type : ""
        property color iconColor: Qt.darker(root.baseColor, 1.5)

        onNodeTypeChanged: requestPaint()
        onIconColorChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()

            var cx = width / 2, cy = height / 2
            var r = Math.min(width, height) / 2 - 2

            ctx.strokeStyle = iconColor
            ctx.fillStyle = iconColor
            ctx.lineWidth = 2
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            switch (nodeType) {
            case "Gizmo":
                // Draw ellipse using scale transform
                ctx.save()
                ctx.translate(cx, cy)
                ctx.scale(1, 0.7)
                ctx.beginPath()
                ctx.arc(0, 0, r, 0, Math.PI * 2)
                ctx.restore()
                ctx.stroke()
                // Highlight arc
                ctx.lineWidth = 1.5
                ctx.beginPath()
                ctx.arc(cx - r * 0.3, cy - r * 0.2, r * 0.4, Math.PI * 1.2, Math.PI * 1.8)
                ctx.stroke()
                break

            case "Transform":
                ctx.beginPath()
                ctx.moveTo(cx - r * 0.6, cy - r * 0.6)
                ctx.lineTo(cx - r * 0.6, cy + r * 0.6)
                ctx.lineTo(cx, cy + r * 0.6)
                ctx.arc(cx, cy, r * 0.6, Math.PI / 2, -Math.PI / 2, true)
                ctx.closePath()
                ctx.stroke()
                ctx.beginPath()
                ctx.arc(cx + r * 0.75, cy, r * 0.15, 0, Math.PI * 2)
                ctx.stroke()
                break

            case "SurfaceFactory":
                ctx.beginPath()
                ctx.moveTo(cx - r, cy)
                ctx.bezierCurveTo(cx - r * 0.5, cy - r, cx + r * 0.5, cy + r, cx + r, cy)
                ctx.stroke()
                ctx.beginPath()
                ctx.arc(cx - r, cy, 2, 0, Math.PI * 2)
                ctx.arc(cx + r, cy, 2, 0, Math.PI * 2)
                ctx.fill()
                break

            case "Mirror":
                ctx.beginPath()
                ctx.moveTo(cx - r * 0.8, cy - r * 0.5)
                ctx.lineTo(cx - r * 0.2, cy)
                ctx.lineTo(cx - r * 0.8, cy + r * 0.5)
                ctx.closePath()
                ctx.fill()
                ctx.beginPath()
                ctx.moveTo(cx + r * 0.8, cy - r * 0.5)
                ctx.lineTo(cx + r * 0.2, cy)
                ctx.lineTo(cx + r * 0.8, cy + r * 0.5)
                ctx.closePath()
                ctx.fill()
                ctx.setLineDash([2, 2])
                ctx.beginPath()
                ctx.moveTo(cx, cy - r)
                ctx.lineTo(cx, cy + r)
                ctx.stroke()
                ctx.setLineDash([])
                break

            case "TimeShift":
                ctx.beginPath()
                ctx.arc(cx, cy, r * 0.7, 0, Math.PI * 2)
                ctx.stroke()
                ctx.beginPath()
                ctx.moveTo(cx, cy)
                ctx.lineTo(cx, cy - r * 0.4)
                ctx.moveTo(cx, cy)
                ctx.lineTo(cx + r * 0.3, cy + r * 0.1)
                ctx.stroke()
                break

            case "PositionTweak":
                ctx.beginPath()
                ctx.moveTo(cx - r * 0.8, cy)
                ctx.lineTo(cx + r * 0.8, cy)
                ctx.moveTo(cx, cy - r * 0.8)
                ctx.lineTo(cx, cy + r * 0.8)
                ctx.stroke()
                var arr = r * 0.25
                ctx.beginPath()
                ctx.moveTo(cx + r * 0.8, cy)
                ctx.lineTo(cx + r * 0.8 - arr, cy - arr)
                ctx.moveTo(cx + r * 0.8, cy)
                ctx.lineTo(cx + r * 0.8 - arr, cy + arr)
                ctx.moveTo(cx, cy - r * 0.8)
                ctx.lineTo(cx - arr, cy - r * 0.8 + arr)
                ctx.moveTo(cx, cy - r * 0.8)
                ctx.lineTo(cx + arr, cy - r * 0.8 + arr)
                ctx.stroke()
                break

            case "ScaleTweak":
                ctx.strokeRect(cx - r * 0.8, cy - r * 0.8, r * 1.6, r * 1.6)
                ctx.strokeRect(cx - r * 0.4, cy - r * 0.4, r * 0.8, r * 0.8)
                break

            case "RotationTweak":
                ctx.beginPath()
                ctx.arc(cx, cy, r * 0.6, -Math.PI * 0.7, Math.PI * 0.5)
                ctx.stroke()
                var ax = cx + r * 0.6 * Math.cos(Math.PI * 0.5)
                var ay = cy + r * 0.6 * Math.sin(Math.PI * 0.5)
                ctx.beginPath()
                ctx.moveTo(ax, ay)
                ctx.lineTo(ax + 4, ay - 4)
                ctx.moveTo(ax, ay)
                ctx.lineTo(ax - 4, ay - 2)
                ctx.stroke()
                break

            case "ColorTweak":
                var segments = 6
                for (var i = 0; i < segments; i++) {
                    ctx.beginPath()
                    ctx.moveTo(cx, cy)
                    ctx.arc(cx, cy, r * 0.7, i * Math.PI * 2 / segments, (i + 1) * Math.PI * 2 / segments)
                    ctx.closePath()
                    ctx.stroke()
                }
                break

            case "PolarTweak":
                ctx.beginPath()
                ctx.arc(cx, cy, r * 0.4, 0, Math.PI * 2)
                ctx.arc(cx, cy, r * 0.8, 0, Math.PI * 2)
                ctx.stroke()
                for (var j = 0; j < 4; j++) {
                    var angle = j * Math.PI / 2
                    ctx.beginPath()
                    ctx.moveTo(cx, cy)
                    ctx.lineTo(cx + r * 0.8 * Math.cos(angle), cy + r * 0.8 * Math.sin(angle))
                    ctx.stroke()
                }
                break

            case "WaveTweak":
                ctx.beginPath()
                for (var wx = -r * 0.9; wx <= r * 0.9; wx += 2) {
                    var wy = Math.sin(wx / r * Math.PI * 2) * r * 0.4
                    if (wx === -r * 0.9) ctx.moveTo(cx + wx, cy + wy)
                    else ctx.lineTo(cx + wx, cy + wy)
                }
                ctx.stroke()
                break

            case "SqueezeTweak":
                ctx.beginPath()
                ctx.moveTo(cx - r * 0.8, cy)
                ctx.lineTo(cx - r * 0.2, cy)
                ctx.moveTo(cx + r * 0.8, cy)
                ctx.lineTo(cx + r * 0.2, cy)
                ctx.stroke()
                ctx.beginPath()
                ctx.moveTo(cx - r * 0.2, cy)
                ctx.lineTo(cx - r * 0.4, cy - r * 0.2)
                ctx.moveTo(cx - r * 0.2, cy)
                ctx.lineTo(cx - r * 0.4, cy + r * 0.2)
                ctx.moveTo(cx + r * 0.2, cy)
                ctx.lineTo(cx + r * 0.4, cy - r * 0.2)
                ctx.moveTo(cx + r * 0.2, cy)
                ctx.lineTo(cx + r * 0.4, cy + r * 0.2)
                ctx.stroke()
                break

            case "SparkleTweak":
                ctx.beginPath()
                ctx.moveTo(cx, cy - r * 0.8)
                ctx.lineTo(cx, cy + r * 0.8)
                ctx.moveTo(cx - r * 0.8, cy)
                ctx.lineTo(cx + r * 0.8, cy)
                ctx.stroke()
                ctx.beginPath()
                ctx.moveTo(cx - r * 0.5, cy - r * 0.5)
                ctx.lineTo(cx + r * 0.5, cy + r * 0.5)
                ctx.moveTo(cx + r * 0.5, cy - r * 0.5)
                ctx.lineTo(cx - r * 0.5, cy + r * 0.5)
                ctx.stroke()
                break
            }
        }
    }

    // Node name watermark
    Label {
        text: nodeData ? nodeData.displayName : ""
        color: Qt.rgba(root.baseColor.r, root.baseColor.g, root.baseColor.b, 0.5)
        font.pixelSize: 12
        visible: nodeData !== null

        // Tweaks: to the right; others: below
        anchors.left: isTweak ? root.right : undefined
        anchors.leftMargin: isTweak ? 4 : 0
        anchors.verticalCenter: isTweak ? root.verticalCenter : undefined
        anchors.top: isTweak ? undefined : root.bottom
        anchors.topMargin: isTweak ? 0 : 2
        anchors.horizontalCenter: isTweak ? undefined : root.horizontalCenter
    }

    // TOP PORTS (for Output node input, and Tweak frame input)
    Row {
        id: topPorts
        visible: hasTopPorts
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: -Theme.portRadius
        spacing: 8

        PortItem {
            port: !nodeData ? null : (nodeData.type === "Output" ? nodeData.inputAt(0) : (isTweak ? getFrameInput() : null))
            isInput: true
            showLabel: false
            connectionDragging: root.connectionDragging
            dragPosition: root.dragPosition

            onDragStarted: function(p, pos) { root.connectionDragStarted(p, pos) }
            onDragUpdated: function(pos) { root.connectionDragUpdated(pos) }
            onDragEnded: function(pos) { root.connectionDragEnded(pos) }
        }
    }

    // BOTTOM PORTS (for Input node output, and Tweak frame output)
    Row {
        id: bottomPorts
        visible: hasBottomPorts
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -Theme.portRadius
        spacing: 8

        PortItem {
            port: !nodeData ? null : (nodeData.type === "Input" ? nodeData.outputAt(0) : (isTweak ? getFrameOutput() : null))
            isInput: false
            showLabel: false
            connectionDragging: root.connectionDragging
            dragPosition: root.dragPosition

            onDragStarted: function(p, pos) { root.connectionDragStarted(p, pos) }
            onDragUpdated: function(pos) { root.connectionDragUpdated(pos) }
            onDragEnded: function(pos) { root.connectionDragEnded(pos) }
        }
    }

    // LEFT PORTS (for Shapes/Utility inputs, and Tweak ratio input — excluding Position)
    Column {
        id: leftPorts
        visible: hasLeftPorts
        anchors.left: parent.left
        anchors.leftMargin: -Theme.portRadius
        anchors.verticalCenter: parent.verticalCenter
        spacing: 4

        Repeater {
            model: (!nodeData || typeof nodeData.inputCount !== 'function') ? 0 : (isShapeOrUtility ? nodeData.inputCount() : (isTweak && getRatioInput() ? 1 : 0))

            delegate: PortItem {
                required property int index
                property var thePort: !nodeData ? null : (isShapeOrUtility ? nodeData.inputAt(index) : (isTweak ? getRatioInput() : null))
                visible: thePort && thePort.visible && thePort.dataType !== Port.DataType.Position
                port: thePort
                isInput: true
                showLabel: false
                connectionDragging: root.connectionDragging
                dragPosition: root.dragPosition

                onDragStarted: function(p, pos) { root.connectionDragStarted(p, pos) }
                onDragUpdated: function(pos) { root.connectionDragUpdated(pos) }
                onDragEnded: function(pos) { root.connectionDragEnded(pos) }
            }
        }
    }

    // RIGHT PORTS (for Shapes/Utility outputs — excluding Position)
    Column {
        id: rightPorts
        visible: hasRightPorts
        anchors.right: parent.right
        anchors.rightMargin: -Theme.portRadius
        anchors.verticalCenter: parent.verticalCenter
        spacing: 4

        Repeater {
            model: isShapeOrUtility && nodeData && typeof nodeData.outputCount === 'function' ? nodeData.outputCount() : 0

            delegate: PortItem {
                required property int index
                property var thePort: nodeData ? nodeData.outputAt(index) : null
                visible: thePort && thePort.dataType !== Port.DataType.Position
                port: thePort
                isInput: false
                showLabel: false
                connectionDragging: root.connectionDragging
                dragPosition: root.dragPosition

                onDragStarted: function(p, pos) { root.connectionDragStarted(p, pos) }
                onDragUpdated: function(pos) { root.connectionDragUpdated(pos) }
                onDragEnded: function(pos) { root.connectionDragEnded(pos) }
            }
        }
    }

    // POSITION INPUT PORT (for tweaks with center — left side, near top)
    PortItem {
        id: positionInputPort
        property var posPort: isTweak ? getPositionInput() : null
        visible: posPort !== null
        port: posPort
        isInput: true
        showLabel: false
        connectionDragging: root.connectionDragging
        dragPosition: root.dragPosition

        anchors.left: parent.left
        anchors.leftMargin: -Theme.portRadius
        anchors.top: parent.top
        anchors.topMargin: 8

        onDragStarted: function(p, pos) { root.connectionDragStarted(p, pos) }
        onDragUpdated: function(pos) { root.connectionDragUpdated(pos) }
        onDragEnded: function(pos) { root.connectionDragEnded(pos) }
    }

    // POSITION OUTPUT PORT (for emitters — right side, near top)
    PortItem {
        id: positionOutputPort
        property var posPort: isShapeOrUtility ? getPositionOutput() : null
        visible: posPort !== null
        port: posPort
        isInput: false
        showLabel: false
        connectionDragging: root.connectionDragging
        dragPosition: root.dragPosition

        anchors.right: parent.right
        anchors.rightMargin: -Theme.portRadius
        anchors.top: parent.top
        anchors.topMargin: 8

        onDragStarted: function(p, pos) { root.connectionDragStarted(p, pos) }
        onDragUpdated: function(pos) { root.connectionDragUpdated(pos) }
        onDragEnded: function(pos) { root.connectionDragEnded(pos) }
    }
}
