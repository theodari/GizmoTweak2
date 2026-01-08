import QtQuick
import QtQuick.Controls
import QtQuick.Shapes
import GizmoTweakLib2
import GizmoTweak2

Item {
    id: root

    required property NodeGraph graph
    property bool showGrid: false

    // Canvas properties
    property real canvasWidth: 4000
    property real canvasHeight: 3000
    property real gridSize: 20

    // Connection preview
    property var dragStartPort: null
    property point dragEndPoint: Qt.point(0, 0)
    property bool isDraggingConnection: false

    // Context menu position
    property point contextMenuPosition: Qt.point(0, 0)

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
        contentWidth: canvasWidth
        contentHeight: canvasHeight
        clip: true

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

            // Mouse area for clicks and context menu
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onClicked: function(mouse) {
                    if (mouse.button === Qt.LeftButton) {
                        graph.clearSelection()
                    }
                }

                onPressed: function(mouse) {
                    if (mouse.button === Qt.RightButton) {
                        // Store position in canvas coordinates
                        contextMenuPosition = Qt.point(mouse.x, mouse.y)
                        contextMenu.popup()
                    }
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
                        var result = graph.connect(root.dragStartPort, targetPort)
                        if (result) {
                            console.log("Connection created!")
                        } else {
                            console.log("Connection failed - incompatible ports")
                        }
                    }
                    root.isDraggingConnection = false
                    root.dragStartPort = null
                }
            }
        }
    }

    // Context menu for creating nodes
    Menu {
        id: contextMenu
        palette.window: Theme.surface
        palette.windowText: Theme.text
        palette.highlight: Theme.menuHighlight
        palette.highlightedText: Theme.text

        Menu {
            title: qsTr("Shapes")
            palette.window: Theme.surface
            palette.windowText: Theme.text
            palette.highlight: Theme.menuHighlight
            palette.highlightedText: Theme.text

            Action {
                text: qsTr("Gizmo")
                onTriggered: createNodeAt("Gizmo")
            }

            Action {
                text: qsTr("Group")
                onTriggered: createNodeAt("Group")
            }

            Action {
                text: qsTr("SurfaceFactory")
                onTriggered: createNodeAt("SurfaceFactory")
            }
        }

        Menu {
            title: qsTr("Tweaks")
            palette.window: Theme.surface
            palette.windowText: Theme.text
            palette.highlight: Theme.menuHighlight
            palette.highlightedText: Theme.text

            Action {
                text: qsTr("Position")
                onTriggered: createNodeAt("PositionTweak")
            }

            Action {
                text: qsTr("Scale")
                onTriggered: createNodeAt("ScaleTweak")
            }

            Action {
                text: qsTr("Rotation")
                onTriggered: createNodeAt("RotationTweak")
            }

            Action {
                text: qsTr("Color")
                onTriggered: createNodeAt("ColorTweak")
            }
        }

        Menu {
            title: qsTr("Utility")
            palette.window: Theme.surface
            palette.windowText: Theme.text
            palette.highlight: Theme.menuHighlight
            palette.highlightedText: Theme.text

            Action {
                text: qsTr("TimeShift")
                onTriggered: createNodeAt("TimeShift")
            }
        }
    }

    function createNodeAt(nodeType) {
        // Snap to grid
        var snappedX = Math.round(contextMenuPosition.x / gridSize) * gridSize
        var snappedY = Math.round(contextMenuPosition.y / gridSize) * gridSize

        graph.createNode(nodeType, Qt.point(snappedX, snappedY))
    }

    // Initialize connections model from existing graph connections
    Component.onCompleted: {
        var conns = graph.connections
        for (var i = 0; i < conns.length; i++) {
            connectionsModel.append({ "connectionObj": conns[i] })
        }
    }

    // Connection preview while dragging (outside Flickable to use scene coordinates)
    ConnectionPreview {
        id: connectionPreview
        visible: isDraggingConnection
        startPort: dragStartPort
        endPoint: dragEndPoint
        x: 0
        y: 0
        width: parent.width
        height: parent.height
    }
}
