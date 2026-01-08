import QtQuick
import QtQuick.Shapes
import GizmoTweakLib2
import GizmoTweak2

Shape {
    id: root

    property Connection connection: null

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

    ShapePath {
        strokeColor: cableColor
        strokeWidth: Theme.cableWidth
        fillColor: "transparent"
        capStyle: ShapePath.RoundCap

        startX: startPos.x
        startY: startPos.y

        PathCubic {
            x: endPos.x
            y: endPos.y

            // Control points for smooth Bézier curve
            property real dx: Math.abs(endPos.x - startPos.x)
            property real offset: Math.max(50, dx * 0.5)

            control1X: startPos.x + offset
            control1Y: startPos.y
            control2X: endPos.x - offset
            control2Y: endPos.y
        }
    }
}
