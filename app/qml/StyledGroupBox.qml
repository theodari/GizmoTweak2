import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

GroupBox {
    id: control

    // Automation properties (optional)
    property bool showAutomation: false
    property var node: null
    property string trackName: ""
    property int paramCount: 1
    property color trackColor: "#4080C0"

    // Automation track reference
    property var automationTrackRef: node && trackName ? node.automationTrack(trackName) : null
    property bool automationEnabled: automationTrackRef ? automationTrackRef.automated : false

    // Listen to track automation changes
    Connections {
        target: control.automationTrackRef
        function onAutomatedChanged() {
            control.automationEnabled = control.automationTrackRef ? control.automationTrackRef.automated : false
        }
    }

    // Update track reference when node changes
    onNodeChanged: automationTrackRef = node && trackName ? node.automationTrack(trackName) : null
    onTrackNameChanged: automationTrackRef = node && trackName ? node.automationTrack(trackName) : null

    signal automationToggled(bool enabled)

    // Function to toggle automation
    function toggleAutomation(enabled) {
        if (!node || !trackName) return

        var track
        if (enabled) {
            // Create or get the track
            track = node.createAutomationTrack(trackName, paramCount, trackColor)
            if (track) {
                track.automated = true
            }
        } else {
            // Disable automation (but keep the track for potential re-enable)
            track = node.automationTrack(trackName)
            if (track) {
                track.automated = false
            }
        }

        // Update track reference to ensure UI updates
        automationTrackRef = track
        automationToggled(enabled)
    }

    topPadding: 26
    leftPadding: 8
    rightPadding: 8
    bottomPadding: 8

    background: Rectangle {
        y: control.topPadding - control.padding
        width: parent.width
        height: parent.height - control.topPadding + control.padding
        color: "transparent"
        border.color: Theme.propGroupBorder
        border.width: 1
        radius: 6
    }

    label: RowLayout {
        x: control.leftPadding
        width: control.availableWidth
        spacing: 4

        Label {
            text: control.title
            color: Theme.propGroupTitle
            font.pixelSize: Theme.propFontSizeTitle
            font.bold: true
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        GearButton {
            visible: control.showAutomation && control.node && control.trackName
            automationEnabled: control.automationEnabled

            onClicked: {
                control.toggleAutomation(!control.automationEnabled)
            }
        }
    }
}
