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

    x: nodeData ? nodeData.position.x : 0
    y: nodeData ? nodeData.position.y : 0
    // Width: preview (70) + icon (30) + margins (4+4+4) = 112 for nodes with icon
    // IO nodes (Input/Output) stay at default width
    width: (isInputNode || (nodeData && nodeData.type === "Output")) ? Theme.nodeMinWidth : 112
    height: 78  // Preview size (70) + 4px top + 4px bottom

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

    // Drag handling for the node itself (declared first so ports are on top)
    MouseArea {
        id: dragArea
        anchors.fill: parent
        drag.target: root
        drag.threshold: 0
        hoverEnabled: true

        property bool isDragging: false

        onPressed: function(mouse) {
            // Clear connection selection when clicking on a node
            if (canvas) canvas.selectedConnection = null

            if (!(mouse.modifiers & Qt.ControlModifier)) {
                graph.clearSelection()
            }
            if (nodeData) {
                nodeData.selected = true
                if (canvas) {
                    canvas.selectedNode = nodeData
                    canvas.selectionChanged()
                    canvas.nodeClicked(nodeData)
                }
                // Track move start for undo
                graph.beginMoveNode(nodeData.uuid)
            }
        }

        onPositionChanged: {
            if (drag.active) {
                isDragging = true
                if (nodeData) nodeData.position = Qt.point(root.x, root.y)
            }
        }

        onReleased: {
            if (nodeData) {
                var desiredPos = Qt.point(root.x, root.y)

                // Use canvas collision detection if available
                if (canvas && typeof canvas.findValidPosition === 'function') {
                    var validPos = canvas.findValidPosition(nodeData.uuid, desiredPos, root.width, root.height)
                    nodeData.position = validPos
                } else {
                    // Fallback: just snap to grid
                    var gridSize = 20
                    nodeData.position = Qt.point(
                        Math.round(root.x / gridSize) * gridSize,
                        Math.round(root.y / gridSize) * gridSize
                    )
                }

                // Track move end for undo (only if we actually dragged)
                if (isDragging) {
                    graph.endMoveNode(nodeData.uuid, nodeData.position)
                    isDragging = false
                }
            }
        }

        onEntered: root.isHovering = true
        onExited: root.isHovering = false

        onDoubleClicked: root.doubleClicked()
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
        anchors.horizontalCenter: parent.horizontalCenter
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

    // LEFT PORTS (for Shapes/Utility inputs, and Tweak ratio input)
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
                visible: thePort && thePort.visible
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

    // RIGHT PORTS (for Shapes/Utility outputs)
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
                port: nodeData ? nodeData.outputAt(index) : null
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
}
