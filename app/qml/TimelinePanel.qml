import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

Rectangle {
    id: root

    property NodeGraph graph: null
    property int playLocatorMs: 0
    property int scrollLocatorMs: 0
    property int animationDurationMs: 10000  // 10 seconds
    property int beatsPerMeasure: 4
    property int measureCount: 2
    property AutomationTrack currentTrack: null
    property int currentKeyFrameMs: -1

    signal playLocatorChanged(int timeMs)

    color: Theme.background

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Left: Timeline tracks
        Rectangle {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 300
            color: Theme.background

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Timeline header with transport controls
                Rectangle {
                    Layout.fillWidth: true
                    height: 32
                    color: Theme.backgroundLight

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 8

                        // Transport controls
                        Button {
                            id: rewindButton
                            width: 24
                            height: 24
                            text: "⏮"
                            font.pixelSize: 12

                            background: Rectangle {
                                color: parent.down ? Theme.surfacePressed : (parent.hovered ? Theme.surfaceHover : Theme.surface)
                                radius: 4
                                border.color: Theme.border
                            }

                            onClicked: {
                                root.playLocatorMs = 0
                                root.scrollLocatorMs = 0
                                root.playLocatorChanged(0)
                            }
                        }

                        Button {
                            id: playButton
                            width: 24
                            height: 24
                            text: playTimer.running ? "⏸" : "▶"
                            font.pixelSize: 12

                            background: Rectangle {
                                color: parent.down ? Theme.surfacePressed : (parent.hovered ? Theme.surfaceHover : Theme.surface)
                                radius: 4
                                border.color: Theme.border
                            }

                            onClicked: {
                                if (playTimer.running) {
                                    playTimer.stop()
                                } else {
                                    playTimer.start()
                                }
                            }
                        }

                        Button {
                            id: stopButton
                            width: 24
                            height: 24
                            text: "⏹"
                            font.pixelSize: 12

                            background: Rectangle {
                                color: parent.down ? Theme.surfacePressed : (parent.hovered ? Theme.surfaceHover : Theme.surface)
                                radius: 4
                                border.color: Theme.border
                            }

                            onClicked: {
                                playTimer.stop()
                                root.playLocatorMs = 0
                                root.scrollLocatorMs = 0
                                root.playLocatorChanged(0)
                            }
                        }

                        // Separator
                        Rectangle {
                            width: 1
                            height: 20
                            color: Theme.border
                        }

                        // Time display
                        Label {
                            text: formatTime(root.scrollLocatorMs)
                            color: Theme.text
                            font.pixelSize: Theme.fontSizeNormal
                            font.family: "Courier New"

                            function formatTime(ms) {
                                var seconds = Math.floor(ms / 1000)
                                var millis = ms % 1000
                                var mins = Math.floor(seconds / 60)
                                var secs = seconds % 60
                                return mins.toString().padStart(2, '0') + ":" +
                                       secs.toString().padStart(2, '0') + "." +
                                       Math.floor(millis / 100).toString()
                            }
                        }

                        Item { Layout.fillWidth: true }

                        // Duration control
                        Label {
                            text: qsTr("Duration:")
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        SpinBox {
                            id: durationSpinBox
                            from: 1
                            to: 300
                            value: root.animationDurationMs / 1000
                            stepSize: 1

                            onValueChanged: {
                                root.animationDurationMs = value * 1000
                            }

                            background: Rectangle {
                                implicitWidth: 70
                                implicitHeight: 24
                                color: Theme.surface
                                border.color: Theme.border
                                radius: 2
                            }

                            contentItem: TextInput {
                                text: durationSpinBox.textFromValue(durationSpinBox.value, durationSpinBox.locale)
                                font.pixelSize: Theme.fontSizeSmall
                                color: Theme.text
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !durationSpinBox.editable
                                validator: durationSpinBox.validator
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                        }

                        Label {
                            text: "s"
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        // Beats per measure
                        Label {
                            text: qsTr("Beats:")
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        SpinBox {
                            from: 1
                            to: 16
                            value: root.beatsPerMeasure
                            onValueChanged: root.beatsPerMeasure = value

                            background: Rectangle {
                                implicitWidth: 50
                                implicitHeight: 24
                                color: Theme.surface
                                border.color: Theme.border
                                radius: 2
                            }

                            contentItem: TextInput {
                                text: parent.textFromValue(parent.value, parent.locale)
                                font.pixelSize: Theme.fontSizeSmall
                                color: Theme.text
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !parent.editable
                                validator: parent.validator
                            }
                        }

                        // Measures
                        Label {
                            text: qsTr("Measures:")
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        SpinBox {
                            from: 1
                            to: 32
                            value: root.measureCount
                            onValueChanged: root.measureCount = value

                            background: Rectangle {
                                implicitWidth: 50
                                implicitHeight: 24
                                color: Theme.surface
                                border.color: Theme.border
                                radius: 2
                            }

                            contentItem: TextInput {
                                text: parent.textFromValue(parent.value, parent.locale)
                                font.pixelSize: Theme.fontSizeSmall
                                color: Theme.text
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !parent.editable
                                validator: parent.validator
                            }
                        }
                    }
                }

                // Separator
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Theme.border
                }

                // Track list
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    ListView {
                        id: trackList
                        model: trackModel
                        spacing: 0

                        delegate: TrackItem {
                            width: trackList.width
                            track: modelData
                            playLocatorMs: root.playLocatorMs
                            scrollLocatorMs: root.scrollLocatorMs
                            animationDurationMs: root.animationDurationMs
                            beatsPerMeasure: root.beatsPerMeasure
                            measureCount: root.measureCount
                            selectedKeyFrameMs: (root.currentTrack === modelData) ? root.currentKeyFrameMs : -1
                            isCurrentTrack: root.currentTrack === modelData

                            onKeyFrameSelected: function(timeMs) {
                                root.currentTrack = modelData
                                root.currentKeyFrameMs = timeMs
                                root.scrollLocatorMs = timeMs
                            }

                            onKeyFrameCreated: function(timeMs) {
                                root.currentTrack = modelData
                                root.currentKeyFrameMs = timeMs
                                root.scrollLocatorMs = timeMs
                            }

                            onScrollLocatorChanged: function(timeMs) {
                                root.scrollLocatorMs = timeMs
                            }

                            onPlayLocatorChanged: function(timeMs) {
                                root.playLocatorMs = timeMs
                                root.playLocatorChanged(timeMs)
                            }
                        }
                    }
                }

                // Empty state
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"
                    visible: trackModel.length === 0

                    Label {
                        anchors.centerIn: parent
                        text: qsTr("No automation tracks.\nSelect a node and click the gear icon on a parameter.")
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontSizeNormal
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        // Right: Keyframe editor
        Rectangle {
            SplitView.preferredWidth: 200
            SplitView.minimumWidth: 150
            SplitView.maximumWidth: 350
            color: Theme.surface

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                // Header
                Label {
                    text: qsTr("Keyframe")
                    color: Theme.text
                    font.pixelSize: Theme.fontSizeLarge
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Theme.border
                }

                // No keyframe selected
                Label {
                    visible: !root.currentTrack || root.currentKeyFrameMs < 0
                    text: qsTr("Select a keyframe to edit")
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontSizeSmall
                    font.italic: true
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                // Keyframe parameters
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: root.currentTrack && root.currentKeyFrameMs >= 0
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: 8

                        // Time display
                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: qsTr("Time:")
                                color: Theme.textMuted
                                font.pixelSize: Theme.fontSizeSmall
                            }

                            Label {
                                text: root.currentKeyFrameMs >= 0 ? (root.currentKeyFrameMs / 1000).toFixed(2) + " s" : "-"
                                color: Theme.text
                                font.pixelSize: Theme.fontSizeSmall
                            }
                        }

                        // Easing curve selection
                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: qsTr("Easing:")
                                color: Theme.textMuted
                                font.pixelSize: Theme.fontSizeSmall
                            }

                            ComboBox {
                                id: easingCombo
                                Layout.fillWidth: true
                                model: [
                                    "Linear",
                                    "InQuad", "OutQuad", "InOutQuad",
                                    "InCubic", "OutCubic", "InOutCubic",
                                    "InSine", "OutSine", "InOutSine",
                                    "InExpo", "OutExpo", "InOutExpo",
                                    "InBounce", "OutBounce", "InOutBounce"
                                ]

                                background: Rectangle {
                                    implicitHeight: 24
                                    color: Theme.surface
                                    border.color: Theme.border
                                    radius: 2
                                }

                                contentItem: Text {
                                    text: easingCombo.displayText
                                    color: Theme.text
                                    font.pixelSize: Theme.fontSizeSmall
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 4
                                }
                            }
                        }

                        // Parameter values (dynamically generated based on track)
                        Repeater {
                            model: root.currentTrack ? root.currentTrack.paramCount : 0

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                property int paramIndex: index
                                property string paramName: root.currentTrack ? root.currentTrack.parameterName(index) : ""
                                property double paramValue: root.currentTrack && root.currentKeyFrameMs >= 0
                                    ? root.currentTrack.timedValue(root.currentKeyFrameMs, index)
                                    : 0
                                property double minVal: root.currentTrack ? root.currentTrack.minValue(index) : 0
                                property double maxVal: root.currentTrack ? root.currentTrack.maxValue(index) : 1
                                property double displayRatio: root.currentTrack ? root.currentTrack.displayRatio(index) : 1
                                property string suffix: root.currentTrack ? root.currentTrack.suffix(index) : ""

                                Label {
                                    text: paramName
                                    color: Theme.textMuted
                                    font.pixelSize: Theme.fontSizeSmall
                                }

                                RowLayout {
                                    Layout.fillWidth: true

                                    Slider {
                                        id: paramSlider
                                        Layout.fillWidth: true
                                        from: minVal
                                        to: maxVal
                                        value: paramValue
                                        stepSize: (maxVal - minVal) / 100

                                        onMoved: {
                                            if (root.currentTrack && root.currentKeyFrameMs >= 0) {
                                                root.currentTrack.updateKeyFrameValue(
                                                    root.currentKeyFrameMs, paramIndex, value)
                                            }
                                        }

                                        background: Rectangle {
                                            x: paramSlider.leftPadding
                                            y: paramSlider.topPadding + paramSlider.availableHeight / 2 - height / 2
                                            width: paramSlider.availableWidth
                                            height: 4
                                            radius: 2
                                            color: Theme.backgroundLight

                                            Rectangle {
                                                width: paramSlider.visualPosition * parent.width
                                                height: parent.height
                                                color: Theme.accent
                                                radius: 2
                                            }
                                        }

                                        handle: Rectangle {
                                            x: paramSlider.leftPadding + paramSlider.visualPosition * (paramSlider.availableWidth - width)
                                            y: paramSlider.topPadding + paramSlider.availableHeight / 2 - height / 2
                                            width: 12
                                            height: 12
                                            radius: 6
                                            color: paramSlider.pressed ? Theme.accentBright : Theme.accent
                                            border.color: Theme.border
                                        }
                                    }

                                    Label {
                                        text: Math.round(paramValue * displayRatio) + suffix
                                        color: Theme.text
                                        font.pixelSize: Theme.fontSizeSmall
                                        Layout.preferredWidth: 50
                                        horizontalAlignment: Text.AlignRight
                                    }
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }
    }

    // Play timer (40ms = 25fps like original)
    Timer {
        id: playTimer
        interval: 40
        repeat: true

        onTriggered: {
            root.playLocatorMs += interval
            if (root.playLocatorMs >= root.animationDurationMs) {
                root.playLocatorMs = 0  // Loop
            }
            root.playLocatorChanged(root.playLocatorMs)
        }
    }

    // Track model - this will be populated from the graph's nodes
    property var trackModel: []

    // Function to collect all automation tracks from the graph
    function updateTrackModel() {
        var tracks = []
        // TODO: Iterate through nodes and collect their automation tracks
        trackModel = tracks
    }
}
