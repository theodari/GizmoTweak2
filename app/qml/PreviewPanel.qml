import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

Rectangle {
    id: root

    property NodeGraph graph: null
    property real currentTime: timeSlider.value / 100.0  // Normalized 0-1

    color: Theme.surface
    border.color: Theme.border
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Header
        Label {
            text: qsTr("Preview")
            color: Theme.text
            font.pixelSize: Theme.fontSizeNormal
            font.bold: true
            Layout.fillWidth: true
        }

        Rectangle {
            height: 1
            color: Theme.border
            Layout.fillWidth: true
        }

        // Preview using FramePreviewItem (C++ QPainter rendering)
        // FramePreviewItem observes the graph directly and evaluates it
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
                graph: root.graph        // Pass graph - FramePreviewItem evaluates it
                time: root.currentTime   // Pass time
                showGrid: showGridCheck.checked
                gridColor: Theme.previewGrid
                backgroundColor: Theme.previewBackground
                lineWidth: 2.0
            }
        }

        // Controls
        GridLayout {
            columns: 3
            columnSpacing: 8
            rowSpacing: 4
            Layout.fillWidth: true

            Label {
                text: qsTr("Time:")
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSmall
            }

            Slider {
                id: timeSlider
                from: 0
                to: 100
                value: 0
                stepSize: 1
                Layout.fillWidth: true

                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Animation time position")
                ToolTip.delay: 500
            }

            Label {
                text: Math.round(timeSlider.value) + "%"
                color: Theme.text
                font.pixelSize: Theme.fontSizeSmall
                Layout.preferredWidth: 35
            }
        }

        // Options row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            StyledCheckBox {
                id: showGridCheck
                text: qsTr("Grid")
                checked: true
                font.pixelSize: Theme.fontSizeSmall

                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Show/hide reference grid")
                ToolTip.delay: 500
            }

            StyledCheckBox {
                id: showSourceCheck
                text: qsTr("Source")
                checked: false
                font.pixelSize: Theme.fontSizeSmall

                // TODO: Implement source frame overlay in FramePreviewItem

                ToolTip.visible: enabled && hovered
                ToolTip.text: qsTr("Show/hide original source frame")
                ToolTip.delay: 500
            }

            Button {
                id: playButton
                implicitWidth: Theme.toolbarButtonSize
                implicitHeight: Theme.toolbarButtonSize

                property bool playing: false

                onClicked: {
                    playing = !playing
                    if (!playing) {
                        // Reset to beginning when stopped
                        timeSlider.value = 0
                        animationTimer.elapsed = 0
                    }
                }

                background: Rectangle {
                    color: playButton.down ? Theme.surfacePressed : (playButton.hovered ? Theme.surfaceHover : Theme.surface)
                    border.color: playButton.playing ? Theme.accent : Theme.border
                    border.width: playButton.playing ? 2 : 1
                    radius: 4
                }

                contentItem: Text {
                    text: playButton.playing ? "\u25A0" : "\u25B6"  // Stop / Play symbols
                    color: playButton.playing ? Theme.accent : Theme.text
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                ToolTip.visible: enabled && hovered
                ToolTip.text: playing ? qsTr("Stop animation") : qsTr("Play animation")
                ToolTip.delay: 500
            }

            Item { Layout.fillWidth: true }
        }

        // Spacer to push everything to the top
        Item { Layout.fillHeight: true }
    }

    // Animation timer
    Timer {
        id: animationTimer
        interval: 16  // ~60 FPS
        running: playButton.playing
        repeat: true

        property real elapsed: 0

        onTriggered: {
            elapsed += interval / 1000.0 * 100  // Scale to percentage
            timeSlider.value = elapsed % 100
        }
    }
}
