import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweak2

Rectangle {
    id: toolbar
    color: Theme.backgroundLight
    height: 48

    // Signal for status bar hover instructions
    signal hoverChanged(string message)

    // Signals for drag proxy (handled by Main.qml)
    signal dragStarted(string nodeType, string nodeLabel, real mouseX, real mouseY)
    signal dragMoved(real mouseX, real mouseY)
    signal dragEnded()

    // Signals for zoom tools
    signal zoomInRequested()
    signal zoomOutRequested()
    signal resetZoomRequested()
    signal zoomToFitRequested()
    signal zoomAreaRequested()

    // Signals for laser control
    signal laserToggled(bool enabled)
    signal zoneSelected(int index)

    // Signals for transport controls
    signal playToggled()
    signal rewindRequested()
    signal stopRequested()
    signal loopToggled()
    signal scrubChanged(int timeMs)

    // Zoom area mode state (set from outside)
    property bool zoomAreaActive: false

    // Laser state
    property bool laserEnabled: false

    // Available zones model (set from outside)
    property var zonesModel: []
    property int currentZoneIndex: 0

    // Favorite nodes (most frequently used, set from Main.qml)
    property var favoriteNodeTypes: []

    // Transport state (set from outside)
    property bool isPlaying: false
    property bool isLooping: true
    property int playLocatorMs: 0
    property int animationDurationMs: 10000

    function isFavorite(nodeType) {
        return favoriteNodeTypes.indexOf(nodeType) >= 0
    }

    // Model of available node types
    property var nodeTypes: [
        { type: "Gizmo", label: "Gizmo", shortcut: "G", hint: "Drag to create a Gizmo shape" },
        { type: "Transform", label: "Transform", shortcut: "Tr", hint: "Drag to create a Transform (combines/transforms shapes)" },
        { type: "SurfaceFactory", label: "Surface", shortcut: "SF", hint: "Drag to create a SurfaceFactory" },
        { type: "Mirror", label: "Mirror", shortcut: "M", hint: "Drag to create a Mirror (duplicates shape)" },
        { type: "TimeShift", label: "TimeShift", shortcut: "T", hint: "Drag to create a TimeShift utility" },
        { type: "PositionTweak", label: "Position", shortcut: "P", hint: "Drag to create a Position tweak" },
        { type: "ScaleTweak", label: "Scale", shortcut: "S", hint: "Drag to create a Scale tweak" },
        { type: "RotationTweak", label: "Rotation", shortcut: "R", hint: "Drag to create a Rotation tweak" },
        { type: "ColorTweak", label: "Color", shortcut: "C", hint: "Drag to create a Color tweak" },
        { type: "PolarTweak", label: "Polar", shortcut: "Po", hint: "Drag to create a Polar distortion tweak" },
        { type: "WaveTweak", label: "Wave", shortcut: "W", hint: "Drag to create a Wave ripple tweak" },
        { type: "SqueezeTweak", label: "Squeeze", shortcut: "Sq", hint: "Drag to create a Squeeze/Stretch hyperbolic tweak" },
        { type: "SparkleTweak", label: "Sparkle", shortcut: "Sk", hint: "Drag to create a Sparkle effect tweak" }
    ]

    function colorForType(nodeType) {
        switch (nodeType) {
            case "Gizmo": return Theme.nodeGizmo
            case "Transform": return Theme.nodeTransform
            case "SurfaceFactory": return Theme.nodeSurface
            case "PositionTweak":
            case "ScaleTweak":
            case "RotationTweak":
            case "ColorTweak":
            case "PolarTweak":
            case "WaveTweak":
            case "SqueezeTweak":
            case "SparkleTweak": return Theme.nodeTweak
            case "TimeShift":
            case "Mirror": return Theme.nodeUtility
            default: return Theme.surface
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 8

        // Zoom tools
        RowLayout {
            spacing: 2

            // Zoom Out
            ToolButton {
                id: zoomOutBtn
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize

                background: Rectangle {
                    radius: 4
                    color: zoomOutBtn.pressed ? Theme.surfacePressed : (zoomOutBtn.hovered ? Theme.surfaceHover : Theme.surface)
                    border.color: zoomOutBtn.hovered ? Theme.accent : Theme.border
                }

                contentItem: Text {
                    text: "\u2212"  // Unicode minus
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: toolbar.zoomOutRequested()
                onHoveredChanged: toolbar.hoverChanged(hovered ? qsTr("Zoom out") : "")

                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Zoom out (Ctrl+-)")
                ToolTip.delay: 500
            }

            // Zoom In
            ToolButton {
                id: zoomInBtn
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize

                background: Rectangle {
                    radius: 4
                    color: zoomInBtn.pressed ? Theme.surfacePressed : (zoomInBtn.hovered ? Theme.surfaceHover : Theme.surface)
                    border.color: zoomInBtn.hovered ? Theme.accent : Theme.border
                }

                contentItem: Text {
                    text: "+"
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: toolbar.zoomInRequested()
                onHoveredChanged: toolbar.hoverChanged(hovered ? qsTr("Zoom in") : "")

                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Zoom in (Ctrl++)")
                ToolTip.delay: 500
            }

            // Reset Zoom (1:1)
            ToolButton {
                id: resetZoomBtn
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize

                background: Rectangle {
                    radius: 4
                    color: resetZoomBtn.pressed ? Theme.surfacePressed : (resetZoomBtn.hovered ? Theme.surfaceHover : Theme.surface)
                    border.color: resetZoomBtn.hovered ? Theme.accent : Theme.border
                }

                contentItem: Text {
                    text: "1:1"
                    font.pixelSize: 11
                    font.bold: true
                    color: Theme.text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: toolbar.resetZoomRequested()
                onHoveredChanged: toolbar.hoverChanged(hovered ? qsTr("Reset zoom to 100%") : "")

                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Reset zoom to 100% (Ctrl+0)")
                ToolTip.delay: 500
            }

            // Zoom to Fit
            ToolButton {
                id: zoomFitBtn
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize

                background: Rectangle {
                    radius: 4
                    color: zoomFitBtn.pressed ? Theme.surfacePressed : (zoomFitBtn.hovered ? Theme.surfaceHover : Theme.surface)
                    border.color: zoomFitBtn.hovered ? Theme.accent : Theme.border
                }

                contentItem: Text {
                    text: "Fit"
                    font.pixelSize: 11
                    font.bold: true
                    color: Theme.text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: toolbar.zoomToFitRequested()
                onHoveredChanged: toolbar.hoverChanged(hovered ? qsTr("Zoom to fit all nodes") : "")

                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Zoom to fit all nodes (Ctrl+F)")
                ToolTip.delay: 500
            }

            // Zoom Area Selection
            ToolButton {
                id: zoomAreaBtn
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize
                checkable: true
                checked: toolbar.zoomAreaActive

                background: Rectangle {
                    radius: 4
                    color: zoomAreaBtn.checked ? Theme.accent : (zoomAreaBtn.pressed ? Theme.surfacePressed : (zoomAreaBtn.hovered ? Theme.surfaceHover : Theme.surface))
                    border.color: zoomAreaBtn.checked ? Theme.accentBright : (zoomAreaBtn.hovered ? Theme.accent : Theme.border)
                    border.width: zoomAreaBtn.checked ? 2 : 1
                }

                contentItem: Item {
                    // Draw a rectangle icon
                    Rectangle {
                        anchors.centerIn: parent
                        width: 16
                        height: 16
                        color: "transparent"
                        border.color: Theme.text
                        border.width: 1.5
                        radius: 1

                        // Inner fill when active
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            color: zoomAreaBtn.checked ? Qt.rgba(1,1,1,0.4) : "transparent"
                            radius: 1
                        }
                    }
                }

                onClicked: toolbar.zoomAreaRequested()
                onHoveredChanged: toolbar.hoverChanged(hovered ? qsTr("Select area to zoom (click & drag)") : "")

                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Select area to zoom")
                ToolTip.delay: 500
            }
        }

        // Left spacer (limited width)
        Item { Layout.preferredWidth: 60; Layout.maximumWidth: 60 }

        // Node buttons
        Repeater {
            model: nodeTypes

            delegate: Item {
                id: nodeButton
                // All node buttons same size
                width: 40
                height: 36

                required property var modelData
                required property int index

                Rectangle {
                    id: buttonRect
                    anchors.fill: parent
                    radius: 4
                    color: toolbar.colorForType(nodeButton.modelData.type)
                    border.color: dragArea.containsMouse ? Theme.accent : Qt.darker(color, 1.2)
                    border.width: dragArea.containsMouse ? 2 : 1

                    // Icon canvas for each node type
                    Canvas {
                        id: buttonIcon
                        anchors.centerIn: parent
                        width: 24
                        height: width

                        property string nodeType: nodeButton.modelData.type
                        property color iconColor: Qt.darker(buttonRect.color, 1.6)

                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.reset()

                            var cx = width / 2, cy = height / 2
                            var r = Math.min(width, height) / 2 - 2

                            ctx.strokeStyle = iconColor
                            ctx.fillStyle = iconColor
                            ctx.lineWidth = 2
                            ctx.lineCap = "round"
                            ctx.lineJoin = "round"

                            switch (nodeType) {
                            case "Gizmo":
                                // Draw ellipse using scale transform
                                ctx.save()
                                ctx.translate(cx, cy)
                                ctx.scale(1, 0.7)
                                ctx.beginPath()
                                ctx.arc(0, 0, r, 0, Math.PI * 2)
                                ctx.restore()
                                ctx.stroke()
                                // Highlight arc
                                ctx.lineWidth = 1.5
                                ctx.beginPath()
                                ctx.arc(cx - r * 0.3, cy - r * 0.2, r * 0.4, Math.PI * 1.2, Math.PI * 1.8)
                                ctx.stroke()
                                break

                            case "Transform":
                                // NAND gate schematic (transform/combine)
                                ctx.beginPath()
                                ctx.moveTo(cx - r * 0.6, cy - r * 0.6)
                                ctx.lineTo(cx - r * 0.6, cy + r * 0.6)
                                ctx.lineTo(cx, cy + r * 0.6)
                                ctx.arc(cx, cy, r * 0.6, Math.PI / 2, -Math.PI / 2, true)
                                ctx.closePath()
                                ctx.stroke()
                                // Output bubble
                                ctx.beginPath()
                                ctx.arc(cx + r * 0.75, cy, r * 0.15, 0, Math.PI * 2)
                                ctx.stroke()
                                break

                            case "SurfaceFactory":
                                // Wave/curve symbol
                                ctx.beginPath()
                                ctx.moveTo(cx - r, cy)
                                ctx.bezierCurveTo(cx - r * 0.5, cy - r, cx + r * 0.5, cy + r, cx + r, cy)
                                ctx.stroke()
                                // Dots at ends
                                ctx.beginPath()
                                ctx.arc(cx - r, cy, 2, 0, Math.PI * 2)
                                ctx.arc(cx + r, cy, 2, 0, Math.PI * 2)
                                ctx.fill()
                                break

                            case "Mirror":
                                // Two mirrored triangles
                                ctx.beginPath()
                                ctx.moveTo(cx - r * 0.8, cy - r * 0.5)
                                ctx.lineTo(cx - r * 0.2, cy)
                                ctx.lineTo(cx - r * 0.8, cy + r * 0.5)
                                ctx.closePath()
                                ctx.fill()
                                ctx.beginPath()
                                ctx.moveTo(cx + r * 0.8, cy - r * 0.5)
                                ctx.lineTo(cx + r * 0.2, cy)
                                ctx.lineTo(cx + r * 0.8, cy + r * 0.5)
                                ctx.closePath()
                                ctx.fill()
                                // Dashed center line
                                ctx.setLineDash([2, 2])
                                ctx.beginPath()
                                ctx.moveTo(cx, cy - r)
                                ctx.lineTo(cx, cy + r)
                                ctx.stroke()
                                ctx.setLineDash([])
                                break

                            case "TimeShift":
                                // Clock circle
                                ctx.beginPath()
                                ctx.arc(cx, cy, r * 0.7, 0, Math.PI * 2)
                                ctx.stroke()
                                // Clock hands
                                ctx.beginPath()
                                ctx.moveTo(cx, cy)
                                ctx.lineTo(cx, cy - r * 0.4)
                                ctx.moveTo(cx, cy)
                                ctx.lineTo(cx + r * 0.3, cy + r * 0.1)
                                ctx.stroke()
                                break

                            case "PositionTweak":
                                // Cross arrows
                                ctx.beginPath()
                                ctx.moveTo(cx - r * 0.8, cy)
                                ctx.lineTo(cx + r * 0.8, cy)
                                ctx.moveTo(cx, cy - r * 0.8)
                                ctx.lineTo(cx, cy + r * 0.8)
                                ctx.stroke()
                                // Arrowheads
                                var arr = r * 0.25
                                ctx.beginPath()
                                ctx.moveTo(cx + r * 0.8, cy)
                                ctx.lineTo(cx + r * 0.8 - arr, cy - arr)
                                ctx.moveTo(cx + r * 0.8, cy)
                                ctx.lineTo(cx + r * 0.8 - arr, cy + arr)
                                ctx.moveTo(cx, cy - r * 0.8)
                                ctx.lineTo(cx - arr, cy - r * 0.8 + arr)
                                ctx.moveTo(cx, cy - r * 0.8)
                                ctx.lineTo(cx + arr, cy - r * 0.8 + arr)
                                ctx.stroke()
                                break

                            case "ScaleTweak":
                                // Concentric squares
                                ctx.strokeRect(cx - r * 0.8, cy - r * 0.8, r * 1.6, r * 1.6)
                                ctx.strokeRect(cx - r * 0.4, cy - r * 0.4, r * 0.8, r * 0.8)
                                break

                            case "RotationTweak":
                                // Circular arrow
                                ctx.beginPath()
                                ctx.arc(cx, cy, r * 0.6, -Math.PI * 0.7, Math.PI * 0.5)
                                ctx.stroke()
                                // Arrowhead
                                var ax = cx + r * 0.6 * Math.cos(Math.PI * 0.5)
                                var ay = cy + r * 0.6 * Math.sin(Math.PI * 0.5)
                                ctx.beginPath()
                                ctx.moveTo(ax, ay)
                                ctx.lineTo(ax + 4, ay - 4)
                                ctx.moveTo(ax, ay)
                                ctx.lineTo(ax - 4, ay - 2)
                                ctx.stroke()
                                break

                            case "ColorTweak":
                                // Color wheel segments
                                var segments = 6
                                for (var i = 0; i < segments; i++) {
                                    ctx.beginPath()
                                    ctx.moveTo(cx, cy)
                                    ctx.arc(cx, cy, r * 0.7, i * Math.PI * 2 / segments, (i + 1) * Math.PI * 2 / segments)
                                    ctx.closePath()
                                    ctx.stroke()
                                }
                                break

                            case "PolarTweak":
                                // Polar grid
                                ctx.beginPath()
                                ctx.arc(cx, cy, r * 0.4, 0, Math.PI * 2)
                                ctx.arc(cx, cy, r * 0.8, 0, Math.PI * 2)
                                ctx.stroke()
                                // Radial lines
                                for (var j = 0; j < 4; j++) {
                                    var angle = j * Math.PI / 2
                                    ctx.beginPath()
                                    ctx.moveTo(cx, cy)
                                    ctx.lineTo(cx + r * 0.8 * Math.cos(angle), cy + r * 0.8 * Math.sin(angle))
                                    ctx.stroke()
                                }
                                break

                            case "WaveTweak":
                                // Sine wave
                                ctx.beginPath()
                                for (var wx = -r * 0.9; wx <= r * 0.9; wx += 2) {
                                    var wy = Math.sin(wx / r * Math.PI * 2) * r * 0.4
                                    if (wx === -r * 0.9) ctx.moveTo(cx + wx, cy + wy)
                                    else ctx.lineTo(cx + wx, cy + wy)
                                }
                                ctx.stroke()
                                break

                            case "SqueezeTweak":
                                // Pinch arrows
                                ctx.beginPath()
                                ctx.moveTo(cx - r * 0.8, cy)
                                ctx.lineTo(cx - r * 0.2, cy)
                                ctx.moveTo(cx + r * 0.8, cy)
                                ctx.lineTo(cx + r * 0.2, cy)
                                ctx.stroke()
                                // Arrow tips pointing inward
                                ctx.beginPath()
                                ctx.moveTo(cx - r * 0.2, cy)
                                ctx.lineTo(cx - r * 0.4, cy - r * 0.2)
                                ctx.moveTo(cx - r * 0.2, cy)
                                ctx.lineTo(cx - r * 0.4, cy + r * 0.2)
                                ctx.moveTo(cx + r * 0.2, cy)
                                ctx.lineTo(cx + r * 0.4, cy - r * 0.2)
                                ctx.moveTo(cx + r * 0.2, cy)
                                ctx.lineTo(cx + r * 0.4, cy + r * 0.2)
                                ctx.stroke()
                                break

                            case "SparkleTweak":
                                // 4-pointed star
                                ctx.beginPath()
                                ctx.moveTo(cx, cy - r * 0.8)
                                ctx.lineTo(cx, cy + r * 0.8)
                                ctx.moveTo(cx - r * 0.8, cy)
                                ctx.lineTo(cx + r * 0.8, cy)
                                ctx.stroke()
                                // Diagonal shorter
                                ctx.beginPath()
                                ctx.moveTo(cx - r * 0.5, cy - r * 0.5)
                                ctx.lineTo(cx + r * 0.5, cy + r * 0.5)
                                ctx.moveTo(cx + r * 0.5, cy - r * 0.5)
                                ctx.lineTo(cx - r * 0.5, cy + r * 0.5)
                                ctx.stroke()
                                break
                            }
                        }
                    }

                    // Favorite star indicator
                    Text {
                        visible: toolbar.isFavorite(nodeButton.modelData.type)
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 2
                        text: "\u2605"  // Filled star
                        color: "#FFD700"  // Gold
                        font.pixelSize: 10
                    }

                    // Visual feedback during drag
                    opacity: dragArea.isDragging ? 0.5 : 1.0
                }

                // Drag handler
                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: containsPress ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                    property bool isDragging: false

                    onEntered: toolbar.hoverChanged(nodeButton.modelData.hint)
                    onExited: if (!isDragging) toolbar.hoverChanged("")

                    onPressed: function(mouse) {
                        isDragging = true
                        var globalPos = mapToItem(null, mouse.x, mouse.y)
                        toolbar.dragStarted(nodeButton.modelData.type, nodeButton.modelData.label, globalPos.x, globalPos.y)
                    }

                    onPositionChanged: function(mouse) {
                        if (isDragging) {
                            var globalPos = mapToItem(null, mouse.x, mouse.y)
                            toolbar.dragMoved(globalPos.x, globalPos.y)
                        }
                    }

                    onReleased: function(mouse) {
                        if (isDragging) {
                            isDragging = false
                            toolbar.dragEnded()
                            toolbar.hoverChanged("")
                        }
                    }

                    ToolTip.visible: nodeButton.modelData && containsMouse && !isDragging
                    ToolTip.text: nodeButton.modelData ? (nodeButton.modelData.label + " - " + nodeButton.modelData.hint) : ""
                    ToolTip.delay: 300
                }
            }
        }

        // Right spacer
        Item { Layout.fillWidth: true }

        // Transport controls
        RowLayout {
            spacing: 2

            // Rewind
            ToolButton {
                id: rewindBtn
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize

                background: Rectangle {
                    radius: 4
                    color: rewindBtn.pressed ? Theme.surfacePressed : (rewindBtn.hovered ? Theme.surfaceHover : Theme.surface)
                    border.color: rewindBtn.hovered ? Theme.accent : Theme.border
                }

                contentItem: Canvas {
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        var s = Math.min(width, height) * 0.8
                        var ox = (width - s) / 2, oy = (height - s) / 2
                        ctx.fillStyle = Theme.text
                        // Bar
                        ctx.fillRect(ox, oy + s * 0.15, s * 0.15, s * 0.7)
                        // Triangle pointing left
                        ctx.beginPath()
                        ctx.moveTo(ox + s, oy + s * 0.15)
                        ctx.lineTo(ox + s * 0.2, oy + s * 0.5)
                        ctx.lineTo(ox + s, oy + s * 0.85)
                        ctx.closePath()
                        ctx.fill()
                    }
                    Component.onCompleted: requestPaint()
                }

                onClicked: toolbar.rewindRequested()
                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Rewind")
                ToolTip.delay: 500
            }

            // Play/Pause
            ToolButton {
                id: playBtn
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize

                background: Rectangle {
                    radius: 4
                    color: playBtn.pressed ? Theme.surfacePressed : (playBtn.hovered ? Theme.surfaceHover : Theme.surface)
                    border.color: toolbar.isPlaying ? Theme.accentBright : (playBtn.hovered ? Theme.accent : Theme.border)
                    border.width: toolbar.isPlaying ? 2 : 1
                }

                contentItem: Canvas {
                    property bool playing: toolbar.isPlaying
                    onPlayingChanged: requestPaint()
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        var s = Math.min(width, height) * 0.8
                        var ox = (width - s) / 2, oy = (height - s) / 2
                        var col = toolbar.isPlaying ? Theme.accentBright : Theme.text
                        ctx.fillStyle = col
                        if (toolbar.isPlaying) {
                            // Pause: two bars
                            var bw = s * 0.25
                            ctx.fillRect(ox + s * 0.15, oy + s * 0.15, bw, s * 0.7)
                            ctx.fillRect(ox + s * 0.6, oy + s * 0.15, bw, s * 0.7)
                        } else {
                            // Play: triangle
                            ctx.beginPath()
                            ctx.moveTo(ox + s * 0.2, oy + s * 0.1)
                            ctx.lineTo(ox + s * 0.9, oy + s * 0.5)
                            ctx.lineTo(ox + s * 0.2, oy + s * 0.9)
                            ctx.closePath()
                            ctx.fill()
                        }
                    }
                    Component.onCompleted: requestPaint()
                }

                onClicked: toolbar.playToggled()
                ToolTip.visible: enabled && hovered
                ToolTip.text: toolbar.isPlaying ? qsTr("Pause (Space)") : qsTr("Play (Space)")
                ToolTip.delay: 500
            }

            // Stop
            ToolButton {
                id: stopBtn
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize

                background: Rectangle {
                    radius: 4
                    color: stopBtn.pressed ? Theme.surfacePressed : (stopBtn.hovered ? Theme.surfaceHover : Theme.surface)
                    border.color: stopBtn.hovered ? Theme.accent : Theme.border
                }

                contentItem: Canvas {
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        var s = Math.min(width, height) * 0.8
                        var ox = (width - s) / 2, oy = (height - s) / 2
                        ctx.fillStyle = Theme.text
                        ctx.fillRect(ox + s * 0.15, oy + s * 0.15, s * 0.7, s * 0.7)
                    }
                    Component.onCompleted: requestPaint()
                }

                onClicked: toolbar.stopRequested()
                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Stop")
                ToolTip.delay: 500
            }

            // Loop toggle
            ToolButton {
                id: loopBtn
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize
                checkable: true
                checked: toolbar.isLooping

                background: Rectangle {
                    radius: 4
                    color: loopBtn.pressed ? Theme.surfacePressed : (loopBtn.hovered ? Theme.surfaceHover : Theme.surface)
                    border.color: loopBtn.checked ? Theme.accentBright : (loopBtn.hovered ? Theme.accent : Theme.border)
                    border.width: loopBtn.checked ? 2 : 1
                }

                contentItem: Canvas {
                    property bool looping: loopBtn.checked
                    onLoopingChanged: requestPaint()
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        var s = Math.min(width, height) * 0.8
                        var ox = (width - s) / 2, oy = (height - s) / 2
                        var cx = ox + s / 2, cy = oy + s / 2
                        var r = s * 0.35
                        var col = loopBtn.checked ? Theme.accentBright : Theme.text
                        ctx.strokeStyle = col
                        ctx.fillStyle = col
                        ctx.lineWidth = 2
                        ctx.lineCap = "round"
                        // Circular arc (270 degrees)
                        ctx.beginPath()
                        ctx.arc(cx, cy, r, -Math.PI * 0.8, Math.PI * 0.6)
                        ctx.stroke()
                        // Arrowhead at end of arc
                        var ax = cx + r * Math.cos(Math.PI * 0.6)
                        var ay = cy + r * Math.sin(Math.PI * 0.6)
                        ctx.beginPath()
                        ctx.moveTo(ax, ay)
                        ctx.lineTo(ax + 5, ay - 1)
                        ctx.lineTo(ax + 1, ay - 5)
                        ctx.closePath()
                        ctx.fill()
                    }
                    Component.onCompleted: requestPaint()
                }

                onClicked: toolbar.loopToggled()
                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Loop (L)")
                ToolTip.delay: 500
            }

            // Separator
            Rectangle {
                width: 1; height: Theme.toolbarButtonSize * 0.6
                color: Theme.border
                Layout.alignment: Qt.AlignVCenter
            }

            // Scrub slider
            Slider {
                id: scrubSlider
                Layout.preferredWidth: 120
                Layout.preferredHeight: Theme.toolbarButtonSize
                from: 0; to: 100
                value: toolbar.animationDurationMs > 0 ? (toolbar.playLocatorMs / toolbar.animationDurationMs * 100) : 0

                onMoved: {
                    var timeMs = Math.round(value / 100 * toolbar.animationDurationMs)
                    toolbar.scrubChanged(timeMs)
                }

                background: Rectangle {
                    x: scrubSlider.leftPadding
                    y: scrubSlider.topPadding + scrubSlider.availableHeight / 2 - height / 2
                    width: scrubSlider.availableWidth
                    height: 4
                    radius: 2
                    color: Theme.backgroundLight

                    Rectangle {
                        width: scrubSlider.visualPosition * parent.width
                        height: parent.height
                        radius: 2
                        color: Theme.accent
                    }
                }

                handle: Rectangle {
                    x: scrubSlider.leftPadding + scrubSlider.visualPosition * (scrubSlider.availableWidth - width)
                    y: scrubSlider.topPadding + scrubSlider.availableHeight / 2 - height / 2
                    width: 12; height: 12; radius: 6
                    color: scrubSlider.pressed ? Theme.accentBright : Theme.accent
                    border.color: Theme.border
                }
            }

            // Percentage label
            Label {
                text: Math.round(scrubSlider.value) + "%"
                color: Theme.text
                font.pixelSize: Theme.fontSizeSmall
                Layout.preferredWidth: 35
                horizontalAlignment: Text.AlignRight
            }
        }

        // Separator
        Rectangle {
            width: 1; height: Theme.toolbarButtonSize * 0.6
            color: Theme.border
            Layout.alignment: Qt.AlignVCenter
        }

        // Zone selector
        RowLayout {
            spacing: 4

            Label {
                text: qsTr("Zone:")
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSmall
            }

            StyledComboBox {
                id: zoneCombo
                Layout.preferredWidth: 120
                Layout.preferredHeight: Theme.toolbarButtonSize
                model: toolbar.zonesModel.length > 0 ? toolbar.zonesModel : [qsTr("(No zones)")]
                currentIndex: toolbar.currentZoneIndex
                enabled: toolbar.zonesModel.length > 0
                font.pixelSize: Theme.fontSizeSmall

                onActivated: function(index) {
                    toolbar.zoneSelected(index)
                }

                onHoveredChanged: toolbar.hoverChanged(hovered ? qsTr("Select output zone for preview") : "")

                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Select output zone for preview")
                ToolTip.delay: 500
            }
        }

        // Laser On/Off button
        ToolButton {
            id: laserBtn
            implicitWidth: Theme.toolbarButtonSize
            implicitHeight: Theme.toolbarButtonSize
            checkable: true
            checked: toolbar.laserEnabled

            background: Rectangle {
                radius: 4
                color: laserBtn.checked ? "#CC0000" : (laserBtn.pressed ? Theme.surfacePressed : (laserBtn.hovered ? Theme.surfaceHover : Theme.surface))
                border.color: laserBtn.checked ? "#FF0000" : (laserBtn.hovered ? Theme.accent : Theme.border)
                border.width: laserBtn.checked ? 2 : 1

                // Glow effect when laser is on
                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    color: "transparent"
                    border.color: "#FF0000"
                    border.width: 3
                    opacity: laserBtn.checked ? laserGlow.opacity : 0
                    visible: laserBtn.checked

                    SequentialAnimation on opacity {
                        id: laserGlow
                        running: laserBtn.checked
                        loops: Animation.Infinite
                        NumberAnimation { from: 0.3; to: 0.8; duration: 500 }
                        NumberAnimation { from: 0.8; to: 0.3; duration: 500 }
                    }
                }
            }

            contentItem: Text {
                text: "\u2600"  // Sun symbol for laser
                font.pixelSize: 16
                color: laserBtn.checked ? "#FFFFFF" : Theme.text
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onToggled: toolbar.laserToggled(checked)
            onHoveredChanged: toolbar.hoverChanged(hovered ? (checked ? qsTr("Laser ON - Click to turn off") : qsTr("Laser OFF - Click to turn on")) : "")

            ToolTip.visible: enabled && hovered
            ToolTip.text: checked ? qsTr("Laser ON - Click to turn off") : qsTr("Laser OFF - Click to turn on")
            ToolTip.delay: 500
        }
    }
}
