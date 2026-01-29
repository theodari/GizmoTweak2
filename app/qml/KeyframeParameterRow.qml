import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

/**
 * KeyframeParameterRow - Parameter editor for keyframe values
 *
 * Similar visual style to ParameterRow but adapted for keyframe editing:
 * - No automation button (we're already in automation mode)
 * - Gets/sets values from AutomationTrack keyframes
 * - Reset button resets to track's initial value
 */
RowLayout {
    id: root

    // Track and keyframe context
    property AutomationTrack track: null
    property int keyFrameMs: -1
    property int paramIndex: 0

    // Validate paramIndex is within bounds
    property bool paramIndexValid: track !== null && paramIndex >= 0 && paramIndex < track.paramCount

    // Keyframe color matching the timeline diamond color
    function colorForNodeType(nodeType) {
        switch (nodeType) {
            case "Gizmo": return Theme.nodeGizmo
            case "Transform": return Theme.nodeTransform
            case "SurfaceFactory": return Theme.nodeSurface
            case "Mirror":
            case "TimeShift": return Theme.nodeUtility
            default: return Theme.nodeTweak
        }
    }
    readonly property color keyframeColor: track ? colorForNodeType(track.nodeType) : Theme.accent

    // Computed properties from track - updated imperatively to ensure correct values
    property string label: ""
    property real minValue: 0
    property real maxValue: 1
    property real defaultValue: 0
    property real displayRatio: 1.0
    property string suffix: ""
    property real stepSize: 0.01

    function refreshTrackParams() {
        if (paramIndexValid) {
            label = track.parameterName(paramIndex)
            minValue = track.minValue(paramIndex)
            maxValue = track.maxValue(paramIndex)
            defaultValue = track.initialValue(paramIndex)
            displayRatio = track.displayRatio(paramIndex)
            suffix = track.suffix(paramIndex)
            stepSize = displayRatio > 0 ? (1.0 / displayRatio) : ((maxValue - minValue) / 100)
        } else {
            label = ""
            minValue = 0
            maxValue = 1
            defaultValue = 0
            displayRatio = 1.0
            suffix = ""
            stepSize = 0.01
        }
    }

    // Current value from keyframe (with validation)
    property real value: defaultValue

    // Refresh value from track when keyframe changes or context changes
    function refreshValue() {
        if (paramIndexValid && keyFrameMs >= 0)
            value = track.timedValue(keyFrameMs, paramIndex)
        else
            value = defaultValue
    }

    // Consolidated refresh handlers
    onTrackChanged: { refreshTrackParams(); refreshValue() }
    onParamIndexChanged: { refreshTrackParams(); refreshValue() }
    onParamIndexValidChanged: { refreshTrackParams(); refreshValue() }
    onKeyFrameMsChanged: refreshValue()
    Component.onCompleted: { refreshTrackParams(); refreshValue() }

    Connections {
        target: root.track
        function onKeyFrameModified(timeMs) {
            if (timeMs === root.keyFrameMs)
                root.refreshValue()
        }
    }

    signal valueModified()

    spacing: 2
    Layout.fillWidth: true
    visible: paramIndexValid  // Hide row if paramIndex is out of bounds

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
            if (root.paramIndexValid && root.keyFrameMs >= 0) {
                root.track.updateKeyFrameValue(root.keyFrameMs, root.paramIndex, root.defaultValue)
                root.valueModified()
            }
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
                    var hw = 5
                    var hh = 5
                    var gap = 1.5

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
        ToolTip.text: qsTr("Reset to initial value (%1)").arg(Math.round(root.defaultValue * root.displayRatio) + root.suffix)
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

        // Update keyframe value immediately during drag for real-time preview
        onMoved: {
            if (root.paramIndexValid && root.keyFrameMs >= 0) {
                root.track.updateKeyFrameValue(root.keyFrameMs, root.paramIndex, value)
                root.valueModified()
            }
        }

        // Double-click to reset
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            propagateComposedEvents: true

            onDoubleClicked: {
                if (root.paramIndexValid && root.keyFrameMs >= 0) {
                    root.track.updateKeyFrameValue(root.keyFrameMs, root.paramIndex, root.defaultValue)
                    root.valueModified()
                }
            }

            // Let single clicks through to the slider (JumpSlider behavior)
            onPressed: function(mouse) {
                // Calculate value from click position
                var clickRatio = mouse.x / width
                var newValue = root.minValue + clickRatio * (root.maxValue - root.minValue)
                // Snap to step
                newValue = Math.round(newValue / root.stepSize) * root.stepSize
                newValue = Math.max(root.minValue, Math.min(root.maxValue, newValue))
                if (root.paramIndexValid && root.keyFrameMs >= 0) {
                    root.track.updateKeyFrameValue(root.keyFrameMs, root.paramIndex, newValue)
                    root.valueModified()
                }
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
                color: root.keyframeColor
                radius: 2
            }
        }

        handle: Rectangle {
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: 12
            height: 12
            radius: 6
            color: slider.pressed ? Qt.lighter(root.keyframeColor, 1.2)
                                  : (root.keyframeColor)
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

        // Use Binding to sync with root.value without breaking the binding
        Binding on value {
            value: Math.round(root.value * root.displayRatio)
            when: !spinBox.activeFocus
        }

        onValueModified: {
            if (root.paramIndexValid && root.keyFrameMs >= 0) {
                var realValue = value / root.displayRatio
                root.track.updateKeyFrameValue(root.keyFrameMs, root.paramIndex, realValue)
                root.valueModified()
            }
        }

        textFromValue: function(value, locale) {
            return value + root.suffix
        }

        valueFromText: function(text, locale) {
            var num = parseFloat(text.replace(root.suffix, ""))
            return Math.round(num)
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
}
