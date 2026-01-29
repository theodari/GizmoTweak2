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

    // Track which TrackItem is being dragged
    property var draggingTrackItem: null

    // Keyframe clipboard for copy/paste between tracks
    property var keyframeClipboard: null

    signal playLocatorChanged(int timeMs)
    signal keyFrameDataModified()
    signal keyFrameDoubleClicked()

    color: Theme.background

    // Global drag overlay - covers entire window during drag
    // This ensures mouse capture even when dragging far outside the track
    // Always present, but only enabled/visible during drag for immediate response
    Item {
        id: globalDragOverlay
        parent: Overlay.overlay
        anchors.fill: parent
        visible: true  // Always visible to avoid creation delay
        z: 99999  // Ensure it's on top of everything

        MouseArea {
            id: globalDragArea
            anchors.fill: parent
            enabled: root.draggingTrackItem !== null  // Only active during drag
            cursorShape: enabled ? Qt.SizeHorCursor : Qt.ArrowCursor
            hoverEnabled: true
            preventStealing: true  // Prevent other items from stealing the drag
            propagateComposedEvents: !enabled  // Pass through when not dragging

            // Accept press events when dragging to maintain capture
            acceptedButtons: enabled ? Qt.LeftButton : Qt.NoButton

            onPositionChanged: function(mouse) {
                if (root.draggingTrackItem) {
                    // Map position from overlay to the track item
                    var localPos = root.draggingTrackItem.mapFromItem(globalDragOverlay, mouse.x, mouse.y)
                    var shiftPressed = (mouse.modifiers & Qt.ShiftModifier)
                    root.draggingTrackItem.updateLocatorFromLocalX(localPos.x, shiftPressed)
                }
            }

            onReleased: {
                if (root.draggingTrackItem) {
                    root.draggingTrackItem.endDrag()
                    root.draggingTrackItem = null
                }
            }

            // Also handle if mouse exits the window entirely
            onCanceled: {
                if (root.draggingTrackItem) {
                    root.draggingTrackItem.endDrag()
                    root.draggingTrackItem = null
                }
            }
        }
    }

    // Timeline tracks (full width, no SplitView)
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

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
                    id: trackDelegate
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
                        root.keyFrameDataModified()
                    }

                    onKeyFrameDeleted: function(timeMs) {
                        root.currentKeyFrameMs = -1
                        root.keyFrameDataModified()
                    }

                    onKeyFrameMoved: function(oldTimeMs, newTimeMs) {
                        root.currentKeyFrameMs = newTimeMs
                        root.scrollLocatorMs = newTimeMs
                        root.keyFrameDataModified()
                    }

                    onScrollLocatorChanged: function(timeMs) {
                        root.scrollLocatorMs = timeMs
                    }

                    onPlayLocatorChanged: function(timeMs) {
                        root.playLocatorChanged(timeMs)
                    }

                    onLocatorDragStarted: {
                        root.draggingTrackItem = this
                    }

                    onLocatorDragEnded: {
                        root.draggingTrackItem = null
                    }

                    onBackgroundClicked: {
                        root.currentTrack = modelData
                        root.currentKeyFrameMs = -1
                    }

                    onKeyFrameDoubleClicked: function(timeMs) {
                        root.currentTrack = modelData
                        root.currentKeyFrameMs = timeMs
                        root.keyFrameDoubleClicked()
                    }

                    onKeyFrameCopied: function(data) {
                        root.keyframeClipboard = data
                    }

                    onKeyFramePasteRequested: function(timeMs) {
                        if (root.keyframeClipboard && modelData) {
                            if (root.keyframeClipboard.paramCount === modelData.paramCount) {
                                modelData.createKeyFrame(timeMs)
                                for (var i = 0; i < root.keyframeClipboard.paramCount; i++) {
                                    modelData.updateKeyFrameValue(timeMs, i, root.keyframeClipboard.values[i])
                                }
                                root.currentTrack = modelData
                                root.currentKeyFrameMs = timeMs
                                root.keyFrameDataModified()
                            }
                        }
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

    // Track model - this will be populated from the graph's nodes
    property var trackModel: []

    // Function to collect all automation tracks from the graph
    function updateTrackModel() {
        var tracks = []
        if (graph) {
            for (var i = 0; i < graph.nodeCount; i++) {
                var node = graph.nodeAt(i)
                if (node) {
                    var nodeTracks = node.automationTracks
                    for (var j = 0; j < nodeTracks.length; j++) {
                        // Only add tracks that have automation enabled or keyframes
                        if (nodeTracks[j].automated || nodeTracks[j].keyFrameCount > 0) {
                            tracks.push(nodeTracks[j])
                        }
                    }
                }
            }
        }
        // Only rebuild if the track list actually changed (avoids destroying delegates
        // during keyframe operations which would break in-progress signal handlers)
        if (tracks.length === trackModel.length) {
            var same = true
            for (var k = 0; k < tracks.length; k++) {
                if (tracks[k] !== trackModel[k]) {
                    same = false
                    break
                }
            }
            if (same) return
        }
        // Force model refresh by clearing first
        trackModel = []
        trackModel = tracks
    }

    // Find the Input node and sync settings
    function findInputNode() {
        if (!graph) return null
        for (var i = 0; i < graph.nodeCount; i++) {
            var node = graph.nodeAt(i)
            if (node && node.type === "Input") {
                return node
            }
        }
        return null
    }

    // Sync with Input node settings
    Connections {
        target: {
            var inputNode = findInputNode()
            return inputNode
        }

        function onDurationChanged() {
            var inputNode = findInputNode()
            if (inputNode) {
                root.animationDurationMs = inputNode.duration
            }
        }

        function onBpmChanged() {
            updateFromInputNode()
        }

        function onBeatsPerMeasureChanged() {
            var inputNode = findInputNode()
            if (inputNode) {
                root.beatsPerMeasure = inputNode.beatsPerMeasure
            }
        }

        function onMeasuresChanged() {
            var inputNode = findInputNode()
            if (inputNode) {
                root.measureCount = inputNode.measures
            }
        }
    }

    function updateFromInputNode() {
        var inputNode = findInputNode()
        if (inputNode) {
            root.animationDurationMs = inputNode.duration
            root.beatsPerMeasure = inputNode.beatsPerMeasure
            root.measureCount = inputNode.measures
        }
    }

    // Update when graph changes
    onGraphChanged: {
        updateTrackModel()
        updateFromInputNode()
    }

    Connections {
        target: graph
        function onNodeAdded() {
            updateTrackModel()
            updateFromInputNode()
        }
        function onNodeRemoved() {
            updateTrackModel()
        }
        function onNodePropertyChanged() {
            updateTrackModel()
        }
    }

    Component.onCompleted: {
        updateTrackModel()
        updateFromInputNode()
    }
}
