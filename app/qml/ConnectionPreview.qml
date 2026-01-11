import QtQuick
import QtQuick.Shapes
import GizmoTweakLib2
import GizmoTweak2

Shape {
    id: root

    property var startPort: null
    property point endPoint: Qt.point(0, 0)

    // Use coordinates directly (already in Flickable content space)
    property point startPos: !startPort ? Qt.point(0, 0) : startPort.scenePosition
    property point localEndPoint: endPoint

    // Determine if this is a vertical connection (Frame type)
    // Use effectiveDataType for RatioAny ports
    property int effectiveType: startPort ? startPort.effectiveDataType : Port.DataType.Frame
    property bool isVertical: startPort && effectiveType === Port.DataType.Frame

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
            property real dy: Math.abs(localEndPoint.y - startPos.y)
            property real offset: isVertical ? Math.max(50, dy * 0.5) : Math.max(50, dx * 0.5)

            control1X: isVertical ? startPos.x : startPos.x + offset
            control1Y: isVertical ? startPos.y + offset : startPos.y
            control2X: isVertical ? localEndPoint.x : localEndPoint.x - offset
            control2Y: isVertical ? localEndPoint.y - offset : localEndPoint.y
        }
    }
}
