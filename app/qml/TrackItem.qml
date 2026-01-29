import QtQuick
import QtQuick.Controls
import GizmoTweakLib2
import GizmoTweak2

Rectangle {
    id: root

    property AutomationTrack track: null
    property int playLocatorMs: 0
    property int scrollLocatorMs: 0
    property int animationDurationMs: 10000  // 10 seconds default
    property int beatsPerMeasure: 4
    property int measureCount: 2
    property int selectedKeyFrameMs: -1
    property bool isCurrentTrack: false
    property bool snapEnabled: false  // Hold Shift to enable snap to beat grid

    signal keyFrameSelected(int timeMs)
    signal keyFrameCreated(int timeMs)
    signal keyFrameMoved(int oldTimeMs, int newTimeMs)
    signal keyFrameDeleted(int timeMs)
    signal scrollLocatorChanged(int timeMs)
    signal playLocatorChanged(int timeMs)
    signal locatorDragStarted()
    signal locatorDragEnded()
    signal keyFrameDoubleClicked(int timeMs)
    signal backgroundClicked()
    signal keyFrameCopied(var keyFrameData)
    signal keyFramePasteRequested(int timeMs)

    height: 28
    color: mouseArea.containsMouse ? Qt.rgba(0, 0, 0, 0) : Qt.rgba(0, 0, 0, 0.36)

    // Diamond handle half-width
    readonly property int handleHalfWidth: 6

    // Colors
    readonly property color brightSeparation: "#FFFFFF"
    readonly property color dimSeparation: "#9F9F9F"
    readonly property color brightPlayLocator: "#FF0000"
    readonly property color dimPlayLocator: "#7F0000"

    // Get color for node type (same as NodeToolbar)
    function colorForNodeType(nodeType) {
        switch (nodeType) {
            case "Gizmo": return Theme.nodeGizmo
            case "Transform": return Theme.nodeTransform
            case "SurfaceFactory": return Theme.nodeSurface
            case "Mirror":
            case "TimeShift": return Theme.nodeUtility
            default: return Theme.nodeTweak  // All tweaks
        }
    }
    readonly property color brightScrollLocator: "#0000FF"
    readonly property color dimScrollLocator: "#00007F"
    readonly property color measureLine: "#7FA0A0"
    readonly property color beatLine: "#7F7F7F"
    readonly property color selectedKeyFrame: "#FFFFFF"
    // Keyframe color matches node type color
    readonly property color unselectedKeyFrame: root.track ? colorForNodeType(root.track.nodeType) : "#00FFFF"

    function timeToX(timeMs) {
        return (timeMs / animationDurationMs) * width
    }

    function xToTime(x) {
        return Math.round((x / width) * animationDurationMs)
    }

    // Snap time to nearest beat grid position
    function snapToGrid(timeMs) {
        if (!snapEnabled) return timeMs
        var totalBeats = measureCount * beatsPerMeasure
        var beatDurationMs = animationDurationMs / totalBeats
        var nearestBeat = Math.round(timeMs / beatDurationMs)
        return Math.round(nearestBeat * beatDurationMs)
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()

            var bright = root.isCurrentTrack
            var separationColor = bright ? root.brightSeparation : root.dimSeparation
            var playLocatorColor = bright ? root.brightPlayLocator : root.dimPlayLocator
            var scrollLocatorColor = bright ? root.brightScrollLocator : root.dimScrollLocator

            // Top separation line
            ctx.strokeStyle = separationColor
            ctx.lineWidth = bright ? 1.5 : 1.0
            ctx.beginPath()
            ctx.moveTo(0, 0)
            ctx.lineTo(width, 0)
            ctx.stroke()

            // Draw beat/measure grid
            var sliceWidth = width / (root.measureCount * root.beatsPerMeasure)
            var lineX = 0
            for (var m = 0; m < root.measureCount; m++) {
                // Measure line
                if (m > 0) {
                    ctx.strokeStyle = root.measureLine
                    ctx.lineWidth = 1.0
                    ctx.beginPath()
                    ctx.moveTo(lineX, 0)
                    ctx.lineTo(lineX, height)
                    ctx.stroke()
                }
                lineX += sliceWidth

                // Beat lines
                ctx.strokeStyle = root.beatLine
                ctx.lineWidth = 1.0
                ctx.setLineDash([2, 2])
                for (var b = 1; b < root.beatsPerMeasure; b++) {
                    ctx.beginPath()
                    ctx.moveTo(lineX, 0)
                    ctx.lineTo(lineX, height)
                    ctx.stroke()
                    lineX += sliceWidth
                }
                ctx.setLineDash([])
            }

            // Scroll locator (blue line) - drawn below keyframes
            var scrollX = root.timeToX(root.scrollLocatorMs)
            ctx.strokeStyle = scrollLocatorColor
            ctx.lineWidth = bright ? 1.5 : 1.0
            ctx.beginPath()
            ctx.moveTo(scrollX, 0)
            ctx.lineTo(scrollX, height - 1)
            ctx.stroke()

            // Play locator (red line) - drawn below keyframes
            var playX = root.timeToX(root.playLocatorMs)
            ctx.strokeStyle = playLocatorColor
            ctx.lineWidth = bright ? 1.5 : 1.0
            ctx.beginPath()
            ctx.moveTo(playX, 0)
            ctx.lineTo(playX, height - 1)
            ctx.stroke()

            // Draw keyframes (on top of locators)
            if (root.track) {
                var times = root.track.keyFrameTimes()
                var trackHalfHeight = (height - 1) / 2

                for (var i = 0; i < times.length; i++) {
                    var timeMs = times[i]
                    var xCoord = root.timeToX(timeMs)
                    var isSelected = (timeMs === root.selectedKeyFrameMs)

                    // Vertical line below diamond
                    ctx.strokeStyle = isSelected ? root.selectedKeyFrame : root.unselectedKeyFrame
                    ctx.lineWidth = 1.0
                    ctx.beginPath()
                    ctx.moveTo(xCoord, trackHalfHeight + root.handleHalfWidth)
                    ctx.lineTo(xCoord, height)
                    ctx.stroke()

                    // Diamond shape
                    ctx.beginPath()
                    ctx.moveTo(xCoord, trackHalfHeight - root.handleHalfWidth)
                    ctx.lineTo(xCoord + root.handleHalfWidth, trackHalfHeight)
                    ctx.lineTo(xCoord, trackHalfHeight + root.handleHalfWidth)
                    ctx.lineTo(xCoord - root.handleHalfWidth, trackHalfHeight)
                    ctx.closePath()

                    if (isSelected) {
                        ctx.fillStyle = Qt.rgba(1, 1, 1, 0.5)
                        ctx.strokeStyle = root.selectedKeyFrame
                    } else {
                        // Use node type color with 50% alpha for fill
                        var kfColor = root.unselectedKeyFrame
                        ctx.fillStyle = Qt.rgba(kfColor.r, kfColor.g, kfColor.b, 0.5)
                        ctx.strokeStyle = root.unselectedKeyFrame
                    }
                    ctx.fill()
                    ctx.stroke()
                }
            }
        }
    }

    // Repaint when properties change
    onPlayLocatorMsChanged: canvas.requestPaint()
    onScrollLocatorMsChanged: canvas.requestPaint()
    onSelectedKeyFrameMsChanged: canvas.requestPaint()
    onIsCurrentTrackChanged: canvas.requestPaint()
    onAnimationDurationMsChanged: canvas.requestPaint()

    Connections {
        target: root.track
        function onKeyFrameCountChanged() { canvas.requestPaint() }
        function onKeyFrameModified() { canvas.requestPaint() }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        preventStealing: true  // Keep mouse capture during drag

        property bool draggingKeyFrame: false
        property int dragStartTimeMs: -1
        property bool isDragging: false

        onDoubleClicked: function(mouse) {
            root.snapEnabled = (mouse.modifiers & Qt.ShiftModifier)
            var timeMs = root.snapToGrid(root.xToTime(mouse.x))
            if (root.track) {
                if (root.track.hasKeyFrameAt(timeMs)) {
                    root.keyFrameSelected(timeMs)
                    root.keyFrameDoubleClicked(timeMs)
                } else {
                    // Check if near an existing keyframe
                    var times = root.track.keyFrameTimes()
                    var closestTime = -1
                    var minDist = 9999
                    for (var i = 0; i < times.length; i++) {
                        var kfX = root.timeToX(times[i])
                        var dist = Math.abs(kfX - mouse.x)
                        if (dist < root.handleHalfWidth && dist < minDist) {
                            minDist = dist
                            closestTime = times[i]
                        }
                    }
                    if (closestTime >= 0) {
                        root.keyFrameSelected(closestTime)
                        root.keyFrameDoubleClicked(closestTime)
                    } else {
                        // Create new keyframe and select it
                        root.track.createKeyFrame(timeMs)
                        root.keyFrameCreated(timeMs)
                        root.keyFrameSelected(timeMs)
                    }
                }
            }
        }

        onPressed: function(mouse) {
            var clickTimeMs = root.xToTime(mouse.x)
            var foundKeyFrame = false
            var locatorTimeMs = clickTimeMs  // Default to mouse position

            // Check if clicking on a keyframe
            if (root.track) {
                var times = root.track.keyFrameTimes()
                var minDist = 9999
                var closestTime = -1

                for (var i = 0; i < times.length; i++) {
                    var kfX = root.timeToX(times[i])
                    var dist = Math.abs(kfX - mouse.x)
                    if (dist < root.handleHalfWidth && dist < minDist) {
                        minDist = dist
                        closestTime = times[i]
                        foundKeyFrame = true
                    }
                }

                if (foundKeyFrame) {
                    draggingKeyFrame = true
                    dragStartTimeMs = closestTime
                    locatorTimeMs = closestTime  // Use exact keyframe time for locators
                    root.keyFrameSelected(closestTime)
                } else {
                    draggingKeyFrame = false
                    root.backgroundClicked()
                }
            }

            // Update locators
            root.scrollLocatorChanged(locatorTimeMs)
            root.playLocatorChanged(locatorTimeMs)

            // Signal drag start for global capture
            if (mouse.button === Qt.LeftButton) {
                isDragging = true
                root.locatorDragStarted()
            }

            if (mouse.button === Qt.RightButton && foundKeyFrame) {
                contextMenu.timeMs = closestTime
                contextMenu.popup()
            }
        }

        onPositionChanged: function(mouse) {
            if (pressed && (mouse.buttons & Qt.LeftButton)) {
                root.snapEnabled = (mouse.modifiers & Qt.ShiftModifier)
                var clampedX = Math.max(0, Math.min(mouse.x, width))
                var newTimeMs = root.xToTime(clampedX)

                root.scrollLocatorChanged(newTimeMs)
                root.playLocatorChanged(newTimeMs)

                if (draggingKeyFrame && root.track) {
                    var snappedTimeMs = root.snapToGrid(newTimeMs)
                    root.track.moveKeyFrame(dragStartTimeMs, snappedTimeMs)
                    root.keyFrameMoved(dragStartTimeMs, snappedTimeMs)
                    dragStartTimeMs = snappedTimeMs
                }
            }
        }

        onReleased: {
            draggingKeyFrame = false
            dragStartTimeMs = -1
            if (isDragging) {
                isDragging = false
                root.locatorDragEnded()
            }
        }

        onContainsMouseChanged: canvas.requestPaint()
    }

    // Public method to update locator from local X position (can be outside bounds)
    function updateLocatorFromLocalX(localX, shiftPressed) {
        snapEnabled = shiftPressed || false
        var clampedX = Math.max(0, Math.min(localX, width))
        var newTimeMs = xToTime(clampedX)

        scrollLocatorChanged(newTimeMs)
        playLocatorChanged(newTimeMs)

        // Also move keyframe if dragging one
        if (mouseArea.draggingKeyFrame && track) {
            var snappedTimeMs = snapToGrid(newTimeMs)
            track.moveKeyFrame(mouseArea.dragStartTimeMs, snappedTimeMs)
            keyFrameMoved(mouseArea.dragStartTimeMs, snappedTimeMs)
            mouseArea.dragStartTimeMs = snappedTimeMs
        }
    }

    // End drag from external call
    function endDrag() {
        mouseArea.draggingKeyFrame = false
        mouseArea.dragStartTimeMs = -1
        mouseArea.isDragging = false
    }

    // Context menu for keyframes
    Menu {
        id: contextMenu
        property int timeMs: -1

        background: Rectangle {
            implicitWidth: 180
            color: Theme.surface
            border.color: Theme.border
            radius: 2
        }

        MenuItem {
            text: qsTr("Copy Keyframe")
            onTriggered: {
                if (root.track && contextMenu.timeMs >= 0) {
                    var data = {
                        paramCount: root.track.paramCount,
                        values: []
                    }
                    for (var i = 0; i < root.track.paramCount; i++) {
                        data.values.push(root.track.timedValue(contextMenu.timeMs, i))
                    }
                    root.keyFrameCopied(data)
                }
            }

            background: Rectangle {
                implicitWidth: 180
                implicitHeight: 28
                color: parent.highlighted ? Theme.menuHighlight : "transparent"
            }

            contentItem: Text {
                text: parent.text
                color: parent.highlighted ? Theme.textOnHighlight : Theme.text
                font.pixelSize: Theme.fontSizeNormal
                verticalAlignment: Text.AlignVCenter
            }
        }

        MenuItem {
            text: qsTr("Paste Keyframe Here")
            onTriggered: {
                root.keyFramePasteRequested(contextMenu.timeMs)
            }

            background: Rectangle {
                implicitWidth: 180
                implicitHeight: 28
                color: parent.highlighted ? Theme.menuHighlight : "transparent"
            }

            contentItem: Text {
                text: parent.text
                color: parent.highlighted ? Theme.textOnHighlight : Theme.text
                font.pixelSize: Theme.fontSizeNormal
                verticalAlignment: Text.AlignVCenter
            }
        }

        MenuSeparator {
            contentItem: Rectangle {
                implicitWidth: 180
                implicitHeight: 1
                color: Theme.border
            }
        }

        MenuItem {
            text: qsTr("Delete Keyframe")
            onTriggered: {
                if (root.track && contextMenu.timeMs >= 0) {
                    var deletedTime = contextMenu.timeMs
                    root.track.deleteKeyFrame(deletedTime)
                    root.keyFrameDeleted(deletedTime)
                }
            }

            background: Rectangle {
                implicitWidth: 180
                implicitHeight: 28
                color: parent.highlighted ? Theme.menuHighlight : "transparent"
            }

            contentItem: Text {
                text: parent.text
                color: parent.highlighted ? Theme.textOnHighlight : Theme.text
                font.pixelSize: Theme.fontSizeNormal
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    // Watermark label - Format: "Instance / Track"
    // Left-aligned with padding
    Text {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        text: root.track ? root.track.nodeName + " / " + root.track.trackName : ""
        color: root.track ? colorForNodeType(root.track.nodeType) : Theme.text
        opacity: 0.6
        font.pixelSize: Theme.fontSizeLarge
        font.bold: true
    }

    // Right-aligned with padding (repeated)
    Text {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        text: root.track ? root.track.nodeName + " / " + root.track.trackName : ""
        color: root.track ? colorForNodeType(root.track.nodeType) : Theme.text
        opacity: 0.6
        font.pixelSize: Theme.fontSizeLarge
        font.bold: true
    }
}
