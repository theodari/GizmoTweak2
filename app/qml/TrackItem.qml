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

    signal keyFrameSelected(int timeMs)
    signal keyFrameCreated(int timeMs)
    signal keyFrameMoved(int oldTimeMs, int newTimeMs)
    signal scrollLocatorChanged(int timeMs)
    signal playLocatorChanged(int timeMs)

    height: 28
    color: mouseArea.containsMouse ? Qt.rgba(0, 0, 0, 0) : Qt.rgba(0, 0, 0, 0.36)

    // Diamond handle half-width
    readonly property int handleHalfWidth: 6

    // Colors
    readonly property color brightSeparation: "#FFFFFF"
    readonly property color dimSeparation: "#9F9F9F"
    readonly property color brightPlayLocator: "#FF0000"
    readonly property color dimPlayLocator: "#7F0000"
    readonly property color brightScrollLocator: "#0000FF"
    readonly property color dimScrollLocator: "#00007F"
    readonly property color measureLine: "#7FA0A0"
    readonly property color beatLine: "#7F7F7F"
    readonly property color selectedKeyFrame: "#FFFFFF"
    readonly property color unselectedKeyFrame: "#00FFFF"

    function timeToX(timeMs) {
        return (timeMs / animationDurationMs) * width
    }

    function xToTime(x) {
        return Math.round((x / width) * animationDurationMs)
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

            // Draw keyframes
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
                        ctx.fillStyle = Qt.rgba(0, 1, 1, 0.5)
                        ctx.strokeStyle = root.unselectedKeyFrame
                    }
                    ctx.fill()
                    ctx.stroke()
                }
            }

            // Scroll locator (blue line)
            var scrollX = root.timeToX(root.scrollLocatorMs)
            ctx.strokeStyle = scrollLocatorColor
            ctx.lineWidth = bright ? 1.5 : 1.0
            ctx.beginPath()
            ctx.moveTo(scrollX, 0)
            ctx.lineTo(scrollX, height - 1)
            ctx.stroke()

            // Play locator (red line)
            var playX = root.timeToX(root.playLocatorMs)
            ctx.strokeStyle = playLocatorColor
            ctx.lineWidth = bright ? 1.5 : 1.0
            ctx.beginPath()
            ctx.moveTo(playX, 0)
            ctx.lineTo(playX, height - 1)
            ctx.stroke()
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

        property bool draggingKeyFrame: false
        property int dragStartTimeMs: -1

        onDoubleClicked: function(mouse) {
            var timeMs = root.xToTime(mouse.x)
            if (root.track && !root.track.hasKeyFrameAt(timeMs)) {
                root.track.createKeyFrame(timeMs)
                root.selectedKeyFrameMs = timeMs
                root.keyFrameCreated(timeMs)
            }
        }

        onPressed: function(mouse) {
            var clickTimeMs = root.xToTime(mouse.x)
            var foundKeyFrame = false

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
                    root.selectedKeyFrameMs = closestTime
                    draggingKeyFrame = true
                    dragStartTimeMs = closestTime
                    root.keyFrameSelected(closestTime)
                } else {
                    root.selectedKeyFrameMs = -1
                    draggingKeyFrame = false
                }
            }

            // Update locators
            root.scrollLocatorChanged(clickTimeMs)
            root.playLocatorChanged(clickTimeMs)

            if (mouse.button === Qt.RightButton && foundKeyFrame) {
                contextMenu.timeMs = root.selectedKeyFrameMs
                contextMenu.popup()
            }
        }

        onPositionChanged: function(mouse) {
            if (pressed && (mouse.buttons & Qt.LeftButton)) {
                var clampedX = Math.max(0, Math.min(mouse.x, width))
                var newTimeMs = root.xToTime(clampedX)

                root.scrollLocatorChanged(newTimeMs)
                root.playLocatorChanged(newTimeMs)

                if (draggingKeyFrame && root.track) {
                    root.track.moveKeyFrame(root.selectedKeyFrameMs, newTimeMs)
                    root.selectedKeyFrameMs = newTimeMs
                    root.keyFrameMoved(dragStartTimeMs, newTimeMs)
                    dragStartTimeMs = newTimeMs
                }
            }
        }

        onReleased: {
            draggingKeyFrame = false
            dragStartTimeMs = -1
        }

        onContainsMouseChanged: canvas.requestPaint()
    }

    // Context menu for keyframes
    Menu {
        id: contextMenu
        property int timeMs: -1

        background: Rectangle {
            implicitWidth: 150
            color: Theme.surface
            border.color: Theme.border
            radius: 2
        }

        MenuItem {
            text: qsTr("Delete Keyframe")
            onTriggered: {
                if (root.track && contextMenu.timeMs >= 0) {
                    root.track.deleteKeyFrame(contextMenu.timeMs)
                    root.selectedKeyFrameMs = -1
                }
            }

            background: Rectangle {
                implicitWidth: 150
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

    // Track label on the left
    Rectangle {
        width: 100
        height: parent.height
        color: root.track ? root.track.color : Theme.surface
        opacity: 0.8

        Text {
            anchors.centerIn: parent
            text: root.track ? root.track.trackName : ""
            color: Theme.text
            font.pixelSize: Theme.fontSizeSmall
            font.bold: root.isCurrentTrack
            elide: Text.ElideRight
            width: parent.width - 8
            horizontalAlignment: Text.AlignHCenter
        }

        // Separator
        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: Theme.border
        }
    }
}
