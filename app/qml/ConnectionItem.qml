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

    // Get scene positions from ports
    property point sourceScenePos: visible ? connection.sourcePort.scenePosition : Qt.point(0, 0)
    property point targetScenePos: visible ? connection.targetPort.scenePosition : Qt.point(0, 0)

    // Compute bounding box with padding for bezier curve control points
    property real padding: 60
    property real minX: Math.min(sourceScenePos.x, targetScenePos.x) - padding
    property real minY: Math.min(sourceScenePos.y, targetScenePos.y) - padding
    property real maxX: Math.max(sourceScenePos.x, targetScenePos.x) + padding
    property real maxY: Math.max(sourceScenePos.y, targetScenePos.y) + padding

    // Position and size the item to cover the bounding box
    x: minX
    y: minY
    width: maxX - minX
    height: maxY - minY

    // Local coordinates relative to this item
    property point startPos: Qt.point(sourceScenePos.x - minX, sourceScenePos.y - minY)
    property point endPos: Qt.point(targetScenePos.x - minX, targetScenePos.y - minY)

    property color cableColor: !visible ? Theme.border : (connection.sourcePort.dataType === Port.DataType.Frame ? Theme.cableFrame : (connection.sourcePort.dataType === Port.DataType.Ratio2D ? Theme.cableRatio2D : (connection.sourcePort.dataType === Port.DataType.Ratio1D ? Theme.cableRatio1D : Theme.border)))

    // Visual properties based on state
    property color displayColor: selected ? Qt.lighter(cableColor, 1.4) : (hitArea.isHovering ? Qt.lighter(cableColor, 1.2) : cableColor)
    property int displayWidth: selected ? Theme.cableWidth + 3 : (hitArea.isHovering ? Theme.cableWidth + 1 : Theme.cableWidth)

    // Check if a point is near the bezier curve
    function isPointNearCurve(px, py, threshold) {
        var samples = 25
        var dx = Math.abs(endPos.x - startPos.x)
        var offset = Math.max(50, dx * 0.5)

        var p0x = startPos.x, p0y = startPos.y
        var p1x = startPos.x + offset, p1y = startPos.y
        var p2x = endPos.x - offset, p2y = endPos.y
        var p3x = endPos.x, p3y = endPos.y

        for (var i = 0; i <= samples; i++) {
            var t = i / samples
            var mt = 1 - t

            var bx = mt*mt*mt*p0x + 3*mt*mt*t*p1x + 3*mt*t*t*p2x + t*t*t*p3x
            var by = mt*mt*mt*p0y + 3*mt*mt*t*p1y + 3*mt*t*t*p2y + t*t*t*p3y

            var dist = Math.sqrt((px - bx)*(px - bx) + (py - by)*(py - by))
            if (dist < threshold) {
                return true
            }
        }
        return false
    }

    // Hit area covering the item
    MouseArea {
        id: hitArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true

        property bool isHovering: false

        onClicked: function(mouse) {
            if (isPointNearCurve(mouse.x, mouse.y, 12)) {
                if (mouse.button === Qt.LeftButton) {
                    root.clicked()
                } else if (mouse.button === Qt.RightButton) {
                    contextMenu.popup()
                }
            }
        }

        onPositionChanged: function(mouse) {
            isHovering = isPointNearCurve(mouse.x, mouse.y, 12)
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
            strokeColor: displayColor
            strokeWidth: displayWidth
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
