import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

/**
 * ReadOnlyParameterRow - Displays current parameter value as read-only
 *
 * Same visual layout as KeyframeParameterRow but with:
 * - Progress bar instead of slider
 * - Label instead of spinbox (no +/- buttons, padding in their place)
 * - Invisible placeholder instead of reset button
 */
RowLayout {
    id: root

    // Track context
    property AutomationTrack track: null
    property int paramIndex: 0
    property int timeMs: 0

    // Validate paramIndex is within bounds
    property bool paramIndexValid: track !== null && paramIndex >= 0 && paramIndex < track.paramCount

    // Computed properties from track - updated imperatively
    property string label: ""
    property real minValue: 0
    property real maxValue: 1
    property real displayRatio: 1.0
    property string suffix: ""

    // Current interpolated value
    property real value: 0

    function refreshTrackParams() {
        if (paramIndexValid) {
            label = track.parameterName(paramIndex)
            minValue = track.minValue(paramIndex)
            maxValue = track.maxValue(paramIndex)
            displayRatio = track.displayRatio(paramIndex)
            suffix = track.suffix(paramIndex)
        } else {
            label = ""
            minValue = 0
            maxValue = 1
            displayRatio = 1.0
            suffix = ""
        }
    }

    function refreshValue() {
        if (paramIndexValid)
            value = track.timedValue(timeMs, paramIndex)
        else
            value = 0
    }

    onTrackChanged: { refreshTrackParams(); refreshValue() }
    onTimeMsChanged: refreshValue()
    onParamIndexChanged: { refreshTrackParams(); refreshValue() }
    onParamIndexValidChanged: { refreshTrackParams(); refreshValue() }
    Component.onCompleted: { refreshTrackParams(); refreshValue() }

    Connections {
        target: root.track
        function onKeyFrameModified() { root.refreshValue() }
        function onKeyFrameCountChanged() { root.refreshValue() }
    }

    spacing: 2
    Layout.fillWidth: true
    visible: paramIndexValid

    // Parameter label (same as KeyframeParameterRow)
    Label {
        text: root.label
        color: Theme.propLabel
        font.pixelSize: Theme.propFontSize
        Layout.preferredWidth: 55
        elide: Text.ElideRight
    }

    // Invisible placeholder (same size as reset button)
    Item {
        Layout.preferredWidth: 20
        Layout.preferredHeight: 20
    }

    // Progress bar (same geometry as slider)
    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: 20

        Rectangle {
            x: 0
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            height: 4
            radius: 2
            color: Theme.backgroundLight

            Rectangle {
                width: {
                    var range = root.maxValue - root.minValue
                    if (range <= 0) return 0
                    var ratio = (root.value - root.minValue) / range
                    return Math.max(0, Math.min(1, ratio)) * parent.width
                }
                height: parent.height
                color: root.track ? root.track.color : Theme.accent
                radius: 2
            }
        }

    }

    // Value label (same width as spinbox, with padding for +/- buttons)
    Rectangle {
        Layout.preferredWidth: 65
        Layout.preferredHeight: 22
        color: Theme.surface
        border.color: Theme.border
        radius: 2

        Label {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            text: Math.round(root.value * root.displayRatio) + root.suffix
            color: Theme.textMuted
            font.pixelSize: Theme.propFontSize
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
