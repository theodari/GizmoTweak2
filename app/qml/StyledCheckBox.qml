import QtQuick
import QtQuick.Controls
import GizmoTweak2

CheckBox {
    id: control

    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: 2
        color: control.hovered && !control.checked
               ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.15)
               : "transparent"
        border.color: control.hovered
                      ? Qt.lighter(Theme.accent, 1.2)
                      : Theme.accent
        border.width: control.checked ? 2 : 1.5

        Behavior on border.color { ColorAnimation { duration: 100 } }
        Behavior on border.width { NumberAnimation { duration: 100 } }
        Behavior on color { ColorAnimation { duration: 100 } }

        Rectangle {
            width: 10
            height: 10
            x: 4
            y: 4
            radius: 1
            color: Theme.accent
            visible: control.checked
        }
    }

    contentItem: Item {
        implicitWidth: label.implicitWidth + control.indicator.width + control.spacing
        implicitHeight: label.implicitHeight
        visible: control.text !== ""

        Text {
            id: label
            x: control.indicator.width + control.spacing
            text: control.text
            font: control.font
            opacity: enabled ? 1.0 : 0.5
            color: Theme.text
            verticalAlignment: Text.AlignVCenter
            anchors.verticalCenter: parent.verticalCenter
        }

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
    }
}
