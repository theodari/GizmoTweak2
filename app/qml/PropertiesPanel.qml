import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import GizmoTweakLib2
import GizmoTweak2

Rectangle {
    id: root

    property var selectedNode: null
    property NodeGraph graph: null  // Graph reference for timeline settings
    property alias colorDialog: colorDialog
    property var colorDialogCallback: null

    signal collapseRequested()

    color: Theme.surface
    border.color: Theme.border
    border.width: 1

    ColorDialog {
        id: colorDialog
        title: qsTr("Choose a color")
        selectedColor: "#FFFFFF"
        onAccepted: {
            if (root.colorDialogCallback) {
                root.colorDialogCallback(selectedColor)
                root.colorDialogCallback = null
            }
        }
    }

    // Preset storage
    Settings {
        id: presetSettings
        category: "Presets"
        // Presets stored as JSON string per node type: "PositionTweak" -> '{"presets":[{"name":"Default","data":{...}}]}'
    }

    // Helper functions for presets
    function getPresetsForType(nodeType) {
        var key = nodeType + "_presets"
        var stored = presetSettings.value(key, "")
        if (stored === "") return []
        try {
            return JSON.parse(stored)
        } catch(e) {
            return []
        }
    }

    function savePresetsForType(nodeType, presets) {
        var key = nodeType + "_presets"
        presetSettings.setValue(key, JSON.stringify(presets))
    }

    function savePreset(name) {
        if (!root.selectedNode || !name) return
        var nodeType = root.selectedNode.type
        var presets = getPresetsForType(nodeType)
        var data = root.selectedNode.propertiesToJson()

        // Check if preset with same name exists
        var found = false
        for (var i = 0; i < presets.length; i++) {
            if (presets[i].name === name) {
                presets[i].data = data
                found = true
                break
            }
        }
        if (!found) {
            presets.push({ name: name, data: data })
        }
        savePresetsForType(nodeType, presets)
        presetModel = getPresetsForType(nodeType)
    }

    function loadPreset(presetName) {
        if (!root.selectedNode || !presetName) return
        var nodeType = root.selectedNode.type
        var presets = getPresetsForType(nodeType)

        for (var i = 0; i < presets.length; i++) {
            if (presets[i].name === presetName) {
                root.selectedNode.propertiesFromJson(presets[i].data)
                break
            }
        }
    }

    function deletePreset(presetName) {
        if (!root.selectedNode || !presetName) return
        var nodeType = root.selectedNode.type
        var presets = getPresetsForType(nodeType)

        presets = presets.filter(function(p) { return p.name !== presetName })
        savePresetsForType(nodeType, presets)
        presetModel = getPresetsForType(nodeType)
    }

    // Current preset model
    property var presetModel: []

    onSelectedNodeChanged: {
        if (selectedNode) {
            presetModel = getPresetsForType(selectedNode.type)
        } else {
            presetModel = []
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 6

        // Header with title and close button
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: root.selectedNode ? root.selectedNode.type : qsTr("Properties")
                color: Theme.text
                font.pixelSize: Theme.fontSizeLarge
                font.bold: true
                Layout.fillWidth: true
            }

            // Close button
            Rectangle {
                width: 24
                height: 24
                radius: 4
                color: closeMouseArea.containsMouse ? Theme.surfaceHover : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "\u2715"  // Unicode X mark
                    color: closeMouseArea.containsMouse ? Theme.text : Theme.textMuted
                    font.pixelSize: Theme.fontSizeNormal
                }

                MouseArea {
                    id: closeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.collapseRequested()
                }
            }
        }

        Rectangle {
            height: 1
            color: Theme.border
            Layout.fillWidth: true
        }

        // Scrollable content
        ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: scrollView.availableWidth
                spacing: 12

                // Common properties (only when a node is selected)
                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTr("General")
                    visible: root.selectedNode !== null

                    GridLayout {
                        columns: 2
                        columnSpacing: 8
                        rowSpacing: 8
                        anchors.fill: parent

                        Label {
                            text: qsTr("Name:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                        }
                        TextField {
                            id: nameField
                            Layout.fillWidth: true
                            text: root.selectedNode ? root.selectedNode.displayName : ""
                            color: Theme.text
                            font.pixelSize: Theme.propFontSize
                            implicitHeight: Theme.propSpinBoxHeight
                            background: Rectangle {
                                color: Theme.background
                                border.color: nameField.activeFocus ? Theme.accent : Theme.border
                                radius: 4
                            }
                            onEditingFinished: {
                                if (root.selectedNode) {
                                    root.selectedNode.displayName = text
                                }
                            }
                        }
                    }
                }

                // Presets section (only for tweaks and some other node types)
                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Presets")
                    visible: root.selectedNode && root.selectedNode.category === Node.Category.Tweak

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        // Load preset row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            ComboBox {
                                id: presetCombo
                                Layout.fillWidth: true
                                Layout.preferredHeight: 24
                                model: root.presetModel.length > 0 ? root.presetModel.map(function(p) { return p.name }) : [qsTr("(No presets)")]
                                enabled: root.presetModel.length > 0

                                background: Rectangle {
                                    implicitHeight: 24
                                    color: Theme.background
                                    border.color: presetCombo.enabled ? Theme.border : Theme.borderLight
                                    radius: 3
                                }

                                contentItem: Text {
                                    text: presetCombo.displayText
                                    color: presetCombo.enabled ? Theme.text : Theme.textMuted
                                    font.pixelSize: Theme.propFontSize
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 6
                                }
                            }

                            Button {
                                text: qsTr("Load")
                                implicitWidth: 50
                                implicitHeight: 24
                                enabled: root.presetModel.length > 0

                                background: Rectangle {
                                    color: parent.down ? Theme.surfacePressed : (parent.hovered ? Theme.surfaceHover : Theme.surface)
                                    border.color: Theme.border
                                    radius: 3
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: parent.enabled ? Theme.text : Theme.textMuted
                                    font.pixelSize: Theme.fontSizeSmall
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: {
                                    if (root.presetModel.length > 0) {
                                        root.loadPreset(root.presetModel[presetCombo.currentIndex].name)
                                    }
                                }
                            }

                            Button {
                                text: "\u2715"  // X mark
                                implicitWidth: 24
                                implicitHeight: 24
                                enabled: root.presetModel.length > 0

                                background: Rectangle {
                                    color: parent.down ? Theme.surfacePressed : (parent.hovered ? "#663333" : Theme.surface)
                                    border.color: Theme.border
                                    radius: 3
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: parent.enabled ? Theme.text : Theme.textMuted
                                    font.pixelSize: Theme.fontSizeSmall
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: {
                                    if (root.presetModel.length > 0) {
                                        root.deletePreset(root.presetModel[presetCombo.currentIndex].name)
                                    }
                                }

                                ToolTip.visible: hovered
                                ToolTip.text: qsTr("Delete preset")
                                ToolTip.delay: 300
                            }
                        }

                        // Save preset row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            TextField {
                                id: presetNameField
                                Layout.fillWidth: true
                                implicitHeight: 24
                                placeholderText: qsTr("Preset name...")
                                color: Theme.text
                                font.pixelSize: Theme.propFontSize

                                background: Rectangle {
                                    color: Theme.background
                                    border.color: presetNameField.activeFocus ? Theme.accent : Theme.border
                                    radius: 3
                                }
                            }

                            Button {
                                text: qsTr("Save")
                                implicitWidth: 50
                                implicitHeight: 24
                                enabled: presetNameField.text.length > 0

                                background: Rectangle {
                                    color: parent.down ? Theme.surfacePressed : (parent.hovered ? Theme.surfaceHover : Theme.surface)
                                    border.color: parent.enabled ? Theme.accent : Theme.border
                                    radius: 3
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: parent.enabled ? Theme.text : Theme.textMuted
                                    font.pixelSize: Theme.fontSizeSmall
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: {
                                    root.savePreset(presetNameField.text)
                                    presetNameField.text = ""
                                }
                            }
                        }
                    }
                }

                // Type-specific properties
                Loader {
                    id: propertiesLoader
                    Layout.fillWidth: true
                    visible: sourceComponent !== null

                    sourceComponent: {
                        if (!root.selectedNode) return null
                        switch (root.selectedNode.type) {
                            case "Input": return inputProperties
                            case "Output": return outputProperties
                            case "Gizmo": return gizmoProperties
                            case "Transform": return transformProperties
                            case "TimeShift": return timeShiftProperties
                            case "SurfaceFactory": return surfaceFactoryProperties
                            case "Mirror": return mirrorProperties
                            case "PositionTweak": return positionTweakProperties
                            case "ScaleTweak": return scaleTweakProperties
                            case "RotationTweak": return rotationTweakProperties
                            case "ColorTweak": return colorTweakProperties
                            case "PolarTweak": return polarTweakProperties
                            case "WaveTweak": return waveTweakProperties
                            case "SqueezeTweak": return squeezeTweakProperties
                            case "SparkleTweak": return sparkleTweakProperties
                            case "FuzzynessTweak": return fuzzynessTweakProperties
                            case "ColorFuzzynessTweak": return colorFuzzynessTweakProperties
                            case "SplitTweak": return splitTweakProperties
                            case "RounderTweak": return rounderTweakProperties
                            default: return null
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }
    }

    // Input properties
    Component {
        id: inputProperties
        ColumnLayout {
            spacing: 8

            // Preview with player
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Preview")

                ColumnLayout {
                    spacing: 8
                    anchors.left: parent.left
                    anchors.right: parent.right

                    // Preview canvas
                    Rectangle {
                        id: inputPreviewContainer
                        Layout.fillWidth: true
                        Layout.preferredHeight: 200
                        color: Theme.previewBackground
                        radius: 4
                        border.color: Theme.border
                        border.width: 1

                        Canvas {
                            id: inputPreviewCanvas
                            anchors.fill: parent
                            anchors.margins: 4

                            property var currentFrame: root.selectedNode && root.selectedNode.currentFrame ? root.selectedNode.currentFrame : null
                            property real animTime: inputTimeSlider.value / 100.0

                            onCurrentFrameChanged: requestPaint()
                            onAnimTimeChanged: requestPaint()

                            Connections {
                                target: inputPreviewCanvas.currentFrame
                                function onSamplesChanged() { inputPreviewCanvas.requestPaint() }
                            }

                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.reset()
                                ctx.fillStyle = Theme.previewBackground
                                ctx.fillRect(0, 0, width, height)

                                if (!currentFrame) return

                                var count = currentFrame.sampleCount
                                if (count === 0) return

                                ctx.lineWidth = 2
                                ctx.lineCap = "round"
                                ctx.lineJoin = "round"

                                var centerX = width / 2
                                var centerY = height / 2
                                var scale = Math.min(width, height) / 2 * 0.9

                                for (var i = 0; i < count - 1; i++) {
                                    var x1 = currentFrame.sampleX(i)
                                    var y1 = currentFrame.sampleY(i)
                                    var x2 = currentFrame.sampleX(i + 1)
                                    var y2 = currentFrame.sampleY(i + 1)

                                    var r = Math.round(currentFrame.sampleR(i) * 255)
                                    var g = Math.round(currentFrame.sampleG(i) * 255)
                                    var b = Math.round(currentFrame.sampleB(i) * 255)

                                    ctx.strokeStyle = "rgb(" + r + "," + g + "," + b + ")"
                                    ctx.beginPath()
                                    ctx.moveTo(centerX + x1 * scale, centerY - y1 * scale)
                                    ctx.lineTo(centerX + x2 * scale, centerY - y2 * scale)
                                    ctx.stroke()
                                }
                            }
                        }
                    }

                    // Player controls
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Button {
                            id: inputPlayButton
                            implicitWidth: 28
                            implicitHeight: 28

                            property bool playing: false

                            onClicked: {
                                playing = !playing
                                if (!playing) {
                                    inputTimeSlider.value = 0
                                    inputAnimTimer.elapsed = 0
                                }
                            }

                            background: Rectangle {
                                color: inputPlayButton.down ? Theme.surfacePressed : (inputPlayButton.hovered ? Theme.surfaceHover : Theme.surface)
                                border.color: inputPlayButton.playing ? Theme.accent : Theme.border
                                border.width: inputPlayButton.playing ? 2 : 1
                                radius: 4
                            }

                            contentItem: Item {
                                Text {
                                    anchors.centerIn: parent
                                    text: inputPlayButton.playing ? "\u25A0" : "\u25B6"
                                    color: inputPlayButton.playing ? Theme.accent : Theme.text
                                    font.pixelSize: Math.min(inputPlayButton.width, inputPlayButton.height) * 0.8
                                }
                            }
                        }

                        Slider {
                            id: inputTimeSlider
                            from: 0
                            to: 100
                            value: 0
                            stepSize: 1
                            Layout.fillWidth: true
                        }

                        Label {
                            text: Math.round(inputTimeSlider.value) + "%"
                            color: Theme.text
                            font.pixelSize: Theme.fontSizeSmall
                            Layout.preferredWidth: 35
                        }
                    }

                    // Animation timer
                    Timer {
                        id: inputAnimTimer
                        interval: 16
                        running: inputPlayButton.playing
                        repeat: true
                        property real elapsed: 0
                        onTriggered: {
                            elapsed += interval / 1000.0 * 100
                            inputTimeSlider.value = elapsed % 100
                        }
                    }
                }
            }

            // Source settings
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Source")

                ColumnLayout {
                    spacing: 6
                    anchors.left: parent.left
                    anchors.right: parent.right

                    RowLayout {
                        spacing: 8
                        Layout.fillWidth: true
                        Label { text: qsTr("Type:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 60 }
                        StyledComboBox {
                            id: sourceTypeCombo
                            model: [qsTr("Pattern"), qsTr("Frame"), qsTr("Frames"), qsTr("Stack"), qsTr("Live")]
                            currentIndex: root.selectedNode ? root.selectedNode.sourceType : 0
                            onActivated: if (root.selectedNode) root.selectedNode.sourceType = currentIndex
                            Layout.fillWidth: true
                        }
                    }

                    // Pattern selection with mini-preview (only visible when source is Pattern)
                    ColumnLayout {
                        spacing: 4
                        visible: sourceTypeCombo.currentIndex === 0
                        Layout.fillWidth: true

                        Label {
                            text: qsTr("Pattern:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                        }

                        // Pattern grid with mini-previews
                        GridLayout {
                            columns: 4
                            columnSpacing: 4
                            rowSpacing: 4
                            Layout.fillWidth: true

                            Repeater {
                                model: root.selectedNode ? root.selectedNode.patternNames : []

                                Rectangle {
                                    width: 70
                                    height: 70
                                    color: (root.selectedNode && root.selectedNode.patternIndex === index) ? Theme.accent : Theme.surface
                                    border.color: patternMouseArea.containsMouse ? Theme.textHighlight : Theme.border
                                    border.width: (root.selectedNode && root.selectedNode.patternIndex === index) ? 2 : 1
                                    radius: 4

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        spacing: 2

                                        // Mini preview from image provider
                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            color: Theme.previewBackground
                                            radius: 2
                                            clip: true

                                            Image {
                                                anchors.fill: parent
                                                anchors.margins: 2
                                                source: "image://patterns/" + index
                                                sourceSize: Qt.size(64, 64)
                                                cache: false
                                                fillMode: Image.PreserveAspectFit
                                            }
                                        }

                                        // Pattern name
                                        Label {
                                            text: modelData
                                            color: (root.selectedNode && root.selectedNode.patternIndex === index) ? Theme.textOnHighlight : Theme.text
                                            font.pixelSize: 9
                                            elide: Text.ElideRight
                                            horizontalAlignment: Text.AlignHCenter
                                            Layout.fillWidth: true
                                        }
                                    }

                                    MouseArea {
                                        id: patternMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: if (root.selectedNode) root.selectedNode.patternIndex = index
                                        onEntered: {
                                            // Preview pattern on hover
                                            if (root.selectedNode) root.selectedNode.previewPatternIndex = index
                                        }
                                        onExited: {
                                            // Restore current pattern
                                            if (root.selectedNode) root.selectedNode.previewPatternIndex = -1
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Info for other source types
                    RowLayout {
                        spacing: 8
                        visible: sourceTypeCombo.currentIndex !== 0
                        Layout.fillWidth: true

                        Label {
                            text: qsTr("Status:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 60
                        }
                        Label {
                            text: {
                                switch (sourceTypeCombo.currentIndex) {
                                    case 1: return qsTr("Waiting for frame...")
                                    case 2: return qsTr("Waiting for frames...")
                                    case 3: return qsTr("Waiting for stack...")
                                    case 4: return qsTr("Waiting for live input...")
                                    default: return ""
                                }
                            }
                            color: Theme.textHighlight
                            font.pixelSize: Theme.propFontSize
                            font.italic: true
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // Frame info
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Frame Info")
                visible: root.selectedNode && root.selectedNode.currentFrame

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Samples:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 60 }
                    Label {
                        text: root.selectedNode && root.selectedNode.currentFrame ? String(root.selectedNode.currentFrame.sampleCount) : "0"
                        color: Theme.text
                        font.pixelSize: Theme.propFontSize
                    }
                }
            }

            // Timeline settings
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Timeline")

                ColumnLayout {
                    spacing: 6
                    anchors.left: parent.left
                    anchors.right: parent.right

                    // Toggle between BPM timing and direct duration
                    RowLayout {
                        spacing: 8
                        Layout.fillWidth: true

                        StyledCheckBox {
                            id: useBpmTimingCheck
                            text: qsTr("Use BPM Timing")
                            checked: root.selectedNode ? root.selectedNode.useBpmTiming : true
                            onCheckedChanged: if (root.selectedNode) root.selectedNode.useBpmTiming = checked
                        }
                    }

                    // BPM settings (visible when useBpmTiming is true)
                    ColumnLayout {
                        spacing: 6
                        visible: useBpmTimingCheck.checked
                        Layout.fillWidth: true

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            Label { text: qsTr("BPM:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 60 }
                            StyledSpinBox {
                                id: inputBpmSpinBox
                                from: 1
                                to: 999
                                stepSize: 1
                                editable: true
                                Binding on value {
                                    value: root.selectedNode ? root.selectedNode.bpm : 120
                                    when: !inputBpmSpinBox.activeFocus
                                }
                                onValueModified: if (root.selectedNode) root.selectedNode.bpm = value
                                Layout.fillWidth: true
                            }
                        }

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            Label { text: qsTr("Beats:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 60 }
                            StyledSpinBox {
                                id: inputBeatsSpinBox
                                from: 1
                                to: 32
                                stepSize: 1
                                editable: true
                                Binding on value {
                                    value: root.selectedNode ? root.selectedNode.beatsPerMeasure : 4
                                    when: !inputBeatsSpinBox.activeFocus
                                }
                                onValueModified: if (root.selectedNode) root.selectedNode.beatsPerMeasure = value
                                Layout.preferredWidth: 80
                            }
                            Label { text: qsTr("per measure"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                        }

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            Label { text: qsTr("Measures:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 60 }
                            StyledSpinBox {
                                id: inputMeasuresSpinBox
                                from: 1
                                to: 999
                                stepSize: 1
                                editable: true
                                Binding on value {
                                    value: root.selectedNode ? root.selectedNode.measures : 8
                                    when: !inputMeasuresSpinBox.activeFocus
                                }
                                onValueModified: if (root.selectedNode) root.selectedNode.measures = value
                                Layout.fillWidth: true
                            }
                        }
                    }

                    // Direct duration (visible when useBpmTiming is false)
                    RowLayout {
                        spacing: 8
                        visible: !useBpmTimingCheck.checked
                        Layout.fillWidth: true

                        Label { text: qsTr("Duration:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 60 }
                        StyledSpinBox {
                            id: inputDurationSpinBox
                            from: 100
                            to: 600000  // 10 minutes max
                            stepSize: 100
                            editable: true
                            Binding on value {
                                value: root.selectedNode ? root.selectedNode.duration : 10000
                                when: !inputDurationSpinBox.activeFocus
                            }
                            onValueModified: if (root.selectedNode) root.selectedNode.duration = value
                            Layout.fillWidth: true
                        }
                        Label { text: qsTr("ms"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                    }

                    // Calculated duration display
                    RowLayout {
                        spacing: 8
                        Layout.fillWidth: true

                        Label { text: qsTr("Total:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 60 }
                        Label {
                            text: {
                                var ms = root.selectedNode ? root.selectedNode.duration : 0
                                var sec = Math.floor(ms / 1000)
                                var min = Math.floor(sec / 60)
                                sec = sec % 60
                                var msRemainder = ms % 1000
                                return min + ":" + String(sec).padStart(2, '0') + "." + String(msRemainder).padStart(3, '0')
                            }
                            color: Theme.text
                            font.pixelSize: Theme.propFontSize
                            font.family: "monospace"
                        }
                    }
                }
            }
        }
    }

    // Output properties
    Component {
        id: outputProperties
        ColumnLayout {
            spacing: 8

            // Zone settings
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Output Zone")


                GridLayout {
                    columns: 2
                    columnSpacing: 8
                    rowSpacing: 6
                    anchors.fill: parent

                    Label { text: qsTr("Engine:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                    Label {
                        text: laserEngine ? laserEngine.engineName : qsTr("Not connected")
                        color: laserEngine && laserEngine.connected ? Theme.text : Theme.textMuted
                        font.pixelSize: Theme.propFontSize
                    }

                    Label { text: qsTr("Zone:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                    StyledComboBox {
                        model: laserEngine ? laserEngine.zones : []
                        currentIndex: root.selectedNode ? root.selectedNode.zoneIndex : 0
                        enabled: laserEngine && laserEngine.connected
                        onActivated: if (root.selectedNode) root.selectedNode.zoneIndex = currentIndex
                        Layout.fillWidth: true
                    }

                    Label { text: qsTr("Enabled:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                    CheckBox {
                        checked: root.selectedNode ? root.selectedNode.enabled : true
                        onToggled: if (root.selectedNode) root.selectedNode.enabled = checked
                    }

                    Label { text: qsTr("Status:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                    Label {
                        text: laserEngine && laserEngine.connected ? qsTr("Connected") : qsTr("Disconnected")
                        color: laserEngine && laserEngine.connected ? Theme.success : Theme.error
                        font.pixelSize: Theme.propFontSize
                        font.bold: true
                    }
                }
            }

            // Engine info
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Engine Info")
                visible: laserEngine && laserEngine.connected


                GridLayout {
                    columns: 2
                    columnSpacing: 8
                    rowSpacing: 6
                    anchors.fill: parent

                    Label { text: qsTr("Zones:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                    Label {
                        text: laserEngine && laserEngine.zoneCount !== undefined ? String(laserEngine.zoneCount) : "0"
                        color: Theme.text
                        font.pixelSize: Theme.propFontSize
                    }
                }
            }

            // Timeline settings
            StyledGroupBox {
                id: timelineGroupBox
                Layout.fillWidth: true
                title: qsTr("Timeline")

                // Find Input node to sync settings
                property var inputNode: findInputNode()

                function findInputNode() {
                    if (!root.graph) return null
                    for (var i = 0; i < root.graph.nodeCount; i++) {
                        var node = root.graph.nodeAt(i)
                        if (node && node.type === "Input") {
                            return node
                        }
                    }
                    return null
                }

                // Refresh input node when graph changes
                Connections {
                    target: root.graph
                    function onNodeAdded() { timelineGroupBox.inputNode = timelineGroupBox.findInputNode() }
                    function onNodeRemoved() { timelineGroupBox.inputNode = timelineGroupBox.findInputNode() }
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    // Duration row with mode buttons
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Label {
                            text: qsTr("Duration:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 60
                        }

                        SpinBox {
                            id: durationSpinBox
                            from: 1000
                            to: 600000
                            stepSize: 1000
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 24

                            Binding on value {
                                value: timelineGroupBox.inputNode ? timelineGroupBox.inputNode.duration : 10000
                                when: !durationSpinBox.activeFocus
                            }

                            property int decimals: 1
                            property real realValue: value / 1000.0

                            textFromValue: function(value, locale) {
                                return Number(value / 1000).toFixed(1) + " s"
                            }

                            valueFromText: function(text, locale) {
                                return parseFloat(text) * 1000
                            }

                            onValueModified: {
                                if (timelineGroupBox.inputNode) {
                                    timelineGroupBox.inputNode.duration = value
                                }
                            }

                            background: Rectangle {
                                color: Theme.surface
                                border.color: Theme.border
                                radius: 2
                            }

                            contentItem: TextInput {
                                text: durationSpinBox.textFromValue(durationSpinBox.value, durationSpinBox.locale)
                                font.pixelSize: Theme.propFontSize
                                color: Theme.text
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !durationSpinBox.editable
                                validator: durationSpinBox.validator
                            }
                        }

                        // Duration mode buttons
                        Button {
                            id: keepTimeBtn
                            implicitWidth: 24
                            implicitHeight: 24
                            checkable: true
                            checked: true

                            background: Rectangle {
                                color: keepTimeBtn.checked ? Theme.accent : (keepTimeBtn.hovered ? Theme.surfaceHover : Theme.surface)
                                border.color: Theme.border
                                radius: 4
                            }

                            contentItem: Item {
                                Text {
                                    anchors.centerIn: parent
                                    text: "\u23F1"  // Stopwatch
                                    color: keepTimeBtn.checked ? Theme.textOnHighlight : Theme.text
                                    font.pixelSize: Math.min(keepTimeBtn.width, keepTimeBtn.height) * 0.8
                                }
                            }

                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Keep Time - Add/remove time at end")
                            ToolTip.delay: 300

                            onClicked: {
                                keepTimeBtn.checked = true
                                stretchBtn.checked = false
                                keepCountdownBtn.checked = false
                            }
                        }

                        Button {
                            id: stretchBtn
                            implicitWidth: 24
                            implicitHeight: 24
                            checkable: true

                            background: Rectangle {
                                color: stretchBtn.checked ? Theme.accent : (stretchBtn.hovered ? Theme.surfaceHover : Theme.surface)
                                border.color: Theme.border
                                radius: 4
                            }

                            contentItem: Item {
                                Text {
                                    anchors.centerIn: parent
                                    text: "\u2194"  // Left-right arrow (stretch)
                                    color: stretchBtn.checked ? Theme.textOnHighlight : Theme.text
                                    font.pixelSize: Math.min(stretchBtn.width, stretchBtn.height) * 0.8
                                }
                            }

                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Stretch - Scale effects to fit new duration")
                            ToolTip.delay: 300

                            onClicked: {
                                keepTimeBtn.checked = false
                                stretchBtn.checked = true
                                keepCountdownBtn.checked = false
                            }
                        }

                        Button {
                            id: keepCountdownBtn
                            implicitWidth: 24
                            implicitHeight: 24
                            checkable: true

                            background: Rectangle {
                                color: keepCountdownBtn.checked ? Theme.accent : (keepCountdownBtn.hovered ? Theme.surfaceHover : Theme.surface)
                                border.color: Theme.border
                                radius: 4
                            }

                            contentItem: Item {
                                Text {
                                    anchors.centerIn: parent
                                    text: "\u23F0"  // Alarm clock (countdown)
                                    color: keepCountdownBtn.checked ? Theme.textOnHighlight : Theme.text
                                    font.pixelSize: Math.min(keepCountdownBtn.width, keepCountdownBtn.height) * 0.8
                                }
                            }

                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Keep Countdown - Add/remove time at beginning")
                            ToolTip.delay: 300

                            onClicked: {
                                keepTimeBtn.checked = false
                                stretchBtn.checked = false
                                keepCountdownBtn.checked = true
                            }
                        }
                    }

                    // Tempo/BPM row
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Label {
                            text: qsTr("Tempo:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 60
                        }

                        SpinBox {
                            id: bpmSpinBox
                            from: 10
                            to: 600
                            stepSize: 1
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 24

                            Binding on value {
                                value: timelineGroupBox.inputNode ? timelineGroupBox.inputNode.bpm : 120
                                when: !bpmSpinBox.activeFocus
                            }

                            textFromValue: function(value, locale) {
                                return value + " BPM"
                            }

                            valueFromText: function(text, locale) {
                                return parseInt(text)
                            }

                            onValueModified: {
                                if (timelineGroupBox.inputNode) {
                                    timelineGroupBox.inputNode.bpm = value
                                }
                            }

                            background: Rectangle {
                                color: Theme.surface
                                border.color: Theme.border
                                radius: 2
                            }

                            contentItem: TextInput {
                                text: bpmSpinBox.textFromValue(bpmSpinBox.value, bpmSpinBox.locale)
                                font.pixelSize: Theme.propFontSize
                                color: Theme.text
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !bpmSpinBox.editable
                                validator: bpmSpinBox.validator
                            }
                        }

                        Item { Layout.fillWidth: true }

                        // Total beats (read-only)
                        Label {
                            text: qsTr("Total:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                        }

                        Rectangle {
                            width: 40
                            height: 24
                            color: Theme.backgroundLight
                            border.color: Theme.border
                            radius: 2

                            Label {
                                anchors.centerIn: parent
                                text: {
                                    if (timelineGroupBox.inputNode) {
                                        return String(timelineGroupBox.inputNode.beatsPerMeasure * timelineGroupBox.inputNode.measures)
                                    }
                                    return "8"
                                }
                                color: Theme.text
                                font.pixelSize: Theme.propFontSize
                                font.bold: true
                            }
                        }
                    }

                    // Beats and Measures row
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Label {
                            text: qsTr("Beats:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 60
                        }

                        SpinBox {
                            id: beatsSpinBox
                            from: 1
                            to: 16
                            Layout.preferredWidth: 50
                            Layout.preferredHeight: 24

                            Binding on value {
                                value: timelineGroupBox.inputNode ? timelineGroupBox.inputNode.beatsPerMeasure : 4
                                when: !beatsSpinBox.activeFocus
                            }

                            onValueModified: {
                                if (timelineGroupBox.inputNode) {
                                    timelineGroupBox.inputNode.beatsPerMeasure = value
                                }
                            }

                            background: Rectangle {
                                color: Theme.surface
                                border.color: Theme.border
                                radius: 2
                            }

                            contentItem: TextInput {
                                text: beatsSpinBox.textFromValue(beatsSpinBox.value, beatsSpinBox.locale)
                                font.pixelSize: Theme.propFontSize
                                color: Theme.text
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !beatsSpinBox.editable
                                validator: beatsSpinBox.validator
                            }
                        }

                        Label {
                            text: qsTr("Measures:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                        }

                        SpinBox {
                            id: measuresSpinBox
                            from: 1
                            to: 200
                            Layout.preferredWidth: 50
                            Layout.preferredHeight: 24

                            Binding on value {
                                value: timelineGroupBox.inputNode ? timelineGroupBox.inputNode.measures : 2
                                when: !measuresSpinBox.activeFocus
                            }

                            onValueModified: {
                                if (timelineGroupBox.inputNode) {
                                    timelineGroupBox.inputNode.measures = value
                                }
                            }

                            background: Rectangle {
                                color: Theme.surface
                                border.color: Theme.border
                                radius: 2
                            }

                            contentItem: TextInput {
                                text: measuresSpinBox.textFromValue(measuresSpinBox.value, measuresSpinBox.locale)
                                font.pixelSize: Theme.propFontSize
                                color: Theme.text
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !measuresSpinBox.editable
                                validator: measuresSpinBox.validator
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    // Gizmo properties
    Component {
        id: gizmoProperties
        ColumnLayout {
            spacing: 8

            // Shape selection
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Shape")


                RowLayout {
                    anchors.fill: parent
                    spacing: 6

                    // Shape icon buttons using original GizmoTweak icons
                    Repeater {
                        model: [
                            { shape: 0, name: qsTr("Rectangle"), icon: "qrc:/resources/icons/gizmo_rectangle.png" },
                            { shape: 1, name: qsTr("Ellipse"), icon: "qrc:/resources/icons/gizmo_ellipse.png" },
                            { shape: 2, name: qsTr("Angle"), icon: "qrc:/resources/icons/gizmo_angle.png" },
                            { shape: 3, name: qsTr("Linear Wave"), icon: "qrc:/resources/icons/gizmo_linear_waves.png" },
                            { shape: 4, name: qsTr("Circular Wave"), icon: "qrc:/resources/icons/gizmo_circular_waves.png" }
                        ]

                        delegate: Rectangle {
                            width: 48
                            height: 48
                            radius: 6
                            color: root.selectedNode && root.selectedNode.shape === modelData.shape ? Theme.accent : Theme.surface
                            border.color: shapeMouseArea.containsMouse ? Theme.accent : Theme.border
                            border.width: root.selectedNode && root.selectedNode.shape === modelData.shape ? 2 : 1

                            Image {
                                id: shapeIcon
                                anchors.fill: parent
                                anchors.margins: 4
                                source: modelData.icon
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                mipmap: true
                            }

                            MouseArea {
                                id: shapeMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: if (root.selectedNode) root.selectedNode.shape = modelData.shape

                                ToolTip.visible: modelData && containsMouse
                                ToolTip.text: modelData ? modelData.name : ""
                                ToolTip.delay: 300
                            }
                        }
                    }
                }
            }

            // Size
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Size")

                ParameterGroup {
                    anchors.fill: parent
                    node: root.selectedNode
                    trackName: "Scale"
                    paramCount: 2
                    trackColor: "#FFA500"

                    ParameterRow {
                        label: qsTr("Scale X")
                        value: root.selectedNode ? root.selectedNode.scaleX : 1.0
                        minValue: 0.01
                        maxValue: 3.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        showAutomation: false
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.scaleX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Scale Y")
                        value: root.selectedNode ? root.selectedNode.scaleY : 1.0
                        minValue: 0.01
                        maxValue: 3.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        showAutomation: false
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.scaleY = newValue }
                    }
                }
            }

            // Position
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Position")

                ParameterGroup {
                    anchors.fill: parent
                    node: root.selectedNode
                    trackName: "Position"
                    paramCount: 2
                    trackColor: "#BA55D3"

                    ParameterRow {
                        label: qsTr("Position X")
                        value: root.selectedNode ? root.selectedNode.centerX : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        showAutomation: false
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Position Y")
                        value: root.selectedNode ? root.selectedNode.centerY : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        showAutomation: false
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerY = newValue }
                    }
                }
            }

            // Border (slope width + bend asymmetry)
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Border")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Border"
                        paramCount: 4
                        trackColor: "#20B2AA"

                        ParameterRow {
                            label: qsTr("H Border")
                            value: root.selectedNode ? root.selectedNode.horizontalBorder : 1.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            showAutomation: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.horizontalBorder = newValue }
                        }

                        ParameterRow {
                            label: qsTr("H Bend")
                            value: root.selectedNode ? root.selectedNode.horizontalBend : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            showAutomation: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.horizontalBend = newValue }
                        }

                        ParameterRow {
                            label: qsTr("V Border")
                            value: root.selectedNode ? root.selectedNode.verticalBorder : 1.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            showAutomation: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.verticalBorder = newValue }
                        }

                        ParameterRow {
                            label: qsTr("V Bend")
                            value: root.selectedNode ? root.selectedNode.verticalBend : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            showAutomation: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.verticalBend = newValue }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            text: qsTr("Curve:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 45
                        }

                        // Curve icon buttons
                        Repeater {
                            model: 10
                            CurveIconButton {
                                curveType: index
                                selected: root.selectedNode && root.selectedNode.falloffCurve === index
                                onClicked: if (root.selectedNode) root.selectedNode.falloffCurve = index
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }

            // Noise
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Noise")

                ParameterGroup {
                    anchors.fill: parent
                    node: root.selectedNode
                    trackName: "Noise"
                    paramCount: 3
                    trackColor: "#808000"

                    ParameterRow {
                        label: qsTr("Intensity")
                        value: root.selectedNode ? root.selectedNode.noiseIntensity : 0
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        showAutomation: false
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.noiseIntensity = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Scale")
                        value: root.selectedNode ? root.selectedNode.noiseScale : 1.0
                        minValue: 0.01
                        maxValue: 2.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        showAutomation: false
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.noiseScale = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Speed")
                        value: root.selectedNode ? root.selectedNode.noiseSpeed : 0
                        minValue: 0
                        maxValue: 10.0
                        defaultValue: 0
                        stepSize: 0.1
                        suffix: ""
                        showAutomation: false
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.noiseSpeed = newValue }
                    }
                }
            }

            // Shape-specific: Angle
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Angle Shape")
                visible: root.selectedNode && root.selectedNode.shape === GizmoNode.Shape.Angle


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Aperture"
                        paramCount: 1
                        trackColor: "#FF6347"

                        ParameterRow {
                            label: qsTr("Aperture")
                            value: root.selectedNode ? root.selectedNode.aperture : 90
                            minValue: 0
                            maxValue: 360
                            defaultValue: 90
                            stepSize: 1
                            suffix: "°"
                            showAutomation: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.aperture = newValue }
                        }
                    }

                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Phase"
                        paramCount: 1
                        trackColor: "#1E90FF"

                        ParameterRow {
                            label: qsTr("Phase")
                            value: root.selectedNode ? root.selectedNode.phase : 0
                            minValue: 0
                            maxValue: 360
                            defaultValue: 0
                            stepSize: 1
                            suffix: "°"
                            showAutomation: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.phase = newValue }
                        }
                    }
                }
            }

            // Shape-specific: Waves
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Wave Shape")
                visible: root.selectedNode && (root.selectedNode.shape === GizmoNode.Shape.LinearWave || root.selectedNode.shape === GizmoNode.Shape.CircularWave)


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "WaveCount"
                        paramCount: 1
                        trackColor: "#8A2BE2"

                        ParameterRow {
                            label: qsTr("Wave Count")
                            value: root.selectedNode ? root.selectedNode.waveCount : 4
                            minValue: 1
                            maxValue: 20
                            defaultValue: 4
                            stepSize: 1
                            showAutomation: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.waveCount = newValue }
                        }
                    }

                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Phase"
                        paramCount: 1
                        trackColor: "#1E90FF"

                        ParameterRow {
                            label: qsTr("Phase")
                            value: root.selectedNode ? root.selectedNode.phase : 0
                            minValue: 0
                            maxValue: 360
                            defaultValue: 0
                            stepSize: 1
                            suffix: "°"
                            showAutomation: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.phase = newValue }
                        }
                    }
                }
            }
        }
    }

    // Transform properties (was Group)
    Component {
        id: transformProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Transform Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Single input mode checkbox
                    StyledCheckBox {
                        text: qsTr("Single Input Mode")
                        checked: root.selectedNode ? root.selectedNode.singleInputMode : false
                        onCheckedChanged: if (root.selectedNode) root.selectedNode.singleInputMode = checked
                        font.pixelSize: Theme.propFontSize

                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Use only one input (no combination)")
                        ToolTip.delay: 500
                    }

                    // Composition mode selector (hidden in single input mode)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: root.selectedNode ? !root.selectedNode.singleInputMode : true

                        Label {
                            text: qsTr("Mode:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 55
                        }

                        StyledComboBox {
                            model: [qsTr("Normal"), qsTr("Max"), qsTr("Min"), qsTr("Sum"), qsTr("AbsDiff"), qsTr("Diff"), qsTr("Product")]
                            currentIndex: root.selectedNode ? root.selectedNode.compositionMode : 0
                            onActivated: if (root.selectedNode) root.selectedNode.compositionMode = currentIndex
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // Position track (2 params: positionX, positionY)
            ParameterGroup {
                Layout.fillWidth: true
                node: root.selectedNode
                trackName: "Position"
                paramCount: 2
                trackColor: "#4682B4"  // Steel blue

                ParameterRow {
                    label: qsTr("Position X")
                    value: root.selectedNode ? root.selectedNode.positionX : 0
                    minValue: -2.0
                    maxValue: 2.0
                    defaultValue: 0
                    stepSize: 0.01
                    displayRatio: 100
                    suffix: "%"
                    showAutomation: false
                    onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.positionX = newValue }
                }

                ParameterRow {
                    label: qsTr("Position Y")
                    value: root.selectedNode ? root.selectedNode.positionY : 0
                    minValue: -2.0
                    maxValue: 2.0
                    defaultValue: 0
                    stepSize: 0.01
                    displayRatio: 100
                    suffix: "%"
                    showAutomation: false
                    onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.positionY = newValue }
                }
            }

            // Scale track (2 params: scaleX, scaleY)
            ParameterGroup {
                Layout.fillWidth: true
                node: root.selectedNode
                trackName: "Scale"
                paramCount: 2
                trackColor: "#3CB371"  // Medium sea green

                ParameterRow {
                    label: qsTr("Scale X")
                    value: root.selectedNode ? root.selectedNode.scaleX : 1.0
                    minValue: 0.01
                    maxValue: 10.0
                    defaultValue: 1.0
                    stepSize: 0.01
                    displayRatio: 100
                    suffix: "%"
                    showAutomation: false
                    onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.scaleX = newValue }
                }

                ParameterRow {
                    label: qsTr("Scale Y")
                    value: root.selectedNode ? root.selectedNode.scaleY : 1.0
                    minValue: 0.01
                    maxValue: 10.0
                    defaultValue: 1.0
                    stepSize: 0.01
                    displayRatio: 100
                    suffix: "%"
                    showAutomation: false
                    onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.scaleY = newValue }
                }
            }

            // Rotation track (1 param: rotation)
            ParameterGroup {
                Layout.fillWidth: true
                node: root.selectedNode
                trackName: "Rotation"
                paramCount: 1
                trackColor: "#FF8C00"  // Dark orange

                ParameterRow {
                    label: qsTr("Rotation")
                    value: root.selectedNode ? root.selectedNode.rotation : 0
                    minValue: -360
                    maxValue: 360
                    defaultValue: 0
                    stepSize: 1
                    displayRatio: 1
                    suffix: "°"
                    showAutomation: false
                    onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.rotation = newValue }
                }
            }
        }
    }

    // TimeShift properties
    Component {
        id: timeShiftProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("TimeShift Settings")


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("Delay")
                        value: root.selectedNode ? root.selectedNode.delay : 0
                        minValue: -10.0
                        maxValue: 10.0
                        defaultValue: 0
                        stepSize: 0.001
                        decimals: 3
                        displayRatio: 1000
                        suffix: " ms"
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.delay = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Scale")
                        value: root.selectedNode ? root.selectedNode.scale : 1.0
                        minValue: 0.01
                        maxValue: 10.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.scale = newValue }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Loop:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            id: loopCheck
                            checked: root.selectedNode ? root.selectedNode.loop : false
                            onToggled: if (root.selectedNode) root.selectedNode.loop = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    ParameterRow {
                        visible: loopCheck.checked
                        label: qsTr("Duration")
                        value: root.selectedNode ? root.selectedNode.loopDuration : 1.0
                        minValue: 0.001
                        maxValue: 60.0
                        defaultValue: 1.0
                        stepSize: 0.001
                        decimals: 3
                        displayRatio: 1000
                        suffix: " ms"
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.loopDuration = newValue }
                    }
                }
            }
        }
    }

    // SurfaceFactory properties
    Component {
        id: surfaceFactoryProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Surface Settings")


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Type selector
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Type:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        StyledComboBox {
                            model: [qsTr("Linear"), qsTr("Sine"), qsTr("Cosine"), qsTr("Triangle"), qsTr("Sawtooth"), qsTr("Square"), qsTr("Exponential"), qsTr("Logarithmic")]
                            currentIndex: root.selectedNode ? root.selectedNode.surfaceType : 0
                            onActivated: if (root.selectedNode) root.selectedNode.surfaceType = currentIndex
                            Layout.fillWidth: true
                        }
                    }

                    ParameterRow {
                        label: qsTr("Amplitude")
                        value: root.selectedNode ? root.selectedNode.amplitude : 1.0
                        minValue: 0
                        maxValue: 2.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.amplitude = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Frequency")
                        value: root.selectedNode ? root.selectedNode.frequency : 1.0
                        minValue: 0.01
                        maxValue: 10.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.frequency = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Phase")
                        value: root.selectedNode ? root.selectedNode.phase : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.phase = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Offset")
                        value: root.selectedNode ? root.selectedNode.offset : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.offset = newValue }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Clamp:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.clamp : true
                            onToggled: if (root.selectedNode) root.selectedNode.clamp = checked
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    // Mirror properties
    Component {
        id: mirrorProperties
        StyledGroupBox {
            title: qsTr("Mirror Settings")

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                // Axis selection with icon buttons
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Label {
                        text: qsTr("Axis:")
                        color: Theme.propLabel
                        font.pixelSize: Theme.propFontSize
                    }

                    Item { Layout.fillWidth: true }

                    // Horizontal axis button (vertical line |)
                    Rectangle {
                        id: btnHorizontal
                        property bool isSelected: root.selectedNode && root.selectedNode.axis === 0
                        property bool isHovered: false
                        width: 32; height: 32
                        radius: 4
                        color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.surfaceHover || "#404040") : (Theme.surface || "#303030"))
                        border.color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.accent || "#4060A0") : (Theme.border || "#555555"))
                        border.width: 1

                        Canvas {
                            id: canvasHorizontal
                            anchors.fill: parent
                            anchors.margins: 6
                            property bool sel: btnHorizontal.isSelected
                            property bool hov: btnHorizontal.isHovered
                            onSelChanged: requestPaint()
                            onHovChanged: requestPaint()
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.strokeStyle = sel ? "#ffffff" : (hov ? Theme.propLabel : Theme.textMuted)
                                ctx.lineWidth = 2
                                ctx.beginPath()
                                ctx.moveTo(width / 2, 0)
                                ctx.lineTo(width / 2, height)
                                ctx.stroke()
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: if (root.selectedNode) root.selectedNode.axis = 0
                            onContainsMouseChanged: btnHorizontal.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("Horizontal")
                    }

                    // Vertical axis button (horizontal line ―)
                    Rectangle {
                        id: btnVertical
                        property bool isSelected: root.selectedNode && root.selectedNode.axis === 1
                        property bool isHovered: false
                        width: 32; height: 32
                        radius: 4
                        color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.surfaceHover || "#404040") : (Theme.surface || "#303030"))
                        border.color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.accent || "#4060A0") : (Theme.border || "#555555"))
                        border.width: 1

                        Canvas {
                            anchors.fill: parent
                            anchors.margins: 6
                            property bool sel: btnVertical.isSelected
                            property bool hov: btnVertical.isHovered
                            onSelChanged: requestPaint()
                            onHovChanged: requestPaint()
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.strokeStyle = sel ? "#ffffff" : (hov ? Theme.propLabel : Theme.textMuted)
                                ctx.lineWidth = 2
                                ctx.beginPath()
                                ctx.moveTo(0, height / 2)
                                ctx.lineTo(width, height / 2)
                                ctx.stroke()
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: if (root.selectedNode) root.selectedNode.axis = 1
                            onContainsMouseChanged: btnVertical.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("Vertical")
                    }

                    // +45° axis button (diagonal /)
                    Rectangle {
                        id: btnDiag45
                        property bool isSelected: root.selectedNode && root.selectedNode.axis === 2
                        property bool isHovered: false
                        width: 32; height: 32
                        radius: 4
                        color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.surfaceHover || "#404040") : (Theme.surface || "#303030"))
                        border.color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.accent || "#4060A0") : (Theme.border || "#555555"))
                        border.width: 1

                        Canvas {
                            anchors.fill: parent
                            anchors.margins: 6
                            property bool sel: btnDiag45.isSelected
                            property bool hov: btnDiag45.isHovered
                            onSelChanged: requestPaint()
                            onHovChanged: requestPaint()
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.strokeStyle = sel ? "#ffffff" : (hov ? Theme.propLabel : Theme.textMuted)
                                ctx.lineWidth = 2
                                ctx.beginPath()
                                ctx.moveTo(0, height)
                                ctx.lineTo(width, 0)
                                ctx.stroke()
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: if (root.selectedNode) root.selectedNode.axis = 2
                            onContainsMouseChanged: btnDiag45.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("+45°")
                    }

                    // -45° axis button (diagonal \)
                    Rectangle {
                        id: btnDiagMinus45
                        property bool isSelected: root.selectedNode && root.selectedNode.axis === 3
                        property bool isHovered: false
                        width: 32; height: 32
                        radius: 4
                        color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.surfaceHover || "#404040") : (Theme.surface || "#303030"))
                        border.color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.accent || "#4060A0") : (Theme.border || "#555555"))
                        border.width: 1

                        Canvas {
                            anchors.fill: parent
                            anchors.margins: 6
                            property bool sel: btnDiagMinus45.isSelected
                            property bool hov: btnDiagMinus45.isHovered
                            onSelChanged: requestPaint()
                            onHovChanged: requestPaint()
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.strokeStyle = sel ? "#ffffff" : (hov ? Theme.propLabel : Theme.textMuted)
                                ctx.lineWidth = 2
                                ctx.beginPath()
                                ctx.moveTo(0, 0)
                                ctx.lineTo(width, height)
                                ctx.stroke()
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: if (root.selectedNode) root.selectedNode.axis = 3
                            onContainsMouseChanged: btnDiagMinus45.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("-45°")
                    }

                    // Custom angle button (angle icon)
                    Rectangle {
                        id: btnCustom
                        property bool isSelected: root.selectedNode && root.selectedNode.axis === 4
                        property bool isHovered: false
                        width: 32; height: 32
                        radius: 4
                        color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.surfaceHover || "#404040") : (Theme.surface || "#303030"))
                        border.color: isSelected ? (Theme.accent || "#4060A0") : (isHovered ? (Theme.accent || "#4060A0") : (Theme.border || "#555555"))
                        border.width: 1

                        Canvas {
                            anchors.fill: parent
                            anchors.margins: 6
                            property bool sel: btnCustom.isSelected
                            property bool hov: btnCustom.isHovered
                            onSelChanged: requestPaint()
                            onHovChanged: requestPaint()
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.strokeStyle = sel ? "#ffffff" : (hov ? Theme.propLabel : Theme.textMuted)
                                ctx.lineWidth = 2
                                // Draw angle arc symbol
                                ctx.beginPath()
                                ctx.moveTo(0, height)
                                ctx.lineTo(width, height)
                                ctx.stroke()
                                ctx.beginPath()
                                ctx.moveTo(0, height)
                                ctx.lineTo(width * 0.7, 0)
                                ctx.stroke()
                                // Small arc
                                ctx.beginPath()
                                ctx.arc(0, height, width * 0.4, -Math.PI/2, 0, false)
                                ctx.stroke()
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: if (root.selectedNode) root.selectedNode.axis = 4
                            onContainsMouseChanged: btnCustom.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("Custom Angle")
                    }
                }

                // Custom angle spinbox (only visible when Custom is selected)
                RowLayout {
                    Layout.fillWidth: true
                    visible: root.selectedNode && root.selectedNode.axis === 4
                    spacing: 8

                    Label {
                        text: qsTr("Angle:")
                        color: Theme.propLabel
                        font.pixelSize: Theme.propFontSize
                    }
                    StyledSpinBox {
                        id: customAngleSpinBox
                        Layout.fillWidth: true
                        from: -180; to: 180
                        Binding on value {
                            value: root.selectedNode ? Math.round(root.selectedNode.customAngle) : 0
                            when: !customAngleSpinBox.activeFocus
                        }
                        onValueModified: if (root.selectedNode) root.selectedNode.customAngle = value
                    }
                }
            }
        }
    }

    // Position Tweak properties
    Component {
        id: positionTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Position Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Position track (2 params: offsetX, offsetY)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Position"
                        paramCount: 2
                        trackColor: "#4682B4"  // Steel blue

                        ParameterRow {
                            label: qsTr("X")
                            value: root.selectedNode ? root.selectedNode.offsetX : 0
                            minValue: -2.0
                            maxValue: 2.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.offsetX = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Y")
                            value: root.selectedNode ? root.selectedNode.offsetY : 0
                            minValue: -2.0
                            maxValue: 2.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.offsetY = newValue }
                        }
                    }
                }
            }
        }
    }

    // Scale Tweak properties
    Component {
        id: scaleTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Scale Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Scale track (2 params: scaleX, scaleY)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Scale"
                        paramCount: 2
                        trackColor: "#3CB371"  // Medium sea green

                        ParameterRow {
                            label: qsTr("Scale X")
                            value: root.selectedNode ? root.selectedNode.scaleX : 1.0
                            minValue: 0.01
                            maxValue: 5.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.scaleX = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Scale Y")
                            value: root.selectedNode ? root.selectedNode.scaleY : 1.0
                            minValue: 0.01
                            maxValue: 5.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.scaleY = newValue }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Uniform:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.uniform : true
                            onToggled: if (root.selectedNode) root.selectedNode.uniform = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    // Center track (2 params: centerX, centerY)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Center"
                        paramCount: 2
                        trackColor: "#BA55D3"  // Medium orchid

                        ParameterRow {
                            label: qsTr("Center X")
                            value: root.selectedNode ? root.selectedNode.centerX : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerX = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Center Y")
                            value: root.selectedNode ? root.selectedNode.centerY : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerY = newValue }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("CrossOver:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.crossOver : false
                            onToggled: if (root.selectedNode) root.selectedNode.crossOver = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Follow Gizmo:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.followGizmo : false
                            onToggled: if (root.selectedNode) root.selectedNode.followGizmo = checked
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    // Rotation Tweak properties
    Component {
        id: rotationTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Rotation Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Rotation track (1 param: angle)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Rotation"
                        paramCount: 1
                        trackColor: "#FF8C00"  // Dark orange

                        ParameterRow {
                            label: qsTr("Angle")
                            value: root.selectedNode ? root.selectedNode.angle : 0
                            minValue: -360
                            maxValue: 360
                            defaultValue: 0
                            stepSize: 1
                            suffix: "°"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.angle = newValue }
                        }
                    }

                    // Center track (2 params: centerX, centerY)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Center"
                        paramCount: 2
                        trackColor: "#BA55D3"  // Medium orchid

                        ParameterRow {
                            label: qsTr("Center X")
                            value: root.selectedNode ? root.selectedNode.centerX : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerX = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Center Y")
                            value: root.selectedNode ? root.selectedNode.centerY : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerY = newValue }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Follow Gizmo:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.followGizmo : false
                            onToggled: if (root.selectedNode) root.selectedNode.followGizmo = checked
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    // Color Tweak properties
    Component {
        id: colorTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Color Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Color track (4 params: R, G, B, Alpha)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Color"
                        paramCount: 4
                        trackColor: "#DC143C"  // Crimson

                        ParameterRow {
                            label: qsTr("Red")
                            value: root.selectedNode ? root.selectedNode.color.r : 1.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) {
                                if (root.selectedNode) {
                                    var c = root.selectedNode.color
                                    root.selectedNode.color = Qt.rgba(newValue, c.g, c.b, c.a)
                                }
                            }
                        }

                        ParameterRow {
                            label: qsTr("Green")
                            value: root.selectedNode ? root.selectedNode.color.g : 1.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) {
                                if (root.selectedNode) {
                                    var c = root.selectedNode.color
                                    root.selectedNode.color = Qt.rgba(c.r, newValue, c.b, c.a)
                                }
                            }
                        }

                        ParameterRow {
                            label: qsTr("Blue")
                            value: root.selectedNode ? root.selectedNode.color.b : 1.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) {
                                if (root.selectedNode) {
                                    var c = root.selectedNode.color
                                    root.selectedNode.color = Qt.rgba(c.r, c.g, newValue, c.a)
                                }
                            }
                        }

                        ParameterRow {
                            label: qsTr("Alpha")
                            value: root.selectedNode ? root.selectedNode.alpha : 0.0
                            minValue: -2.0
                            maxValue: 2.0
                            defaultValue: 0.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.alpha = newValue }
                        }
                    }
                }
            }

            // Filter GroupBox
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Filter (apply only to)")
                showAutomation: true
                node: root.selectedNode
                trackName: "Filter"
                paramCount: 6
                trackColor: "#6495ED"  // Cornflower blue


                GridLayout {
                    anchors.fill: parent
                    columns: 5
                    rowSpacing: 4
                    columnSpacing: 4

                    // Header
                    Item { Layout.preferredWidth: 30 }
                    Label { text: qsTr("Min"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                    Item { Layout.preferredWidth: 10 }
                    Label { text: qsTr("Max"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                    Item { Layout.preferredWidth: 4 }

                    // Red
                    Label { text: "R"; color: "#FF6666"; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 30 }
                    StyledSpinBox {
                        id: filterRedMinSpinBox
                        from: 0; to: 100
                        Binding on value {
                            value: root.selectedNode ? Math.round(root.selectedNode.filterRedMin * 100) : 0
                            when: !filterRedMinSpinBox.activeFocus
                        }
                        onValueModified: if (root.selectedNode) root.selectedNode.filterRedMin = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "-"; color: Theme.propLabel; horizontalAlignment: Text.AlignHCenter }
                    StyledSpinBox {
                        id: filterRedMaxSpinBox
                        from: 0; to: 100
                        Binding on value {
                            value: root.selectedNode ? Math.round(root.selectedNode.filterRedMax * 100) : 100
                            when: !filterRedMaxSpinBox.activeFocus
                        }
                        onValueModified: if (root.selectedNode) root.selectedNode.filterRedMax = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "%"; color: Theme.propLabel; font.pixelSize: Theme.propFontSize }

                    // Green
                    Label { text: "G"; color: "#66FF66"; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 30 }
                    StyledSpinBox {
                        id: filterGreenMinSpinBox
                        from: 0; to: 100
                        Binding on value {
                            value: root.selectedNode ? Math.round(root.selectedNode.filterGreenMin * 100) : 0
                            when: !filterGreenMinSpinBox.activeFocus
                        }
                        onValueModified: if (root.selectedNode) root.selectedNode.filterGreenMin = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "-"; color: Theme.propLabel; horizontalAlignment: Text.AlignHCenter }
                    StyledSpinBox {
                        id: filterGreenMaxSpinBox
                        from: 0; to: 100
                        Binding on value {
                            value: root.selectedNode ? Math.round(root.selectedNode.filterGreenMax * 100) : 100
                            when: !filterGreenMaxSpinBox.activeFocus
                        }
                        onValueModified: if (root.selectedNode) root.selectedNode.filterGreenMax = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "%"; color: Theme.propLabel; font.pixelSize: Theme.propFontSize }

                    // Blue
                    Label { text: "B"; color: "#6666FF"; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 30 }
                    StyledSpinBox {
                        id: filterBlueMinSpinBox
                        from: 0; to: 100
                        Binding on value {
                            value: root.selectedNode ? Math.round(root.selectedNode.filterBlueMin * 100) : 0
                            when: !filterBlueMinSpinBox.activeFocus
                        }
                        onValueModified: if (root.selectedNode) root.selectedNode.filterBlueMin = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "-"; color: Theme.propLabel; horizontalAlignment: Text.AlignHCenter }
                    StyledSpinBox {
                        id: filterBlueMaxSpinBox
                        from: 0; to: 100
                        Binding on value {
                            value: root.selectedNode ? Math.round(root.selectedNode.filterBlueMax * 100) : 100
                            when: !filterBlueMaxSpinBox.activeFocus
                        }
                        onValueModified: if (root.selectedNode) root.selectedNode.filterBlueMax = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "%"; color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                }
            }

            // Follow Gizmo
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    text: qsTr("Follow Gizmo:")
                    color: Theme.propLabel
                    font.pixelSize: Theme.propFontSize
                    Layout.preferredWidth: 70
                }

                CheckBox {
                    checked: root.selectedNode ? root.selectedNode.followGizmo : true
                    onToggled: if (root.selectedNode) root.selectedNode.followGizmo = checked
                }

                Item { Layout.fillWidth: true }
            }
        }
    }

    // Polar Tweak properties
    Component {
        id: polarTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Polar Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Expansion track (2 params: expansion, ringRadius) - matches GizmoTweak v1
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Expansion"
                        paramCount: 2
                        trackColor: "#FF7F50"  // Coral

                        ParameterRow {
                            label: qsTr("Expansion")
                            value: root.selectedNode ? root.selectedNode.expansion : 0
                            minValue: -2.0
                            maxValue: 2.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.expansion = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Radius")
                            value: root.selectedNode ? root.selectedNode.ringRadius : 0.5
                            minValue: 0.01
                            maxValue: 2.0
                            defaultValue: 0.5
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.ringRadius = newValue }
                        }
                    }

                    // RingScale track (1 param: ringScale) - matches GizmoTweak v1
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "RingScale"
                        paramCount: 1
                        trackColor: "#8A2BE2"  // Blue violet

                        ParameterRow {
                            label: qsTr("Ring Scale")
                            value: root.selectedNode ? root.selectedNode.ringScale : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.ringScale = newValue }
                        }
                    }

                    // Center parameters (not automated in GizmoTweak v1)
                    ParameterRow {
                        label: qsTr("Center X")
                        value: root.selectedNode ? root.selectedNode.centerX : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        showAutomation: false
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center Y")
                        value: root.selectedNode ? root.selectedNode.centerY : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        showAutomation: false
                        onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerY = newValue }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("CrossOver:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.crossOver : false
                            onToggled: if (root.selectedNode) root.selectedNode.crossOver = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Targetted:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.targetted : false
                            onToggled: if (root.selectedNode) root.selectedNode.targetted = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Follow Gizmo:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.followGizmo : false
                            onToggled: if (root.selectedNode) root.selectedNode.followGizmo = checked
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    // Wave Tweak properties
    Component {
        id: waveTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Wave Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Wave track (4 params: amplitude, wavelength, phase, angle)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Wave"
                        paramCount: 4
                        trackColor: "#1E90FF"  // Dodger blue

                        ParameterRow {
                            label: qsTr("Amplitude")
                            value: root.selectedNode ? root.selectedNode.amplitude : 0.1
                            minValue: 0
                            maxValue: 2.0
                            defaultValue: 0.1
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.amplitude = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Wavelength")
                            value: root.selectedNode ? root.selectedNode.wavelength : 0.5
                            minValue: 0.01
                            maxValue: 2.0
                            defaultValue: 0.5
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.wavelength = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Phase")
                            value: root.selectedNode ? root.selectedNode.phase : 0
                            minValue: 0
                            maxValue: 360
                            defaultValue: 0
                            stepSize: 1
                            suffix: "°"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.phase = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Angle")
                            value: root.selectedNode ? root.selectedNode.angle : 0
                            minValue: 0
                            maxValue: 360
                            defaultValue: 0
                            stepSize: 1
                            suffix: "°"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.angle = newValue }
                        }
                    }

                    // Center track (2 params: centerX, centerY)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Center"
                        paramCount: 2
                        trackColor: "#BA55D3"  // Medium orchid

                        ParameterRow {
                            label: qsTr("Center X")
                            value: root.selectedNode ? root.selectedNode.centerX : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerX = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Center Y")
                            value: root.selectedNode ? root.selectedNode.centerY : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerY = newValue }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Radial Mode:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.radial : true
                            onToggled: if (root.selectedNode) root.selectedNode.radial = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Follow Gizmo:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.followGizmo : false
                            onToggled: if (root.selectedNode) root.selectedNode.followGizmo = checked
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    // Squeeze Tweak properties
    Component {
        id: squeezeTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Squeeze Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Squeeze track (2 params: intensity, angle)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Squeeze"
                        paramCount: 2
                        trackColor: "#D2691E"  // Chocolate

                        ParameterRow {
                            label: qsTr("Intensity")
                            value: root.selectedNode ? root.selectedNode.intensity : 0.5
                            minValue: -2.0
                            maxValue: 2.0
                            defaultValue: 0.5
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.intensity = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Angle")
                            value: root.selectedNode ? root.selectedNode.angle : 0
                            minValue: 0
                            maxValue: 360
                            defaultValue: 0
                            stepSize: 1
                            suffix: "°"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.angle = newValue }
                        }
                    }

                    // Center track (2 params: centerX, centerY)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Center"
                        paramCount: 2
                        trackColor: "#BA55D3"  // Medium orchid

                        ParameterRow {
                            label: qsTr("Center X")
                            value: root.selectedNode ? root.selectedNode.centerX : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerX = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Center Y")
                            value: root.selectedNode ? root.selectedNode.centerY : 0
                            minValue: -1.0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.centerY = newValue }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Follow Gizmo:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.followGizmo : false
                            onToggled: if (root.selectedNode) root.selectedNode.followGizmo = checked
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    // Sparkle Tweak properties
    Component {
        id: sparkleTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Sparkle Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Sparkle track (5 params: density, red, green, blue, alpha) - matches GizmoTweak v1
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Sparkle"
                        paramCount: 5
                        trackColor: "#FFD700"  // Gold

                        ParameterRow {
                            label: qsTr("Density")
                            value: root.selectedNode ? root.selectedNode.density : 0.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 0.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.density = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Red")
                            value: root.selectedNode ? root.selectedNode.red : 1.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.red = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Green")
                            value: root.selectedNode ? root.selectedNode.green : 1.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.green = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Blue")
                            value: root.selectedNode ? root.selectedNode.blue : 1.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.blue = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Alpha")
                            value: root.selectedNode ? root.selectedNode.alpha : 1.0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.alpha = newValue }
                        }
                    }

                    // Color picker (convenience)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Color:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        Rectangle {
                            width: 60
                            height: 24
                            radius: 4
                            color: root.selectedNode ? Qt.rgba(root.selectedNode.red, root.selectedNode.green, root.selectedNode.blue, 1.0) : "#FFFFFF"
                            border.color: Theme.border

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (root.selectedNode) {
                                        root.colorDialog.selectedColor = Qt.rgba(root.selectedNode.red, root.selectedNode.green, root.selectedNode.blue, 1.0)
                                        root.colorDialogCallback = function(color) {
                                            root.selectedNode.red = color.r
                                            root.selectedNode.green = color.g
                                            root.selectedNode.blue = color.b
                                        }
                                        root.colorDialog.open()
                                    }
                                }
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Follow Gizmo:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            checked: root.selectedNode ? root.selectedNode.followGizmo : true
                            onToggled: if (root.selectedNode) root.selectedNode.followGizmo = checked
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    // Fuzzyness Tweak properties
    Component {
        id: fuzzynessTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Fuzzyness Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Amount track (1 param: amount)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Amount"
                        paramCount: 1
                        trackColor: "#FFB6C1"  // Light pink

                        ParameterRow {
                            label: qsTr("Amount")
                            value: root.selectedNode ? root.selectedNode.amount : 0.1
                            minValue: 0
                            maxValue: 2.0
                            defaultValue: 0.1
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.amount = newValue }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Affect:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            text: "X"
                            checked: root.selectedNode ? root.selectedNode.affectX : true
                            onToggled: if (root.selectedNode) root.selectedNode.affectX = checked
                        }

                        CheckBox {
                            text: "Y"
                            checked: root.selectedNode ? root.selectedNode.affectY : true
                            onToggled: if (root.selectedNode) root.selectedNode.affectY = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Use Seed:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            id: useSeedCheck
                            checked: root.selectedNode ? root.selectedNode.useSeed : false
                            onToggled: if (root.selectedNode) root.selectedNode.useSeed = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        visible: useSeedCheck.checked
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Seed:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        StyledSpinBox {
                            id: sparkleSeedSpinBox
                            from: 0
                            to: 999999
                            Binding on value {
                                value: root.selectedNode ? root.selectedNode.seed : 0
                                when: !sparkleSeedSpinBox.activeFocus
                            }
                            onValueModified: if (root.selectedNode) root.selectedNode.seed = value
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }

    // Color Fuzzyness Tweak properties
    Component {
        id: colorFuzzynessTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Color Fuzzyness Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Amount track (1 param: amount)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Amount"
                        paramCount: 1
                        trackColor: "#FFB6C1"  // Light pink

                        ParameterRow {
                            label: qsTr("Amount")
                            value: root.selectedNode ? root.selectedNode.amount : 0.1
                            minValue: 0
                            maxValue: 2.0
                            defaultValue: 0.1
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.amount = newValue }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Affect:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            text: "R"
                            checked: root.selectedNode ? root.selectedNode.affectRed : true
                            onToggled: if (root.selectedNode) root.selectedNode.affectRed = checked
                        }

                        CheckBox {
                            text: "G"
                            checked: root.selectedNode ? root.selectedNode.affectGreen : true
                            onToggled: if (root.selectedNode) root.selectedNode.affectGreen = checked
                        }

                        CheckBox {
                            text: "B"
                            checked: root.selectedNode ? root.selectedNode.affectBlue : true
                            onToggled: if (root.selectedNode) root.selectedNode.affectBlue = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Use Seed:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        CheckBox {
                            id: colorUseSeedCheck
                            checked: root.selectedNode ? root.selectedNode.useSeed : false
                            onToggled: if (root.selectedNode) root.selectedNode.useSeed = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        visible: colorUseSeedCheck.checked
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Seed:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        StyledSpinBox {
                            id: colorFuzzySeedSpinBox
                            from: 0
                            to: 999999
                            Binding on value {
                                value: root.selectedNode ? root.selectedNode.seed : 0
                                when: !colorFuzzySeedSpinBox.activeFocus
                            }
                            onValueModified: if (root.selectedNode) root.selectedNode.seed = value
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }

    // Split Tweak properties
    Component {
        id: splitTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Split Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Threshold track (1 param: splitThreshold)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Threshold"
                        paramCount: 1
                        trackColor: "#F4A460"  // Sandy brown

                        ParameterRow {
                            label: qsTr("Threshold")
                            value: root.selectedNode ? root.selectedNode.splitThreshold : 0.5
                            minValue: 0.001
                            maxValue: 4.0
                            defaultValue: 0.5
                            stepSize: 0.01
                            decimals: 3
                            suffix: ""
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.splitThreshold = newValue }
                        }
                    }

                    Label {
                        text: qsTr("Distance in normalized coords.\nLower = more splits (explosion effect)")
                        color: Theme.propLabel
                        font.pixelSize: Theme.propFontSize - 1
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }

    // Rounder Tweak properties
    Component {
        id: rounderTweakProperties
        ColumnLayout {
            spacing: 8

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Rounder Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    // Rounder track (6 params: amount, vShift, hShift, tighten, radialResize, radialShift)
                    ParameterGroup {
                        Layout.fillWidth: true
                        node: root.selectedNode
                        trackName: "Rounder"
                        paramCount: 6
                        trackColor: "#40E0D0"  // Turquoise

                        ParameterRow {
                            label: qsTr("Amount")
                            value: root.selectedNode ? root.selectedNode.amount : 0
                            minValue: -2.0
                            maxValue: 2.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.amount = newValue }
                        }

                        ParameterRow {
                            label: qsTr("V Shift")
                            value: root.selectedNode ? root.selectedNode.verticalShift : 0
                            minValue: -2.0
                            maxValue: 2.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.verticalShift = newValue }
                        }

                        ParameterRow {
                            label: qsTr("H Shift")
                            value: root.selectedNode ? root.selectedNode.horizontalShift : 0
                            minValue: -2.0
                            maxValue: 2.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.horizontalShift = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Tighten")
                            value: root.selectedNode ? root.selectedNode.tighten : 0
                            minValue: 0
                            maxValue: 1.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.tighten = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Radial Resize")
                            value: root.selectedNode ? root.selectedNode.radialResize : 1.0
                            minValue: 0.5
                            maxValue: 2.0
                            defaultValue: 1.0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.radialResize = newValue }
                        }

                        ParameterRow {
                            label: qsTr("Radial Shift")
                            value: root.selectedNode ? root.selectedNode.radialShift : 0
                            minValue: -2.0
                            maxValue: 2.0
                            defaultValue: 0
                            stepSize: 0.01
                            displayRatio: 100
                            suffix: "%"
                            reserveAutomationSpace: false
                            onValueModified: function(newValue) { if (root.selectedNode) root.selectedNode.radialShift = newValue }
                        }
                    }

                    Label {
                        text: qsTr("Cylindrical distortion effect.\nWraps the frame around a cylinder.")
                        color: Theme.propLabel
                        font.pixelSize: Theme.propFontSize - 1
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
