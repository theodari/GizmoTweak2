import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

RowLayout {
    id: root

    property string label: ""
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property real defaultValue: 0
    property real stepSize: 1
    property int decimals: 0
    property string suffix: ""
    property real displayRatio: 1.0  // Multiplier for display (e.g., 100 for percentages)
    property bool showAutomation: true

    // Automation properties
    property var node: null           // The node this parameter belongs to
    property string trackName: ""     // Name of the automation track (e.g., "Position", "Scale")
    property int paramIndex: 0        // Index of this parameter in the track
    property int paramCount: 1        // Total number of parameters in the track
    property color trackColor: "#4080C0"  // Color for the automation track

    // Computed automation state
    property bool automationEnabled: {
        if (!node || !trackName) return false
        var track = node.automationTrack(trackName)
        return track ? track.automated : false
    }

    signal valueModified(real newValue)
    signal automationToggled(bool enabled)
    signal resetClicked()

    // Function to toggle automation
    function toggleAutomation(enabled) {
        if (!node || !trackName) return

        if (enabled) {
            // Create or get the track
            var track = node.createAutomationTrack(trackName, paramCount, trackColor)
            if (track) {
                track.automated = true
                // Setup this parameter
                track.setupParameter(paramIndex, minValue, maxValue, value, label, displayRatio, suffix)
            }
        } else {
            // Disable automation (but keep the track for potential re-enable)
            var track = node.automationTrack(trackName)
            if (track) {
                track.automated = false
            }
        }
        automationToggled(enabled)
    }

    spacing: 2
    Layout.fillWidth: true

    // Parameter label
    Label {
        text: root.label
        color: Theme.propLabel
        font.pixelSize: Theme.propFontSize
        Layout.preferredWidth: 55
        elide: Text.ElideRight
    }

    // Slider (JumpSlider behavior)
    Slider {
        id: slider
        Layout.fillWidth: true
        Layout.preferredHeight: 20

        from: root.minValue
        to: root.maxValue
        value: root.value
        stepSize: root.stepSize

        // Update value when slider moves
        onMoved: root.valueModified(value)

        // Double-click to reset
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            propagateComposedEvents: true

            onDoubleClicked: {
                root.value = root.defaultValue
                root.valueModified(root.defaultValue)
                root.resetClicked()
            }

            // Let single clicks through to the slider
            onPressed: function(mouse) {
                // Calculate value from click position (JumpSlider behavior)
                var clickRatio = mouse.x / width
                var newValue = root.minValue + clickRatio * (root.maxValue - root.minValue)
                // Snap to step
                newValue = Math.round(newValue / root.stepSize) * root.stepSize
                newValue = Math.max(root.minValue, Math.min(root.maxValue, newValue))
                slider.value = newValue
                root.valueModified(newValue)
                mouse.accepted = false  // Let slider handle dragging
            }
        }

        background: Rectangle {
            x: slider.leftPadding
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: slider.availableWidth
            height: 4
            radius: 2
            color: Theme.backgroundLight

            Rectangle {
                width: slider.visualPosition * parent.width
                height: parent.height
                color: root.automationEnabled ? Theme.accentBright : Theme.accent
                radius: 2
            }
        }

        handle: Rectangle {
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: 12
            height: 12
            radius: 6
            color: slider.pressed ? Theme.accentBright : (root.automationEnabled ? Theme.accentBright : Theme.accent)
            border.color: Theme.border
        }
    }

    // SpinBox for precise value entry
    SpinBox {
        id: spinBox
        Layout.preferredWidth: 65
        Layout.preferredHeight: 22

        from: Math.round(root.minValue * root.displayRatio)
        to: Math.round(root.maxValue * root.displayRatio)
        value: Math.round(root.value * root.displayRatio)
        stepSize: Math.round(root.stepSize * root.displayRatio)

        editable: true

        onValueModified: {
            var realValue = value / root.displayRatio
            root.valueModified(realValue)
        }

        // Update when external value changes
        Connections {
            target: root
            function onValueChanged() {
                spinBox.value = Math.round(root.value * root.displayRatio)
            }
        }

        textFromValue: function(value, locale) {
            if (root.decimals > 0) {
                return Number(value / root.displayRatio).toFixed(root.decimals) + root.suffix
            }
            return value + root.suffix
        }

        valueFromText: function(text, locale) {
            var num = parseFloat(text.replace(root.suffix, ""))
            return Math.round(num * root.displayRatio)
        }

        background: Rectangle {
            color: Theme.surface
            border.color: Theme.border
            radius: 2
        }

        contentItem: TextInput {
            text: spinBox.textFromValue(spinBox.value, spinBox.locale)
            font.pixelSize: Theme.propFontSize
            color: Theme.text
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            readOnly: !spinBox.editable
            validator: spinBox.validator
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            selectByMouse: true
        }

        up.indicator: Rectangle {
            x: spinBox.mirrored ? 0 : parent.width - width
            height: parent.height
            width: 16
            color: spinBox.up.pressed ? Theme.surfacePressed : (spinBox.up.hovered ? Theme.surfaceHover : "transparent")
            radius: 2

            Text {
                anchors.centerIn: parent
                text: "+"
                color: Theme.text
                font.pixelSize: 10
            }
        }

        down.indicator: Rectangle {
            x: spinBox.mirrored ? parent.width - width : 0
            height: parent.height
            width: 16
            color: spinBox.down.pressed ? Theme.surfacePressed : (spinBox.down.hovered ? Theme.surfaceHover : "transparent")
            radius: 2

            Text {
                anchors.centerIn: parent
                text: "-"
                color: Theme.text
                font.pixelSize: 10
            }
        }
    }

    // Reset button
    Button {
        id: resetButton
        Layout.preferredWidth: 20
        Layout.preferredHeight: 20

        onClicked: {
            root.value = root.defaultValue
            root.valueModified(root.defaultValue)
            root.resetClicked()
        }

        background: Rectangle {
            color: resetButton.down ? Theme.surfacePressed : (resetButton.hovered ? Theme.surfaceHover : Theme.surface)
            border.color: Theme.border
            radius: 4
        }

        contentItem: Text {
            text: "↺"
            color: Theme.text
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        ToolTip.visible: enabled && hovered
        ToolTip.text: qsTr("Reset to default (%1)").arg(root.defaultValue)
        ToolTip.delay: 500
    }

    // Gear button for automation
    GearButton {
        id: gearButton
        visible: root.showAutomation && root.node && root.trackName
        automationEnabled: root.automationEnabled

        onClicked: {
            root.toggleAutomation(!root.automationEnabled)
        }
    }
}
