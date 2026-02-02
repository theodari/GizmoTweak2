import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

Rectangle {
    id: root

    property NodeGraph graph: null
    property real currentTime: playLocatorMs / 1000.0  // Time in seconds for evaluator

    // Playback state (shared with timeline)
    property bool isPlaying: false
    property bool isLooping: true
    property int playLocatorMs: 0
    property int animationDurationMs: 10000

    // Grid toggle
    property bool showGrid: true

    // Laser engine output
    property int currentZoneIndex: 0

    // Keyframe editing context (set from Main.qml)
    property AutomationTrack currentTrack: null
    property int currentKeyFrameMs: -1
    property int scrollLocatorMs: 0

    // Signals for timeline synchronization
    signal playStateChanged(bool playing)
    signal playLocatorChanged(int timeMs)
    signal rewindRequested()
    signal stopRequested()
    signal keyFrameDataModified()

    // Public function for external control (keyboard shortcuts from Main.qml)
    function togglePlayback() {
        root.isPlaying = !root.isPlaying
        root.playStateChanged(root.isPlaying)
    }

    function rewind() {
        root.isPlaying = false
        root.playLocatorMs = 0
        root.rewindRequested()
        root.playLocatorChanged(0)
    }

    function stop() {
        root.isPlaying = false
        root.playLocatorMs = 0
        root.stopRequested()
        root.playLocatorChanged(0)
    }

    function toggleLoop() {
        root.isLooping = !root.isLooping
    }

    color: Theme.surface
    border.color: Theme.border
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        // Preview rendering area with hover Grid button
        Rectangle {
            id: previewBackground
            Layout.fillWidth: true
            Layout.preferredHeight: width  // Keep it square
            Layout.maximumHeight: width
            color: Theme.previewBackground
            border.color: Theme.border
            border.width: 1

            FramePreviewItem {
                id: framePreview
                anchors.fill: parent
                graph: root.graph
                time: root.currentTime
                showGrid: root.showGrid
                gridColor: Theme.previewGrid
                backgroundColor: Theme.previewBackground
                lineWidth: 2.0
                laserEngine: laserEngine
                zoneIndex: root.currentZoneIndex
            }

            // Hover area for Grid toggle button
            MouseArea {
                id: previewHover
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
            }

            // Grid toggle button (visible on hover)
            ToolButton {
                id: gridToggle
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 4
                width: 24; height: 24
                opacity: (previewHover.containsMouse || gridToggle.hovered) ? 0.8 : 0
                visible: opacity > 0

                Behavior on opacity { NumberAnimation { duration: 150 } }

                background: Rectangle {
                    radius: 3
                    color: root.showGrid ? Theme.accent : Theme.surface
                    border.color: Theme.border
                    opacity: 0.9
                }

                contentItem: Canvas {
                    id: gridIconCanvas
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        var p = 3, w = width - p * 2, h = height - p * 2
                        var c = root.showGrid ? "#FFFFFF" : Theme.text
                        ctx.strokeStyle = c
                        ctx.lineWidth = 1

                        // 4x4 grid lines (matching actual preview grid)
                        for (var i = 1; i < 4; i++) {
                            ctx.beginPath()
                            ctx.moveTo(p + i * w / 4, p)
                            ctx.lineTo(p + i * w / 4, p + h)
                            ctx.stroke()
                            ctx.beginPath()
                            ctx.moveTo(p, p + i * h / 4)
                            ctx.lineTo(p + w, p + i * h / 4)
                            ctx.stroke()
                        }

                        // Center cross (brighter)
                        ctx.strokeStyle = Qt.lighter(c, 1.5)
                        ctx.lineWidth = 1.5
                        var cx = p + w / 2, cy = p + h / 2, cr = 2
                        ctx.beginPath()
                        ctx.moveTo(cx - cr, cy)
                        ctx.lineTo(cx + cr, cy)
                        ctx.stroke()
                        ctx.beginPath()
                        ctx.moveTo(cx, cy - cr)
                        ctx.lineTo(cx, cy + cr)
                        ctx.stroke()
                    }
                    Component.onCompleted: requestPaint()

                    Connections {
                        target: root
                        function onShowGridChanged() { gridIconCanvas.requestPaint() }
                    }
                }

                onClicked: root.showGrid = !root.showGrid
                ToolTip.visible: enabled && hovered
                ToolTip.text: root.showGrid ? qsTr("Hide grid") : qsTr("Show grid")
                ToolTip.delay: 500
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true; height: 1; color: Theme.border
        }

        // Keyframe editor section (below preview)
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            // Header
            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: root.currentKeyFrameMs >= 0 ? qsTr("Keyframe") : qsTr("Current values")
                    color: Theme.text
                    font.pixelSize: Theme.fontSizeNormal
                    font.bold: root.currentKeyFrameMs >= 0
                    font.italic: root.currentKeyFrameMs < 0 && root.currentTrack !== null
                }

                Item { Layout.fillWidth: true }

                // Track color indicator
                Rectangle {
                    visible: root.currentTrack !== null
                    width: 10; height: 10; radius: 5
                    color: root.currentTrack ? root.currentTrack.color : "transparent"
                    border.color: Theme.border
                }

                Label {
                    visible: root.currentTrack !== null
                    text: root.currentTrack ? root.currentTrack.trackName : ""
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeSmall
                }
            }

            // Keyframe selected: show time + curve + params
            ColumnLayout {
                Layout.fillWidth: true
                visible: root.currentTrack !== null && root.currentKeyFrameMs >= 0
                spacing: 4

                // Time display
                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Time:")
                        color: Theme.propLabel
                        font.pixelSize: Theme.propFontSize
                        Layout.preferredWidth: 55
                    }

                    Label {
                        text: root.currentKeyFrameMs >= 0 ? (root.currentKeyFrameMs / 1000).toFixed(3) + " s" : "-"
                        color: Theme.text
                        font.pixelSize: Theme.propFontSize
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }
                }

                // Curve selection grid
                ColumnLayout {
                    id: curveSection
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        text: qsTr("Curve")
                        color: Theme.propLabel
                        font.pixelSize: Theme.propFontSize
                    }

                    readonly property var curveTypes: [
                        0,          // Linear
                        1, 2, 3,    // Quad
                        5, 6, 7,    // Cubic
                        9, 10, 11,  // Sine
                        13, 14, 15, // Expo
                        25, 26, 27  // Bounce
                    ]

                    property int selectedCurveType: 0

                    function refreshCurveType() {
                        if (root.currentTrack && root.currentKeyFrameMs >= 0)
                            selectedCurveType = root.currentTrack.keyFrameCurveType(root.currentKeyFrameMs)
                        else
                            selectedCurveType = 0
                    }

                    Connections {
                        target: root
                        function onCurrentTrackChanged() { curveSection.refreshCurveType() }
                        function onCurrentKeyFrameMsChanged() { curveSection.refreshCurveType() }
                    }

                    Grid {
                        columns: 4
                        spacing: 2
                        Layout.fillWidth: true

                        Repeater {
                            model: curveSection.curveTypes

                            CurveIconButton {
                                curveType: modelData
                                selected: curveSection.selectedCurveType === modelData
                                width: 28; height: 28
                                onClicked: {
                                    if (root.currentTrack && root.currentKeyFrameMs >= 0) {
                                        root.currentTrack.setKeyFrameCurveType(root.currentKeyFrameMs, modelData)
                                        curveSection.selectedCurveType = modelData
                                        root.keyFrameDataModified()
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true; height: 1; color: Theme.border
                    Layout.topMargin: 2; Layout.bottomMargin: 2
                }

                // Editable parameter rows
                Repeater {
                    model: root.currentTrack ? root.currentTrack.paramCount : 0

                    KeyframeParameterRow {
                        Layout.fillWidth: true
                        track: root.currentTrack
                        keyFrameMs: root.currentKeyFrameMs
                        paramIndex: index

                        onValueModified: root.keyFrameDataModified()
                    }
                }
            }

            // No keyframe selected but track active: show read-only values
            ColumnLayout {
                Layout.fillWidth: true
                visible: root.currentTrack !== null && root.currentKeyFrameMs < 0
                spacing: 4

                Repeater {
                    model: root.currentTrack ? root.currentTrack.paramCount : 0

                    ReadOnlyParameterRow {
                        Layout.fillWidth: true
                        track: root.currentTrack
                        paramIndex: index
                        timeMs: root.scrollLocatorMs
                    }
                }
            }

            // No track selected
            Label {
                visible: root.currentTrack === null
                text: qsTr("Select a track")
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSmall
                font.italic: true
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 8
            }

            // Spacer
            Item { Layout.fillHeight: true }
        }
    }

    // Animation timer
    Timer {
        id: animationTimer
        interval: 40  // 25 FPS
        running: root.isPlaying
        repeat: true

        onTriggered: {
            root.playLocatorMs += interval
            if (root.playLocatorMs >= root.animationDurationMs) {
                if (root.isLooping) {
                    root.playLocatorMs = 0
                } else {
                    root.isPlaying = false
                    root.playLocatorMs = root.animationDurationMs
                }
            }
            root.playLocatorChanged(root.playLocatorMs)
        }
    }
}
