import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweakLib2
import GizmoTweak2

Rectangle {
    id: root

    property var selectedNode: null

    color: Theme.surface
    border.color: Theme.border
    border.width: 1

    visible: selectedNode !== null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Header
        Label {
            text: selectedNode ? selectedNode.type : ""
            color: Theme.text
            font.pixelSize: Theme.fontSizeNormal
            font.bold: true
            Layout.fillWidth: true
        }

        Rectangle {
            height: 1
            color: Theme.border
            Layout.fillWidth: true
        }

        // Common properties
        GridLayout {
            columns: 2
            columnSpacing: 8
            rowSpacing: 4
            Layout.fillWidth: true

            Label {
                text: qsTr("Name:")
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSmall
            }
            TextField {
                id: nameField
                Layout.fillWidth: true
                text: selectedNode ? selectedNode.displayName : ""
                color: Theme.text
                font.pixelSize: Theme.fontSizeSmall
                background: Rectangle {
                    color: Theme.background
                    border.color: nameField.activeFocus ? Theme.accent : Theme.border
                    radius: 2
                }
                onEditingFinished: {
                    if (selectedNode) {
                        selectedNode.displayName = text
                    }
                }
            }
        }

        // Type-specific properties
        Loader {
            id: propertiesLoader
            Layout.fillWidth: true
            Layout.fillHeight: true

            sourceComponent: {
                if (!selectedNode) return null
                switch (selectedNode.type) {
                    case "Gizmo": return gizmoProperties
                    case "Group": return groupProperties
                    case "TimeShift": return timeShiftProperties
                    case "SurfaceFactory": return surfaceFactoryProperties
                    default: return null
                }
            }
        }

        Item { Layout.fillHeight: true }
    }

    // Gizmo properties
    Component {
        id: gizmoProperties
        GridLayout {
            columns: 2
            columnSpacing: 8
            rowSpacing: 4

            Label { text: qsTr("Center X:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: centerXSpin
                from: -100
                to: 100
                value: selectedNode ? Math.round(selectedNode.centerX * 100) : 0
                onValueModified: if (selectedNode) selectedNode.centerX = value / 100.0
                Layout.fillWidth: true
            }

            Label { text: qsTr("Center Y:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: centerYSpin
                from: -100
                to: 100
                value: selectedNode ? Math.round(selectedNode.centerY * 100) : 0
                onValueModified: if (selectedNode) selectedNode.centerY = value / 100.0
                Layout.fillWidth: true
            }

            Label { text: qsTr("Radius:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: radiusSpin
                from: 0
                to: 200
                value: selectedNode ? Math.round(selectedNode.radius * 100) : 50
                onValueModified: if (selectedNode) selectedNode.radius = value / 100.0
                Layout.fillWidth: true
            }

            Label { text: qsTr("Falloff:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: falloffSpin
                from: 0
                to: 100
                value: selectedNode ? Math.round(selectedNode.falloff * 100) : 20
                onValueModified: if (selectedNode) selectedNode.falloff = value / 100.0
                Layout.fillWidth: true
            }
        }
    }

    // Group properties
    Component {
        id: groupProperties
        GridLayout {
            columns: 2
            columnSpacing: 8
            rowSpacing: 4

            Label { text: qsTr("Mode:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            ComboBox {
                id: modeCombo
                model: [qsTr("Normal"), qsTr("Max"), qsTr("Min"), qsTr("Sum"), qsTr("Product"), qsTr("Average"), qsTr("AbsDiff")]
                currentIndex: selectedNode ? selectedNode.compositionMode : 0
                onActivated: if (selectedNode) selectedNode.compositionMode = currentIndex
                Layout.fillWidth: true
            }

            Label { text: qsTr("Inputs:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: inputCountSpin
                from: 1
                to: 8
                value: selectedNode ? selectedNode.ratioInputCount : 2
                onValueModified: if (selectedNode) selectedNode.ratioInputCount = value
                Layout.fillWidth: true
            }

            Label { text: qsTr("Invert:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            CheckBox {
                id: invertCheck
                checked: selectedNode ? selectedNode.invert : false
                onToggled: if (selectedNode) selectedNode.invert = checked
            }
        }
    }

    // TimeShift properties
    Component {
        id: timeShiftProperties
        GridLayout {
            columns: 2
            columnSpacing: 8
            rowSpacing: 4

            Label { text: qsTr("Delay (s):"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: delaySpin
                from: -10000
                to: 10000
                value: selectedNode ? Math.round(selectedNode.delay * 1000) : 0
                onValueModified: if (selectedNode) selectedNode.delay = value / 1000.0
                Layout.fillWidth: true
            }

            Label { text: qsTr("Scale:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: scaleSpin
                from: 1
                to: 1000
                value: selectedNode ? Math.round(selectedNode.scale * 100) : 100
                onValueModified: if (selectedNode) selectedNode.scale = value / 100.0
                Layout.fillWidth: true
            }

            Label { text: qsTr("Loop:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            CheckBox {
                id: loopCheck
                checked: selectedNode ? selectedNode.loop : false
                onToggled: if (selectedNode) selectedNode.loop = checked
            }

            Label { text: qsTr("Duration (s):"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall; visible: loopCheck.checked }
            SpinBox {
                id: durationSpin
                visible: loopCheck.checked
                from: 1
                to: 60000
                value: selectedNode ? Math.round(selectedNode.loopDuration * 1000) : 1000
                onValueModified: if (selectedNode) selectedNode.loopDuration = value / 1000.0
                Layout.fillWidth: true
            }
        }
    }

    // SurfaceFactory properties
    Component {
        id: surfaceFactoryProperties
        GridLayout {
            columns: 2
            columnSpacing: 8
            rowSpacing: 4

            Label { text: qsTr("Type:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            ComboBox {
                id: surfaceTypeCombo
                model: [qsTr("Linear"), qsTr("Sine"), qsTr("Cosine"), qsTr("Triangle"), qsTr("Sawtooth"), qsTr("Square"), qsTr("Exponential"), qsTr("Logarithmic")]
                currentIndex: selectedNode ? selectedNode.surfaceType : 0
                onActivated: if (selectedNode) selectedNode.surfaceType = currentIndex
                Layout.fillWidth: true
            }

            Label { text: qsTr("Amplitude:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: amplitudeSpin
                from: 0
                to: 200
                value: selectedNode ? Math.round(selectedNode.amplitude * 100) : 100
                onValueModified: if (selectedNode) selectedNode.amplitude = value / 100.0
                Layout.fillWidth: true
            }

            Label { text: qsTr("Frequency:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: frequencySpin
                from: 1
                to: 1000
                value: selectedNode ? Math.round(selectedNode.frequency * 100) : 100
                onValueModified: if (selectedNode) selectedNode.frequency = value / 100.0
                Layout.fillWidth: true
            }

            Label { text: qsTr("Phase:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: phaseSpin
                from: -100
                to: 100
                value: selectedNode ? Math.round(selectedNode.phase * 100) : 0
                onValueModified: if (selectedNode) selectedNode.phase = value / 100.0
                Layout.fillWidth: true
            }

            Label { text: qsTr("Offset:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            SpinBox {
                id: offsetSpin
                from: -100
                to: 100
                value: selectedNode ? Math.round(selectedNode.offset * 100) : 0
                onValueModified: if (selectedNode) selectedNode.offset = value / 100.0
                Layout.fillWidth: true
            }

            Label { text: qsTr("Clamp:"); color: Theme.textMuted; font.pixelSize: Theme.fontSizeSmall }
            CheckBox {
                id: clampCheck
                checked: selectedNode ? selectedNode.clamp : true
                onToggled: if (selectedNode) selectedNode.clamp = checked
            }
        }
    }
}
