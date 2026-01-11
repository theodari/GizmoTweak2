import QtQuick
import QtQuick.Controls

AbstractButton {
    id: root

    property int curveType: 0  // 0=Linear, 1=InQuad, 2=OutQuad, etc.
    property bool selected: false

    implicitWidth: 24
    implicitHeight: 24

    background: Rectangle {
        color: root.selected ? Theme.accent : (root.hovered ? Theme.surfaceHover : Theme.surface)
        border.color: root.selected ? Theme.accentBright : Theme.border
        border.width: root.selected ? 2 : 1
        radius: 3
    }

    contentItem: Canvas {
        id: curveCanvas

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var padding = 4
            var w = width - padding * 2
            var h = height - padding * 2

            ctx.strokeStyle = root.selected ? "#FFFFFF" : Theme.text
            ctx.lineWidth = 1.5
            ctx.beginPath()

            // Draw curve based on type
            var steps = 16
            for (var i = 0; i <= steps; i++) {
                var t = i / steps
                var y = easingValue(t, root.curveType)

                var px = padding + t * w
                var py = padding + (1 - y) * h

                if (i === 0)
                    ctx.moveTo(px, py)
                else
                    ctx.lineTo(px, py)
            }
            ctx.stroke()
        }

        function easingValue(t, type) {
            switch (type) {
            case 0: return t  // Linear
            case 1: return t * t  // InQuad
            case 2: return t * (2 - t)  // OutQuad
            case 3: return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t  // InOutQuad
            case 4: return t * t * t  // InCubic
            case 5: return (--t) * t * t + 1  // OutCubic
            case 6: return t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1  // InOutCubic
            case 7: return 1 - Math.cos(t * Math.PI / 2)  // InSine
            case 8: return Math.sin(t * Math.PI / 2)  // OutSine
            case 9: return 0.5 * (1 - Math.cos(Math.PI * t))  // InOutSine
            default: return t
            }
        }

        Component.onCompleted: requestPaint()
    }

    onSelectedChanged: curveCanvas.requestPaint()
    onHoveredChanged: curveCanvas.requestPaint()

    ToolTip.visible: enabled && hovered
    ToolTip.text: {
        var names = ["Linear", "In Quad", "Out Quad", "In/Out Quad",
                     "In Cubic", "Out Cubic", "In/Out Cubic",
                     "In Sine", "Out Sine", "In/Out Sine"]
        return names[curveType] || ""
    }
    ToolTip.delay: 500
}
