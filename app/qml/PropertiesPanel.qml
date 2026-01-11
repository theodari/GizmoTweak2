import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import GizmoTweakLib2
import GizmoTweak2

Drawer {
    id: root

    property var selectedNode: null
    property alias colorDialog: colorDialog
    property var colorDialogCallback: null

    edge: Qt.LeftEdge
    width: 380
    height: parent.height

    // Only close when explicitly requested
    interactive: false
    modal: false
    dim: false

    // Fast animations
    enter: Transition {
        NumberAnimation { property: "position"; to: 1.0; duration: 120; easing.type: Easing.OutCubic }
    }
    exit: Transition {
        NumberAnimation { property: "position"; to: 0.0; duration: 100; easing.type: Easing.InCubic }
    }

    background: Rectangle {
        color: Theme.surface
        border.color: Theme.border
        border.width: 1
    }

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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 6

        // Header with title and close button
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: selectedNode ? selectedNode.type : qsTr("Properties")
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
                    onClicked: root.close()
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

                // Common properties
                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTr("General")

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
                            text: selectedNode ? selectedNode.displayName : ""
                            color: Theme.text
                            font.pixelSize: Theme.propFontSize
                            implicitHeight: Theme.propSpinBoxHeight
                            background: Rectangle {
                                color: Theme.background
                                border.color: nameField.activeFocus ? Theme.accent : Theme.border
                                radius: 4
                            }
                            onEditingFinished: {
                                if (selectedNode) {
                                    selectedNode.displayName = text
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
                        if (!selectedNode) return null
                        switch (selectedNode.type) {
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

                            property var currentFrame: selectedNode && selectedNode.currentFrame ? selectedNode.currentFrame : null
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

                            contentItem: Text {
                                text: inputPlayButton.playing ? "\u25A0" : "\u25B6"
                                color: inputPlayButton.playing ? Theme.accent : Theme.text
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
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
                            currentIndex: selectedNode ? selectedNode.sourceType : 0
                            onActivated: if (selectedNode) selectedNode.sourceType = currentIndex
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
                                model: selectedNode ? selectedNode.patternNames : []

                                Rectangle {
                                    width: 70
                                    height: 70
                                    color: (selectedNode && selectedNode.patternIndex === index) ? Theme.accent : Theme.surface
                                    border.color: patternMouseArea.containsMouse ? Theme.textHighlight : Theme.border
                                    border.width: (selectedNode && selectedNode.patternIndex === index) ? 2 : 1
                                    radius: 4

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        spacing: 2

                                        // Mini preview canvas
                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            color: Theme.previewBackground
                                            radius: 2

                                            Canvas {
                                                id: patternMiniPreview
                                                anchors.fill: parent
                                                anchors.margins: 2

                                                property int patternIdx: index
                                                property var patternFrame: selectedNode ? selectedNode.getPatternFrame(index) : null

                                                Component.onCompleted: requestPaint()
                                                onPatternFrameChanged: requestPaint()

                                                onPaint: {
                                                    var ctx = getContext("2d")
                                                    ctx.reset()
                                                    ctx.fillStyle = Theme.previewBackground
                                                    ctx.fillRect(0, 0, width, height)

                                                    if (!patternFrame) return

                                                    var count = patternFrame.sampleCount
                                                    if (count === 0) return

                                                    ctx.lineWidth = 1.5
                                                    ctx.lineCap = "round"

                                                    var centerX = width / 2
                                                    var centerY = height / 2
                                                    var scale = Math.min(width, height) / 2 * 0.85

                                                    for (var i = 0; i < count - 1; i++) {
                                                        var x1 = patternFrame.sampleX(i)
                                                        var y1 = patternFrame.sampleY(i)
                                                        var x2 = patternFrame.sampleX(i + 1)
                                                        var y2 = patternFrame.sampleY(i + 1)

                                                        var r = Math.round(patternFrame.sampleR(i) * 255)
                                                        var g = Math.round(patternFrame.sampleG(i) * 255)
                                                        var b = Math.round(patternFrame.sampleB(i) * 255)

                                                        ctx.strokeStyle = "rgb(" + r + "," + g + "," + b + ")"
                                                        ctx.beginPath()
                                                        ctx.moveTo(centerX + x1 * scale, centerY - y1 * scale)
                                                        ctx.lineTo(centerX + x2 * scale, centerY - y2 * scale)
                                                        ctx.stroke()
                                                    }
                                                }
                                            }
                                        }

                                        // Pattern name
                                        Label {
                                            text: modelData
                                            color: (selectedNode && selectedNode.patternIndex === index) ? Theme.textOnHighlight : Theme.text
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
                                        onClicked: if (selectedNode) selectedNode.patternIndex = index
                                        onEntered: {
                                            // Preview pattern on hover
                                            if (selectedNode) selectedNode.previewPatternIndex = index
                                        }
                                        onExited: {
                                            // Restore current pattern
                                            if (selectedNode) selectedNode.previewPatternIndex = -1
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
                visible: selectedNode && selectedNode.currentFrame

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Samples:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 60 }
                    Label {
                        text: selectedNode && selectedNode.currentFrame ? String(selectedNode.currentFrame.sampleCount) : "0"
                        color: Theme.text
                        font.pixelSize: Theme.propFontSize
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
                        currentIndex: selectedNode ? selectedNode.zoneIndex : 0
                        enabled: laserEngine && laserEngine.connected
                        onActivated: if (selectedNode) selectedNode.zoneIndex = currentIndex
                        Layout.fillWidth: true
                    }

                    Label { text: qsTr("Enabled:"); color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                    CheckBox {
                        checked: selectedNode ? selectedNode.enabled : true
                        onToggled: if (selectedNode) selectedNode.enabled = checked
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

                    // Shape icon buttons
                    Repeater {
                        model: [
                            { shape: 0, name: qsTr("Rectangle"), icon: "rect" },
                            { shape: 1, name: qsTr("Ellipse"), icon: "ellipse" },
                            { shape: 2, name: qsTr("Angle"), icon: "angle" },
                            { shape: 3, name: qsTr("Linear Wave"), icon: "linearWave" },
                            { shape: 4, name: qsTr("Circular Wave"), icon: "circularWave" }
                        ]

                        delegate: Rectangle {
                            width: 48
                            height: 48
                            radius: 6
                            color: selectedNode && selectedNode.shape === modelData.shape ? Theme.accent : Theme.surface
                            border.color: shapeMouseArea.containsMouse ? Theme.accent : Theme.border
                            border.width: selectedNode && selectedNode.shape === modelData.shape ? 2 : 1

                            Canvas {
                                id: shapeIcon
                                anchors.fill: parent
                                anchors.margins: 8

                                // Repaint when selection changes
                                property bool isSelected: selectedNode && selectedNode.shape === modelData.shape
                                onIsSelectedChanged: requestPaint()

                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)
                                    ctx.strokeStyle = selectedNode && selectedNode.shape === modelData.shape ? "#FFFFFF" : Theme.text
                                    ctx.lineWidth = 2
                                    ctx.fillStyle = "transparent"

                                    var cx = width / 2, cy = height / 2
                                    var r = Math.min(width, height) / 2 - 2

                                    if (modelData.icon === "rect") {
                                        ctx.strokeRect(4, 4, width - 8, height - 8)
                                    } else if (modelData.icon === "ellipse") {
                                        ctx.beginPath()
                                        ctx.ellipse(cx, cy, r, r * 0.7, 0, 0, 2 * Math.PI)
                                        ctx.stroke()
                                    } else if (modelData.icon === "angle") {
                                        ctx.beginPath()
                                        ctx.moveTo(4, height - 4)
                                        ctx.lineTo(width / 2, 4)
                                        ctx.lineTo(width - 4, height - 4)
                                        ctx.stroke()
                                    } else if (modelData.icon === "linearWave") {
                                        ctx.beginPath()
                                        ctx.moveTo(4, cy)
                                        for (var x = 4; x < width - 4; x += 2) {
                                            var t = (x - 4) / (width - 8)
                                            ctx.lineTo(x, cy + Math.sin(t * Math.PI * 2) * r * 0.6)
                                        }
                                        ctx.stroke()
                                    } else if (modelData.icon === "circularWave") {
                                        for (var i = 0; i < 3; i++) {
                                            ctx.beginPath()
                                            ctx.arc(cx, cy, r * (0.3 + i * 0.3), 0, 2 * Math.PI)
                                            ctx.stroke()
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                id: shapeMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: if (selectedNode) selectedNode.shape = modelData.shape

                                ToolTip.visible: modelData && containsMouse
                                ToolTip.text: modelData ? modelData.name : ""
                                ToolTip.delay: 300
                            }
                        }
                    }
                }
            }

            // Position
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Position")


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("Center X")
                        value: selectedNode ? selectedNode.centerX : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 0
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center Y")
                        value: selectedNode ? selectedNode.centerY : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 1
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerY = newValue }
                    }
                }
            }

            // Size (borders)
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Size")


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("H Border")
                        value: selectedNode ? selectedNode.horizontalBorder : 0.5
                        minValue: 0.01
                        maxValue: 2.0
                        defaultValue: 0.5
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 2
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.horizontalBorder = newValue }
                    }

                    ParameterRow {
                        label: qsTr("V Border")
                        value: selectedNode ? selectedNode.verticalBorder : 0.5
                        minValue: 0.01
                        maxValue: 2.0
                        defaultValue: 0.5
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 3
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.verticalBorder = newValue }
                    }
                }
            }

            // Falloff
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Falloff")


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("Amount")
                        value: selectedNode ? selectedNode.falloff : 0.2
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 0.2
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 4
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.falloff = newValue }
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
                                selected: selectedNode && selectedNode.falloffCurve === index
                                onClicked: if (selectedNode) selectedNode.falloffCurve = index
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }

            // Bend (distortion)
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Bend")


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("H Bend")
                        value: selectedNode ? selectedNode.horizontalBend : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 5
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.horizontalBend = newValue }
                    }

                    ParameterRow {
                        label: qsTr("V Bend")
                        value: selectedNode ? selectedNode.verticalBend : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 6
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.verticalBend = newValue }
                    }
                }
            }

            // Noise
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Noise")


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("Intensity")
                        value: selectedNode ? selectedNode.noiseIntensity : 0
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 9
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.noiseIntensity = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Scale")
                        value: selectedNode ? selectedNode.noiseScale : 1.0
                        minValue: 0.01
                        maxValue: 2.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 10
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.noiseScale = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Speed")
                        value: selectedNode ? selectedNode.noiseSpeed : 0
                        minValue: 0
                        maxValue: 10.0
                        defaultValue: 0
                        stepSize: 0.1
                        suffix: ""
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 11
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.noiseSpeed = newValue }
                    }
                }
            }

            // Shape-specific: Angle
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Angle Shape")
                visible: selectedNode && selectedNode.shape === GizmoNode.Shape.Angle


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("Aperture")
                        value: selectedNode ? selectedNode.aperture : 90
                        minValue: 0
                        maxValue: 360
                        defaultValue: 90
                        stepSize: 1
                        suffix: "°"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 7
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.aperture = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Phase")
                        value: selectedNode ? selectedNode.phase : 0
                        minValue: 0
                        maxValue: 360
                        defaultValue: 0
                        stepSize: 1
                        suffix: "°"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 8
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.phase = newValue }
                    }
                }
            }

            // Shape-specific: Waves
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Wave Shape")
                visible: selectedNode && (selectedNode.shape === GizmoNode.Shape.LinearWave || selectedNode.shape === GizmoNode.Shape.CircularWave)


                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("Wave Count")
                        value: selectedNode ? selectedNode.waveCount : 4
                        minValue: 1
                        maxValue: 20
                        defaultValue: 4
                        stepSize: 1
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 7
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.waveCount = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Phase")
                        value: selectedNode ? selectedNode.phase : 0
                        minValue: 0
                        maxValue: 360
                        defaultValue: 0
                        stepSize: 1
                        suffix: "°"
                        node: selectedNode
                        trackName: "Gizmo"
                        paramIndex: 8
                        paramCount: 12
                        trackColor: "#6080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.phase = newValue }
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
                        checked: selectedNode ? selectedNode.singleInputMode : false
                        onCheckedChanged: if (selectedNode) selectedNode.singleInputMode = checked
                        font.pixelSize: Theme.propFontSize

                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Use only one input (no combination)")
                        ToolTip.delay: 500
                    }

                    // Composition mode selector (hidden in single input mode)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: selectedNode ? !selectedNode.singleInputMode : true

                        Label {
                            text: qsTr("Mode:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 55
                        }

                        StyledComboBox {
                            model: [qsTr("Normal"), qsTr("Max"), qsTr("Min"), qsTr("Sum"), qsTr("AbsDiff"), qsTr("Diff"), qsTr("Product")]
                            currentIndex: selectedNode ? selectedNode.compositionMode : 0
                            onActivated: if (selectedNode) selectedNode.compositionMode = currentIndex
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Position")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("X")
                        value: selectedNode ? selectedNode.positionX : 0
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.positionX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Y")
                        value: selectedNode ? selectedNode.positionY : 0
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.positionY = newValue }
                    }
                }
            }

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Scale")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("X")
                        value: selectedNode ? selectedNode.scaleX : 1.0
                        minValue: 0.01
                        maxValue: 10.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.scaleX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Y")
                        value: selectedNode ? selectedNode.scaleY : 1.0
                        minValue: 0.01
                        maxValue: 10.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.scaleY = newValue }
                    }
                }
            }

            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Rotation")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ParameterRow {
                        label: qsTr("Angle")
                        value: selectedNode ? selectedNode.rotation : 0
                        minValue: -360
                        maxValue: 360
                        defaultValue: 0
                        stepSize: 1
                        displayRatio: 1
                        suffix: "°"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.rotation = newValue }
                    }
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
                        value: selectedNode ? selectedNode.delay : 0
                        minValue: -10.0
                        maxValue: 10.0
                        defaultValue: 0
                        stepSize: 0.001
                        decimals: 3
                        displayRatio: 1000
                        suffix: " ms"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.delay = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Scale")
                        value: selectedNode ? selectedNode.scale : 1.0
                        minValue: 0.01
                        maxValue: 10.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.scale = newValue }
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
                            checked: selectedNode ? selectedNode.loop : false
                            onToggled: if (selectedNode) selectedNode.loop = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    ParameterRow {
                        visible: loopCheck.checked
                        label: qsTr("Duration")
                        value: selectedNode ? selectedNode.loopDuration : 1.0
                        minValue: 0.001
                        maxValue: 60.0
                        defaultValue: 1.0
                        stepSize: 0.001
                        decimals: 3
                        displayRatio: 1000
                        suffix: " ms"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.loopDuration = newValue }
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
                            currentIndex: selectedNode ? selectedNode.surfaceType : 0
                            onActivated: if (selectedNode) selectedNode.surfaceType = currentIndex
                            Layout.fillWidth: true
                        }
                    }

                    ParameterRow {
                        label: qsTr("Amplitude")
                        value: selectedNode ? selectedNode.amplitude : 1.0
                        minValue: 0
                        maxValue: 2.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.amplitude = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Frequency")
                        value: selectedNode ? selectedNode.frequency : 1.0
                        minValue: 0.01
                        maxValue: 10.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.frequency = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Phase")
                        value: selectedNode ? selectedNode.phase : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.phase = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Offset")
                        value: selectedNode ? selectedNode.offset : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.offset = newValue }
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
                            checked: selectedNode ? selectedNode.clamp : true
                            onToggled: if (selectedNode) selectedNode.clamp = checked
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
                        property bool isSelected: selectedNode && selectedNode.axis === 0
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
                            onClicked: if (selectedNode) selectedNode.axis = 0
                            onContainsMouseChanged: btnHorizontal.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("Horizontal")
                    }

                    // Vertical axis button (horizontal line ―)
                    Rectangle {
                        id: btnVertical
                        property bool isSelected: selectedNode && selectedNode.axis === 1
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
                            onClicked: if (selectedNode) selectedNode.axis = 1
                            onContainsMouseChanged: btnVertical.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("Vertical")
                    }

                    // +45° axis button (diagonal /)
                    Rectangle {
                        id: btnDiag45
                        property bool isSelected: selectedNode && selectedNode.axis === 2
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
                            onClicked: if (selectedNode) selectedNode.axis = 2
                            onContainsMouseChanged: btnDiag45.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("+45°")
                    }

                    // -45° axis button (diagonal \)
                    Rectangle {
                        id: btnDiagMinus45
                        property bool isSelected: selectedNode && selectedNode.axis === 3
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
                            onClicked: if (selectedNode) selectedNode.axis = 3
                            onContainsMouseChanged: btnDiagMinus45.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("-45°")
                    }

                    // Custom angle button (angle icon)
                    Rectangle {
                        id: btnCustom
                        property bool isSelected: selectedNode && selectedNode.axis === 4
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
                            onClicked: if (selectedNode) selectedNode.axis = 4
                            onContainsMouseChanged: btnCustom.isHovered = containsMouse
                        }

                        ToolTip.visible: isHovered
                        ToolTip.text: qsTr("Custom Angle")
                    }
                }

                // Custom angle spinbox (only visible when Custom is selected)
                RowLayout {
                    Layout.fillWidth: true
                    visible: selectedNode && selectedNode.axis === 4
                    spacing: 8

                    Label {
                        text: qsTr("Angle:")
                        color: Theme.propLabel
                        font.pixelSize: Theme.propFontSize
                    }
                    StyledSpinBox {
                        Layout.fillWidth: true
                        from: -180; to: 180
                        value: selectedNode ? Math.round(selectedNode.customAngle) : 0
                        onValueModified: if (selectedNode) selectedNode.customAngle = value
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

                    ParameterRow {
                        label: qsTr("X")
                        value: selectedNode ? selectedNode.offsetX : 0
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Position"
                        paramIndex: 0
                        paramCount: 2
                        trackColor: "#4080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.offsetX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Y")
                        value: selectedNode ? selectedNode.offsetY : 0
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Position"
                        paramIndex: 1
                        paramCount: 2
                        trackColor: "#4080C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.offsetY = newValue }
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

                    ParameterRow {
                        label: qsTr("Scale X")
                        value: selectedNode ? selectedNode.scaleX : 1.0
                        minValue: 0.01
                        maxValue: 5.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Scale"
                        paramIndex: 0
                        paramCount: 4
                        trackColor: "#40C080"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.scaleX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Scale Y")
                        value: selectedNode ? selectedNode.scaleY : 1.0
                        minValue: 0.01
                        maxValue: 5.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Scale"
                        paramIndex: 1
                        paramCount: 4
                        trackColor: "#40C080"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.scaleY = newValue }
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
                            checked: selectedNode ? selectedNode.uniform : true
                            onToggled: if (selectedNode) selectedNode.uniform = checked
                        }

                        Item { Layout.fillWidth: true }
                    }

                    ParameterRow {
                        label: qsTr("Center X")
                        value: selectedNode ? selectedNode.centerX : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Scale"
                        paramIndex: 2
                        paramCount: 4
                        trackColor: "#40C080"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center Y")
                        value: selectedNode ? selectedNode.centerY : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Scale"
                        paramIndex: 3
                        paramCount: 4
                        trackColor: "#40C080"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerY = newValue }
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
                            checked: selectedNode ? selectedNode.crossOver : false
                            onToggled: if (selectedNode) selectedNode.crossOver = checked
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
                            checked: selectedNode ? selectedNode.followGizmo : false
                            onToggled: if (selectedNode) selectedNode.followGizmo = checked
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

                    ParameterRow {
                        label: qsTr("Angle")
                        value: selectedNode ? selectedNode.angle : 0
                        minValue: -360
                        maxValue: 360
                        defaultValue: 0
                        stepSize: 1
                        suffix: "°"
                        node: selectedNode
                        trackName: "Rotation"
                        paramIndex: 0
                        paramCount: 3
                        trackColor: "#C04080"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.angle = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center X")
                        value: selectedNode ? selectedNode.centerX : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Rotation"
                        paramIndex: 1
                        paramCount: 3
                        trackColor: "#C04080"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center Y")
                        value: selectedNode ? selectedNode.centerY : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Rotation"
                        paramIndex: 2
                        paramCount: 3
                        trackColor: "#C04080"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerY = newValue }
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
                            checked: selectedNode ? selectedNode.followGizmo : false
                            onToggled: if (selectedNode) selectedNode.followGizmo = checked
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

                    // Mode selector
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Mode:")
                            color: Theme.propLabel
                            font.pixelSize: Theme.propFontSize
                            Layout.preferredWidth: 70
                        }

                        StyledComboBox {
                            model: [qsTr("Tint"), qsTr("Multiply"), qsTr("Add"), qsTr("Replace")]
                            currentIndex: selectedNode ? selectedNode.mode : 0
                            onActivated: if (selectedNode) selectedNode.mode = currentIndex
                            Layout.fillWidth: true
                        }
                    }

                    // Color picker
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
                            color: selectedNode ? selectedNode.color : "#FFFFFF"
                            border.color: Theme.border

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (selectedNode) {
                                        root.colorDialog.selectedColor = selectedNode.color
                                        root.colorDialogCallback = function(color) {
                                            selectedNode.color = color
                                        }
                                        root.colorDialog.open()
                                    }
                                }
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }

                    ParameterRow {
                        label: qsTr("Intensity")
                        value: selectedNode ? selectedNode.intensity : 1.0
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Color"
                        paramIndex: 0
                        paramCount: 1
                        trackColor: "#C0C040"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.intensity = newValue }
                    }

                    // Affect channels
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
                            checked: selectedNode ? selectedNode.affectRed : true
                            onToggled: if (selectedNode) selectedNode.affectRed = checked
                        }

                        CheckBox {
                            text: "G"
                            checked: selectedNode ? selectedNode.affectGreen : true
                            onToggled: if (selectedNode) selectedNode.affectGreen = checked
                        }

                        CheckBox {
                            text: "B"
                            checked: selectedNode ? selectedNode.affectBlue : true
                            onToggled: if (selectedNode) selectedNode.affectBlue = checked
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }

            // Filter GroupBox
            StyledGroupBox {
                Layout.fillWidth: true
                title: qsTr("Filter (apply only to)")


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
                        from: 0; to: 100
                        value: selectedNode ? Math.round(selectedNode.filterRedMin * 100) : 0
                        onValueModified: if (selectedNode) selectedNode.filterRedMin = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "-"; color: Theme.propLabel; horizontalAlignment: Text.AlignHCenter }
                    StyledSpinBox {
                        from: 0; to: 100
                        value: selectedNode ? Math.round(selectedNode.filterRedMax * 100) : 100
                        onValueModified: if (selectedNode) selectedNode.filterRedMax = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "%"; color: Theme.propLabel; font.pixelSize: Theme.propFontSize }

                    // Green
                    Label { text: "G"; color: "#66FF66"; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 30 }
                    StyledSpinBox {
                        from: 0; to: 100
                        value: selectedNode ? Math.round(selectedNode.filterGreenMin * 100) : 0
                        onValueModified: if (selectedNode) selectedNode.filterGreenMin = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "-"; color: Theme.propLabel; horizontalAlignment: Text.AlignHCenter }
                    StyledSpinBox {
                        from: 0; to: 100
                        value: selectedNode ? Math.round(selectedNode.filterGreenMax * 100) : 100
                        onValueModified: if (selectedNode) selectedNode.filterGreenMax = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "%"; color: Theme.propLabel; font.pixelSize: Theme.propFontSize }

                    // Blue
                    Label { text: "B"; color: "#6666FF"; font.pixelSize: Theme.propFontSize; Layout.preferredWidth: 30 }
                    StyledSpinBox {
                        from: 0; to: 100
                        value: selectedNode ? Math.round(selectedNode.filterBlueMin * 100) : 0
                        onValueModified: if (selectedNode) selectedNode.filterBlueMin = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "-"; color: Theme.propLabel; horizontalAlignment: Text.AlignHCenter }
                    StyledSpinBox {
                        from: 0; to: 100
                        value: selectedNode ? Math.round(selectedNode.filterBlueMax * 100) : 100
                        onValueModified: if (selectedNode) selectedNode.filterBlueMax = value / 100
                        Layout.fillWidth: true
                    }
                    Label { text: "%"; color: Theme.propLabel; font.pixelSize: Theme.propFontSize }
                }
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

                    ParameterRow {
                        label: qsTr("Expansion")
                        value: selectedNode ? selectedNode.expansion : 0
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Polar"
                        paramIndex: 0
                        paramCount: 5
                        trackColor: "#8040C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.expansion = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Ring Radius")
                        value: selectedNode ? selectedNode.ringRadius : 0.5
                        minValue: 0.01
                        maxValue: 2.0
                        defaultValue: 0.5
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Polar"
                        paramIndex: 1
                        paramCount: 5
                        trackColor: "#8040C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.ringRadius = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Ring Scale")
                        value: selectedNode ? selectedNode.ringScale : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Polar"
                        paramIndex: 2
                        paramCount: 5
                        trackColor: "#8040C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.ringScale = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center X")
                        value: selectedNode ? selectedNode.centerX : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Polar"
                        paramIndex: 3
                        paramCount: 5
                        trackColor: "#8040C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center Y")
                        value: selectedNode ? selectedNode.centerY : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Polar"
                        paramIndex: 4
                        paramCount: 5
                        trackColor: "#8040C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerY = newValue }
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
                            checked: selectedNode ? selectedNode.crossOver : false
                            onToggled: if (selectedNode) selectedNode.crossOver = checked
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
                            checked: selectedNode ? selectedNode.targetted : false
                            onToggled: if (selectedNode) selectedNode.targetted = checked
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
                            checked: selectedNode ? selectedNode.followGizmo : false
                            onToggled: if (selectedNode) selectedNode.followGizmo = checked
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

                    ParameterRow {
                        label: qsTr("Amplitude")
                        value: selectedNode ? selectedNode.amplitude : 0.1
                        minValue: 0
                        maxValue: 2.0
                        defaultValue: 0.1
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Wave"
                        paramIndex: 0
                        paramCount: 6
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.amplitude = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Wavelength")
                        value: selectedNode ? selectedNode.wavelength : 0.5
                        minValue: 0.01
                        maxValue: 2.0
                        defaultValue: 0.5
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Wave"
                        paramIndex: 1
                        paramCount: 6
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.wavelength = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Phase")
                        value: selectedNode ? selectedNode.phase : 0
                        minValue: 0
                        maxValue: 360
                        defaultValue: 0
                        stepSize: 1
                        suffix: "°"
                        node: selectedNode
                        trackName: "Wave"
                        paramIndex: 2
                        paramCount: 6
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.phase = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Angle")
                        value: selectedNode ? selectedNode.angle : 0
                        minValue: 0
                        maxValue: 360
                        defaultValue: 0
                        stepSize: 1
                        suffix: "°"
                        node: selectedNode
                        trackName: "Wave"
                        paramIndex: 3
                        paramCount: 6
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.angle = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center X")
                        value: selectedNode ? selectedNode.centerX : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Wave"
                        paramIndex: 4
                        paramCount: 6
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center Y")
                        value: selectedNode ? selectedNode.centerY : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Wave"
                        paramIndex: 5
                        paramCount: 6
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerY = newValue }
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
                            checked: selectedNode ? selectedNode.radial : true
                            onToggled: if (selectedNode) selectedNode.radial = checked
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
                            checked: selectedNode ? selectedNode.followGizmo : false
                            onToggled: if (selectedNode) selectedNode.followGizmo = checked
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

                    ParameterRow {
                        label: qsTr("Intensity")
                        value: selectedNode ? selectedNode.intensity : 0.5
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0.5
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Squeeze"
                        paramIndex: 0
                        paramCount: 3
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.intensity = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Angle")
                        value: selectedNode ? selectedNode.angle : 0
                        minValue: 0
                        maxValue: 360
                        defaultValue: 0
                        stepSize: 1
                        suffix: "°"
                        node: selectedNode
                        trackName: "Squeeze"
                        paramIndex: 1
                        paramCount: 3
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.angle = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center X")
                        value: selectedNode ? selectedNode.centerX : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Squeeze"
                        paramIndex: 2
                        paramCount: 3
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerX = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Center Y")
                        value: selectedNode ? selectedNode.centerY : 0
                        minValue: -1.0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Squeeze"
                        paramIndex: 2
                        paramCount: 3
                        trackColor: "#40C0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.centerY = newValue }
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
                            checked: selectedNode ? selectedNode.followGizmo : false
                            onToggled: if (selectedNode) selectedNode.followGizmo = checked
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

                    ParameterRow {
                        label: qsTr("Density")
                        value: selectedNode ? selectedNode.density : 0.0
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 0.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Sparkle"
                        paramIndex: 0
                        paramCount: 5
                        trackColor: "#C08040"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.density = newValue }
                    }

                    // Color picker
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
                            color: selectedNode ? Qt.rgba(selectedNode.red, selectedNode.green, selectedNode.blue, 1.0) : "#FFFFFF"
                            border.color: Theme.border

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (selectedNode) {
                                        root.colorDialog.selectedColor = Qt.rgba(selectedNode.red, selectedNode.green, selectedNode.blue, 1.0)
                                        root.colorDialogCallback = function(color) {
                                            selectedNode.red = color.r
                                            selectedNode.green = color.g
                                            selectedNode.blue = color.b
                                        }
                                        root.colorDialog.open()
                                    }
                                }
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }

                    ParameterRow {
                        label: qsTr("Red")
                        value: selectedNode ? selectedNode.red : 1.0
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Sparkle"
                        paramIndex: 1
                        paramCount: 5
                        trackColor: "#FF4040"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.red = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Green")
                        value: selectedNode ? selectedNode.green : 1.0
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Sparkle"
                        paramIndex: 2
                        paramCount: 5
                        trackColor: "#40FF40"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.green = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Blue")
                        value: selectedNode ? selectedNode.blue : 1.0
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Sparkle"
                        paramIndex: 3
                        paramCount: 5
                        trackColor: "#4040FF"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.blue = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Alpha")
                        value: selectedNode ? selectedNode.alpha : 1.0
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Sparkle"
                        paramIndex: 4
                        paramCount: 5
                        trackColor: "#C08040"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.alpha = newValue }
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
                            checked: selectedNode ? selectedNode.followGizmo : true
                            onToggled: if (selectedNode) selectedNode.followGizmo = checked
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

                    ParameterRow {
                        label: qsTr("Amount")
                        value: selectedNode ? selectedNode.amount : 0.1
                        minValue: 0
                        maxValue: 2.0
                        defaultValue: 0.1
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Fuzzyness"
                        paramIndex: 0
                        paramCount: 1
                        trackColor: "#C0A040"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.amount = newValue }
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
                            checked: selectedNode ? selectedNode.affectX : true
                            onToggled: if (selectedNode) selectedNode.affectX = checked
                        }

                        CheckBox {
                            text: "Y"
                            checked: selectedNode ? selectedNode.affectY : true
                            onToggled: if (selectedNode) selectedNode.affectY = checked
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
                            checked: selectedNode ? selectedNode.useSeed : false
                            onToggled: if (selectedNode) selectedNode.useSeed = checked
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
                            from: 0
                            to: 999999
                            value: selectedNode ? selectedNode.seed : 0
                            onValueModified: if (selectedNode) selectedNode.seed = value
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

                    ParameterRow {
                        label: qsTr("Amount")
                        value: selectedNode ? selectedNode.amount : 0.1
                        minValue: 0
                        maxValue: 2.0
                        defaultValue: 0.1
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "ColorFuzzyness"
                        paramIndex: 0
                        paramCount: 1
                        trackColor: "#C0A080"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.amount = newValue }
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
                            checked: selectedNode ? selectedNode.affectRed : true
                            onToggled: if (selectedNode) selectedNode.affectRed = checked
                        }

                        CheckBox {
                            text: "G"
                            checked: selectedNode ? selectedNode.affectGreen : true
                            onToggled: if (selectedNode) selectedNode.affectGreen = checked
                        }

                        CheckBox {
                            text: "B"
                            checked: selectedNode ? selectedNode.affectBlue : true
                            onToggled: if (selectedNode) selectedNode.affectBlue = checked
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
                            checked: selectedNode ? selectedNode.useSeed : false
                            onToggled: if (selectedNode) selectedNode.useSeed = checked
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
                            from: 0
                            to: 999999
                            value: selectedNode ? selectedNode.seed : 0
                            onValueModified: if (selectedNode) selectedNode.seed = value
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

                    ParameterRow {
                        label: qsTr("Threshold")
                        value: selectedNode ? selectedNode.splitThreshold : 0.5
                        minValue: 0.001
                        maxValue: 4.0
                        defaultValue: 0.5
                        stepSize: 0.01
                        decimals: 3
                        suffix: ""
                        node: selectedNode
                        trackName: "Split"
                        paramIndex: 0
                        paramCount: 1
                        trackColor: "#A0A0C0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.splitThreshold = newValue }
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

                    ParameterRow {
                        label: qsTr("Amount")
                        value: selectedNode ? selectedNode.amount : 0
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Rounder"
                        paramIndex: 0
                        paramCount: 6
                        trackColor: "#C060A0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.amount = newValue }
                    }

                    ParameterRow {
                        label: qsTr("V Shift")
                        value: selectedNode ? selectedNode.verticalShift : 0
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Rounder"
                        paramIndex: 1
                        paramCount: 6
                        trackColor: "#C060A0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.verticalShift = newValue }
                    }

                    ParameterRow {
                        label: qsTr("H Shift")
                        value: selectedNode ? selectedNode.horizontalShift : 0
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Rounder"
                        paramIndex: 2
                        paramCount: 6
                        trackColor: "#C060A0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.horizontalShift = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Tighten")
                        value: selectedNode ? selectedNode.tighten : 0
                        minValue: 0
                        maxValue: 1.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Rounder"
                        paramIndex: 3
                        paramCount: 6
                        trackColor: "#C060A0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.tighten = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Radial Resize")
                        value: selectedNode ? selectedNode.radialResize : 1.0
                        minValue: 0.5
                        maxValue: 2.0
                        defaultValue: 1.0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Rounder"
                        paramIndex: 4
                        paramCount: 6
                        trackColor: "#C060A0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.radialResize = newValue }
                    }

                    ParameterRow {
                        label: qsTr("Radial Shift")
                        value: selectedNode ? selectedNode.radialShift : 0
                        minValue: -2.0
                        maxValue: 2.0
                        defaultValue: 0
                        stepSize: 0.01
                        displayRatio: 100
                        suffix: "%"
                        node: selectedNode
                        trackName: "Rounder"
                        paramIndex: 5
                        paramCount: 6
                        trackColor: "#C060A0"
                        onValueModified: function(newValue) { if (selectedNode) selectedNode.radialShift = newValue }
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
