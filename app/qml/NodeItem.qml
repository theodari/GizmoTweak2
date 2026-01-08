import QtQuick
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

Rectangle {
    id: root

    property Node nodeData: null
    required property NodeGraph graph
    property var canvas: null  // Reference to NodeCanvas for collision detection

    // Connection dragging state (passed from NodeCanvas)
    property bool connectionDragging: false
    property point dragPosition: Qt.point(0, 0)

    signal connectionDragStarted(var port, point mousePos)
    signal connectionDragUpdated(point mousePos)
    signal connectionDragEnded(point mousePos)  // Now passes position for hit-testing

    x: nodeData ? nodeData.position.x : 0
    y: nodeData ? nodeData.position.y : 0
    width: Theme.nodeMinWidth
    height: 50

    radius: Theme.nodeRadius
    border.color: nodeData && nodeData.selected ? Theme.accent : Theme.border
    border.width: nodeData && nodeData.selected ? 2 : 1

    // Node background color based on category and type
    color: {
        if (!nodeData) return Theme.surface
        if (nodeData.category === Node.Category.IO) return Theme.nodeIO
        if (nodeData.category === Node.Category.Shape) {
            return nodeData.type === "Group" ? Theme.nodeGroup : Theme.nodeShape
        }
        if (nodeData.category === Node.Category.Utility) return Theme.nodeUtility
        if (nodeData.category === Node.Category.Tweak) return Theme.nodeTweak
        return Theme.surface
    }

    // Determine layout type based on category
    readonly property bool isIONode: nodeData && (nodeData.category === Node.Category.IO)
    readonly property bool isShapeOrUtility: nodeData && (nodeData.category === Node.Category.Shape || nodeData.category === Node.Category.Utility)
    readonly property bool isTweak: nodeData && (nodeData.category === Node.Category.Tweak)

    // Determine which sides have ports
    readonly property bool hasTopPorts: nodeData && typeof nodeData.inputCount === 'function' ? (nodeData.type === "Output" ? nodeData.inputCount() > 0 : (isTweak ? getFrameInput() !== null : false)) : false
    readonly property bool hasBottomPorts: nodeData && typeof nodeData.outputCount === 'function' ? (nodeData.type === "Input" ? nodeData.outputCount() > 0 : (isTweak ? getFrameOutput() !== null : false)) : false
    readonly property bool hasLeftPorts: nodeData && typeof nodeData.inputCount === 'function' ? (isShapeOrUtility ? nodeData.inputCount() > 0 : (isTweak ? getRatioInput() !== null : false)) : false
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
            if (port && (port.dataType === Port.DataType.Ratio2D || port.dataType === Port.DataType.Ratio1D)) return port
        }
        return null
    }

    // Drag handling for the node itself (declared first so ports are on top)
    MouseArea {
        id: dragArea
        anchors.fill: parent
        drag.target: root
        drag.threshold: 0

        onPressed: function(mouse) {
            if (!(mouse.modifiers & Qt.ControlModifier)) {
                graph.clearSelection()
            }
            if (nodeData) nodeData.selected = true
        }

        onPositionChanged: {
            if (drag.active) {
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
            }
        }
    }

    // Node title
    Text {
        anchors.centerIn: parent
        text: nodeData ? (nodeData.displayName || nodeData.type) : ""
        color: Theme.text
        font.pixelSize: Theme.fontSizeNormal
        font.bold: true
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
                port: !nodeData ? null : (isShapeOrUtility ? nodeData.inputAt(index) : (isTweak ? getRatioInput() : null))
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
