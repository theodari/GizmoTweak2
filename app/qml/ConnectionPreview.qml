import QtQuick
import QtQuick.Shapes
import GizmoTweakLib2
import GizmoTweak2

Shape {
    id: root

    property var startPort: null
    property point endPoint: Qt.point(0, 0)

    // Convert scene coordinates to local coordinates
    property point startPos: !startPort ? Qt.point(0, 0) : mapFromItem(null, startPort.scenePosition.x, startPort.scenePosition.y)
    property point localEndPoint: mapFromItem(null, endPoint.x, endPoint.y)

    ShapePath {
        strokeColor: Theme.cablePreview
        strokeWidth: Theme.cableWidth
        strokeStyle: ShapePath.DashLine
        dashPattern: [4, 4]
        fillColor: "transparent"
        capStyle: ShapePath.RoundCap

        startX: startPos.x
        startY: startPos.y

        PathCubic {
            x: localEndPoint.x
            y: localEndPoint.y

            property real dx: Math.abs(localEndPoint.x - startPos.x)
            property real offset: Math.max(50, dx * 0.5)

            control1X: startPos.x + offset
            control1Y: startPos.y
            control2X: localEndPoint.x - offset
            control2Y: localEndPoint.y
        }
    }
}
