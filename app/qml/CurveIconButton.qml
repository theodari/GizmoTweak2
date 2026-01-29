import QtQuick
import QtQuick.Controls

AbstractButton {
    id: root

    property int curveType: 0  // QEasingCurve::Type value
    property bool selected: false

    implicitWidth: 24
    implicitHeight: 24

    background: Rectangle {
        color: root.selected ? Theme.accent : (root.hovered ? Theme.surfaceHover : Theme.surface)
        border.color: root.selected ? Theme.accentBright : Theme.border
        border.width: root.selected ? 2 : 1
        radius: 3
    }

    contentItem: Item {
        Canvas {
            id: curveCanvas
            anchors.centerIn: parent
            width: Math.min(root.width, root.height) * 0.8
            height: width

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                var padding = width * 0.1
                var w = width - padding * 2
                var h = height - padding * 2

                ctx.strokeStyle = root.selected ? "#FFFFFF" : Theme.text
                ctx.lineWidth = 1.5
                ctx.beginPath()

                var steps = 20
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
                case 5: return t * t * t  // InCubic
                case 6: { var u = t - 1; return u * u * u + 1 }  // OutCubic
                case 7: return t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1  // InOutCubic
                case 9: return 1 - Math.cos(t * Math.PI / 2)  // InSine
                case 10: return Math.sin(t * Math.PI / 2)  // OutSine
                case 11: return 0.5 * (1 - Math.cos(Math.PI * t))  // InOutSine
                case 13: return t === 0 ? 0 : Math.pow(2, 10 * (t - 1))  // InExpo
                case 14: return t === 1 ? 1 : 1 - Math.pow(2, -10 * t)  // OutExpo
                case 15: {  // InOutExpo
                    if (t === 0) return 0
                    if (t === 1) return 1
                    if (t < 0.5) return 0.5 * Math.pow(2, 20 * t - 10)
                    return 1 - 0.5 * Math.pow(2, -20 * t + 10)
                }
                case 25: {  // InBounce
                    return 1 - bounceOut(1 - t)
                }
                case 26: return bounceOut(t)  // OutBounce
                case 27: {  // InOutBounce
                    if (t < 0.5) return 0.5 * (1 - bounceOut(1 - 2 * t))
                    return 0.5 * bounceOut(2 * t - 1) + 0.5
                }
                default: return t
                }
            }

            function bounceOut(t) {
                if (t < 1 / 2.75) return 7.5625 * t * t
                if (t < 2 / 2.75) { t -= 1.5 / 2.75; return 7.5625 * t * t + 0.75 }
                if (t < 2.5 / 2.75) { t -= 2.25 / 2.75; return 7.5625 * t * t + 0.9375 }
                t -= 2.625 / 2.75; return 7.5625 * t * t + 0.984375
            }

            onWidthChanged: requestPaint()

            Component.onCompleted: requestPaint()
        }
    }

    onSelectedChanged: curveCanvas.requestPaint()
    onHoveredChanged: curveCanvas.requestPaint()

    ToolTip.visible: enabled && hovered
    ToolTip.text: {
        var names = {
            0: "Linear",
            1: "In Quad", 2: "Out Quad", 3: "In/Out Quad",
            5: "In Cubic", 6: "Out Cubic", 7: "In/Out Cubic",
            9: "In Sine", 10: "Out Sine", 11: "In/Out Sine",
            13: "In Expo", 14: "Out Expo", 15: "In/Out Expo",
            25: "In Bounce", 26: "Out Bounce", 27: "In/Out Bounce"
        }
        return names[curveType] || ""
    }
    ToolTip.delay: 500
}
