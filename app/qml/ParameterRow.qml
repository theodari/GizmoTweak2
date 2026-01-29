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
    property bool reserveAutomationSpace: true  // Set to false when used inside ParameterGroup

    // Automation properties
    property var node: null           // The node this parameter belongs to
    property string trackName: ""     // Name of the automation track (e.g., "Position", "Scale")
    property int paramIndex: 0        // Index of this parameter in the track
    property int paramCount: 1        // Total number of parameters in the track
    property color trackColor: "#4080C0"  // Color for the automation track

    // Automation track reference (for signal connections)
    property var automationTrackRef: node && trackName ? node.automationTrack(trackName) : null

    // Computed automation state
    property bool automationEnabled: automationTrackRef ? automationTrackRef.automated : false

    // Listen to track automation changes
    Connections {
        target: automationTrackRef
        function onAutomatedChanged() {
            root.automationEnabled = automationTrackRef ? automationTrackRef.automated : false
        }
    }

    // Update track reference when node changes
    onNodeChanged: automationTrackRef = node && trackName ? node.automationTrack(trackName) : null
    onTrackNameChanged: automationTrackRef = node && trackName ? node.automationTrack(trackName) : null

    signal valueModified(real newValue)
    signal automationToggled(bool enabled)
    signal resetClicked()

    // Function to toggle automation
    function toggleAutomation(enabled) {
        if (!node || !trackName) return

        if (!enabled) {
            confirmDisableDialog.open()
            return
        }

        var track = node.createAutomationTrack(trackName, paramCount, trackColor)
        if (track) {
            track.automated = true
            track.setupParameter(paramIndex, minValue, maxValue, value, label, displayRatio, suffix)
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

    // Reset button (to the left of slider)
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

        contentItem: Item {
            Canvas {
                id: resetCanvas
                anchors.centerIn: parent
                width: 14
                height: 14

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    var c = resetButton.hovered ? Theme.text : Theme.textMuted
                    ctx.fillStyle = c
                    ctx.strokeStyle = c
                    ctx.lineWidth = 1.5

                    var cx = width / 2
                    var cy = height / 2
                    var hw = 5  // half-width of triangles
                    var hh = 5  // half-height of triangles
                    var gap = 1.5  // gap from center line

                    // Left triangle pointing right (toward center)
                    ctx.beginPath()
                    ctx.moveTo(0, cy - hh)
                    ctx.lineTo(0, cy + hh)
                    ctx.lineTo(cx - gap, cy)
                    ctx.closePath()
                    ctx.fill()

                    // Right triangle pointing left (toward center)
                    ctx.beginPath()
                    ctx.moveTo(width, cy - hh)
                    ctx.lineTo(width, cy + hh)
                    ctx.lineTo(cx + gap, cy)
                    ctx.closePath()
                    ctx.fill()

                    // Center vertical line
                    ctx.beginPath()
                    ctx.moveTo(cx, cy - hh)
                    ctx.lineTo(cx, cy + hh)
                    ctx.stroke()
                }

                Connections {
                    target: resetButton
                    function onHoveredChanged() { resetCanvas.requestPaint() }
                }
            }
        }

        ToolTip.visible: enabled && hovered
        ToolTip.text: qsTr("Reset to default (%1)").arg(root.defaultValue)
        ToolTip.delay: 500
    }

    // Slider (JumpSlider behavior)
    Slider {
        id: slider
        Layout.fillWidth: true
        Layout.preferredHeight: 20

        from: root.minValue
        to: root.maxValue
        stepSize: root.stepSize

        // Use Binding to allow programmatic updates without breaking the binding
        Binding on value {
            value: root.value
            when: !slider.pressed
        }

        // Only emit valueModified on release, not during drag
        onPressedChanged: {
            if (!pressed) {
                root.valueModified(value)
            }
        }

        // Double-click to reset
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            propagateComposedEvents: true

            onDoubleClicked: {
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
        stepSize: 1

        editable: true

        // Use Binding to sync with slider value during drag, or root.value otherwise
        Binding on value {
            value: Math.round((slider.pressed ? slider.value : root.value) * root.displayRatio)
            when: !spinBox.activeFocus
        }

        onValueModified: {
            var realValue = value / root.displayRatio
            root.valueModified(realValue)
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

    // Wrapper for gear button - can be hidden completely when inside ParameterGroup
    Item {
        implicitWidth: root.reserveAutomationSpace ? 20 : 0
        implicitHeight: root.reserveAutomationSpace ? 20 : 0
        visible: root.reserveAutomationSpace

        GearButton {
            id: gearButton
            anchors.fill: parent
            visible: root.showAutomation && root.node && root.trackName
            automationEnabled: root.automationEnabled

            onClicked: {
                root.toggleAutomation(!root.automationEnabled)
            }
        }
    }
}

