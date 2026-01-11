import QtQuick
import QtQuick.Controls
import GizmoTweak2

SpinBox {
    id: control

    implicitHeight: Theme.propSpinBoxHeight
    font.pixelSize: Theme.propFontSize
    editable: true

    contentItem: TextInput {
        z: 2
        text: control.textFromValue(control.value, control.locale)
        font: control.font
        color: Theme.text
        selectionColor: Theme.accent
        selectedTextColor: Theme.text
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly

        Rectangle {
            anchors.fill: parent
            anchors.margins: -4
            z: -1
            color: "transparent"
        }
    }

    up.indicator: Rectangle {
        x: control.mirrored ? 0 : parent.width - width
        height: parent.height
        implicitWidth: Theme.propSpinBoxHeight
        implicitHeight: Theme.propSpinBoxHeight
        color: control.up.pressed ? Theme.surfacePressed
             : control.up.hovered ? Theme.surfaceHover
             : Theme.surface
        border.color: Theme.border
        radius: 4

        Text {
            text: "+"
            font.pixelSize: Theme.propFontSize
            font.bold: true
            color: control.up.pressed ? Theme.accent : Theme.text
            anchors.centerIn: parent
        }
    }

    down.indicator: Rectangle {
        x: control.mirrored ? parent.width - width : 0
        height: parent.height
        implicitWidth: Theme.propSpinBoxHeight
        implicitHeight: Theme.propSpinBoxHeight
        color: control.down.pressed ? Theme.surfacePressed
             : control.down.hovered ? Theme.surfaceHover
             : Theme.surface
        border.color: Theme.border
        radius: 4

        Text {
            text: "\u2212"  // Unicode minus
            font.pixelSize: Theme.propFontSize
            font.bold: true
            color: control.down.pressed ? Theme.accent : Theme.text
            anchors.centerIn: parent
        }
    }

    background: Rectangle {
        implicitWidth: 100
        color: Theme.background
        border.color: control.activeFocus ? Theme.accent : Theme.border
        border.width: control.activeFocus ? 2 : 1
        radius: 4
    }
}
