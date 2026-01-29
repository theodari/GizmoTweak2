import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

/**
 * ParameterGroup - Groups multiple parameters with a single automation button
 *
 * Displays a vertical rail on the right side of the parameters with a centered
 * automation button. The rail visually indicates which parameters are grouped
 * together for automation.
 *
 * Usage:
 *   ParameterGroup {
 *       node: root.selectedNode
 *       trackName: "Scale"
 *       paramCount: 2
 *       trackColor: "#3CB371"
 *
 *       ParameterRow {
 *           label: "X"
 *           showAutomation: false  // Hide individual button
 *           ...
 *       }
 *       ParameterRow {
 *           label: "Y"
 *           showAutomation: false
 *           ...
 *       }
 *   }
 */
RowLayout {
    id: root

    // Automation properties
    property var node: null
    property string trackName: ""
    property int paramCount: 1
    property color trackColor: "#4080C0"

    // Rail appearance
    property real railWidth: 3
    property real railMargin: 6
    property color railActiveColor: trackColor
    property color railInactiveColor: Theme.border

    // Content (ParameterRows)
    default property alias content: parameterColumn.data

    // Automation track reference
    property var automationTrackRef: node && trackName ? node.automationTrack(trackName) : null
    property bool automationEnabled: automationTrackRef ? automationTrackRef.automated : false

    // Listen to track automation changes
    Connections {
        target: root.automationTrackRef
        function onAutomatedChanged() {
            root.automationEnabled = root.automationTrackRef ? root.automationTrackRef.automated : false
        }
    }

    // Update track reference when node changes
    onNodeChanged: automationTrackRef = node && trackName ? node.automationTrack(trackName) : null
    onTrackNameChanged: automationTrackRef = node && trackName ? node.automationTrack(trackName) : null

    signal automationToggled(bool enabled)

    // Function to toggle automation
    function toggleAutomation(enabled) {
        if (!node || !trackName) return

        if (!enabled) {
            // Show confirmation before disabling
            confirmDisableDialog.open()
            return
        }

        var track = node.createAutomationTrack(trackName, paramCount, trackColor)
        if (track) {
            track.automated = true
        }
        automationTrackRef = track
        automationToggled(true)
    }

    // Confirmation dialog for disabling automation
    Dialog {
        id: confirmDisableDialog
        title: qsTr("Disable Automation")
        modal: true
        anchors.centerIn: Overlay.overlay
        parent: Overlay.overlay

        Label {
            text: qsTr("Remove automation for \"%1\"?\nAll keyframes will be lost.").arg(root.trackName)
            color: Theme.text
        }

        background: Rectangle {
            color: Theme.surface
            border.color: Theme.border
            radius: 4
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Remove")
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
            Button {
                text: qsTr("Cancel")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }

            background: Rectangle { color: "transparent" }
        }

        onAccepted: {
            var track = root.node.automationTrack(root.trackName)
            if (track) {
                track.automated = false
            }
            root.automationTrackRef = track
            root.automationToggled(false)
        }
    }

    spacing: 0
    Layout.fillWidth: true

    // Column containing the ParameterRows
    ColumnLayout {
        id: parameterColumn
        Layout.fillWidth: true
        spacing: 4
    }

    // Rail + Button container
    Item {
        id: railContainer
        Layout.preferredWidth: railWidth + railMargin * 2 + 20  // rail + margins + button width
        Layout.fillHeight: true
        Layout.minimumHeight: parameterColumn.implicitHeight
        visible: root.node && root.trackName

        // Vertical rail
        Rectangle {
            id: rail
            anchors.left: parent.left
            anchors.leftMargin: railMargin
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: railWidth
            radius: railWidth / 2
            color: root.automationEnabled ? root.railActiveColor : root.railInactiveColor
            opacity: root.automationEnabled ? 1.0 : 0.5

            Behavior on color {
                ColorAnimation { duration: 150 }
            }
            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }

        // Centered automation button
        GearButton {
            id: gearButton
            anchors.left: rail.right
            anchors.leftMargin: railMargin
            anchors.verticalCenter: parent.verticalCenter
            automationEnabled: root.automationEnabled

            onClicked: {
                root.toggleAutomation(!root.automationEnabled)
            }
        }
    }
}
