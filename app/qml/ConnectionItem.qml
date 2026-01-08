import QtQuick
import QtQuick.Controls
import QtQuick.Shapes
import GizmoTweakLib2
import GizmoTweak2

Item {
    id: root

    property Connection connection: null
    property bool selected: false

    signal deleteRequested()
    signal clicked()

    visible: connection !== null && connection.sourcePort !== null && connection.targetPort !== null

    // Convert scene coordinates to local (Flickable content) coordinates
    property point startPos: visible ? mapFromItem(null, connection.sourcePort.scenePosition.x, connection.sourcePort.scenePosition.y) : Qt.point(0, 0)
    property point endPos: visible ? mapFromItem(null, connection.targetPort.scenePosition.x, connection.targetPort.scenePosition.y) : Qt.point(0, 0)

    property color cableColor: !visible ? Theme.border : (connection.sourcePort.dataType === Port.DataType.Frame ? Theme.cableFrame : (connection.sourcePort.dataType === Port.DataType.Ratio2D ? Theme.cableRatio2D : (connection.sourcePort.dataType === Port.DataType.Ratio1D ? Theme.cableRatio1D : Theme.border)))

    // Recalculate on port position changes
    Connections {
        target: connection && connection.sourcePort ? connection.sourcePort : null
        function onScenePositionChanged() {
            if (root.visible) {
                root.startPos = root.mapFromItem(null, connection.sourcePort.scenePosition.x, connection.sourcePort.scenePosition.y)
            }
        }
    }

    Connections {
        target: connection && connection.targetPort ? connection.targetPort : null
        function onScenePositionChanged() {
            if (root.visible) {
                root.endPos = root.mapFromItem(null, connection.targetPort.scenePosition.x, connection.targetPort.scenePosition.y)
            }
        }
    }

    // Check if a point is near the bezier curve
    function isPointNearCurve(px, py, threshold) {
        // Sample points along the bezier curve and check distance
        var samples = 20
        var dx = Math.abs(endPos.x - startPos.x)
        var offset = Math.max(50, dx * 0.5)

        var p0x = startPos.x, p0y = startPos.y
        var p1x = startPos.x + offset, p1y = startPos.y
        var p2x = endPos.x - offset, p2y = endPos.y
        var p3x = endPos.x, p3y = endPos.y

        for (var i = 0; i <= samples; i++) {
            var t = i / samples
            var mt = 1 - t

            // Cubic bezier formula
            var bx = mt*mt*mt*p0x + 3*mt*mt*t*p1x + 3*mt*t*t*p2x + t*t*t*p3x
            var by = mt*mt*mt*p0y + 3*mt*mt*t*p1y + 3*mt*t*t*p2y + t*t*t*p3y

            var dist = Math.sqrt((px - bx)*(px - bx) + (py - by)*(py - by))
            if (dist < threshold) {
                return true
            }
        }
        return false
    }

    // Invisible hit area covering the bounding box
    MouseArea {
        id: hitArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true

        property bool isHovering: false

        onClicked: function(mouse) {
            if (isPointNearCurve(mouse.x, mouse.y, 10)) {
                if (mouse.button === Qt.LeftButton) {
                    root.clicked()
                } else if (mouse.button === Qt.RightButton) {
                    contextMenu.popup()
                }
            }
        }

        onPositionChanged: function(mouse) {
            isHovering = isPointNearCurve(mouse.x, mouse.y, 10)
        }

        onExited: {
            isHovering = false
        }
    }

    // The visible cable shape
    Shape {
        id: cableShape
        anchors.fill: parent

        ShapePath {
            strokeColor: root.selected ? Theme.accent : (hitArea.isHovering ? Qt.lighter(cableColor, 1.3) : cableColor)
            strokeWidth: root.selected ? Theme.cableWidth + 2 : (hitArea.isHovering ? Theme.cableWidth + 1 : Theme.cableWidth)
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap

            startX: startPos.x
            startY: startPos.y

            PathCubic {
                x: endPos.x
                y: endPos.y

                property real dx: Math.abs(endPos.x - startPos.x)
                property real offset: Math.max(50, dx * 0.5)

                control1X: startPos.x + offset
                control1Y: startPos.y
                control2X: endPos.x - offset
                control2Y: endPos.y
            }
        }
    }

    // Context menu
    Menu {
        id: contextMenu
        palette.window: Theme.surface
        palette.windowText: Theme.text
        palette.base: Theme.surface
        palette.text: Theme.text
        palette.highlight: Theme.menuHighlight
        palette.highlightedText: Theme.text
        palette.button: Theme.surface
        palette.buttonText: Theme.text
        palette.mid: Theme.border

        Action {
            text: qsTr("Delete")
            onTriggered: root.deleteRequested()
        }
    }
}
