import QtQuick
import QtQuick.Controls
import GizmoTweak2

Button {
    id: root

    property bool automationEnabled: false

    width: 20
    height: 20

    background: Rectangle {
        color: root.automationEnabled
            ? (root.hovered ? "#4080C0" : Theme.accent)
            : (root.hovered ? Theme.surfaceHover : Theme.surface)
        border.color: root.automationEnabled ? Theme.accentBright : Theme.border
        border.width: 1
        radius: 4
    }

    contentItem: Item {
        // Gear icon drawn with Canvas
        Canvas {
            id: gearCanvas
            anchors.centerIn: parent
            // Use button dimensions directly
            width: Math.min(root.width, root.height) * 0.8
            height: width

            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()

                var centerX = width / 2
                var centerY = height / 2

                // Scale all dimensions relative to canvas size
                var outerRadius = width / 2
                var innerRadius = outerRadius * 0.4
                var toothCount = 8
                var toothDepth = outerRadius * 0.25

                ctx.strokeStyle = root.automationEnabled ? "#FFFFFF" : Theme.text
                ctx.fillStyle = root.automationEnabled ? "#FFFFFF" : Theme.text
                ctx.lineWidth = 1

                // Draw gear teeth
                ctx.beginPath()
                for (var i = 0; i < toothCount; i++) {
                    var angle1 = (i / toothCount) * 2 * Math.PI - Math.PI / 2
                    var angle2 = ((i + 0.35) / toothCount) * 2 * Math.PI - Math.PI / 2
                    var angle3 = ((i + 0.65) / toothCount) * 2 * Math.PI - Math.PI / 2
                    var angle4 = ((i + 1) / toothCount) * 2 * Math.PI - Math.PI / 2

                    var r1 = outerRadius - toothDepth
                    var r2 = outerRadius

                    if (i === 0) {
                        ctx.moveTo(centerX + r1 * Math.cos(angle1), centerY + r1 * Math.sin(angle1))
                    }
                    ctx.lineTo(centerX + r2 * Math.cos(angle2), centerY + r2 * Math.sin(angle2))
                    ctx.lineTo(centerX + r2 * Math.cos(angle3), centerY + r2 * Math.sin(angle3))
                    ctx.lineTo(centerX + r1 * Math.cos(angle4), centerY + r1 * Math.sin(angle4))
                }
                ctx.closePath()
                ctx.fill()

                // Draw center hole
                ctx.globalCompositeOperation = "destination-out"
                ctx.beginPath()
                ctx.arc(centerX, centerY, innerRadius, 0, 2 * Math.PI)
                ctx.fill()

                // Draw center circle outline
                ctx.globalCompositeOperation = "source-over"
                ctx.beginPath()
                ctx.arc(centerX, centerY, innerRadius, 0, 2 * Math.PI)
                ctx.stroke()
            }

            onWidthChanged: requestPaint()

            Connections {
                target: root
                function onAutomationEnabledChanged() { gearCanvas.requestPaint() }
                function onHoveredChanged() { gearCanvas.requestPaint() }
            }
        }
    }

    ToolTip.visible: enabled && hovered
    ToolTip.text: automationEnabled ? qsTr("Disable automation") : qsTr("Enable automation")
    ToolTip.delay: 500
}
