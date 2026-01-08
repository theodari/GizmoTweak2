import QtQuick
import QtQuick.Controls
import GizmoTweak2

MenuItem {
    id: control

    // Convert &X to <u>X</u> for mnemonic display
    function formatMnemonic(text) {
        return text.replace(/&([^&])/g, "<u>$1</u>").replace(/&&/g, "&")
    }

    contentItem: Text {
        leftPadding: control.checkable ? control.indicator.width + control.spacing : 0
        text: control.formatMnemonic(control.text)
        textFormat: Text.RichText
        font: control.font
        opacity: control.enabled ? 1.0 : 0.3
        color: control.highlighted ? Theme.textOnHighlight : Theme.text
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Item {
        x: control.mirrored ? control.width - width - control.rightPadding : control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 20
        height: 20
        visible: control.checkable

        Rectangle {
            width: 14
            height: 14
            anchors.centerIn: parent
            visible: control.checkable && !control.checked
            color: "transparent"
            border.color: control.highlighted ? Theme.textOnHighlight : Theme.textMuted
            radius: 2
        }

        Text {
            anchors.centerIn: parent
            visible: control.checked
            text: "\u2713"
            font.pixelSize: 14
            color: control.highlighted ? Theme.textOnHighlight : Theme.text
        }
    }

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 32
        opacity: control.enabled ? 1 : 0.3
        color: control.highlighted ? Theme.menuHighlight : "transparent"
    }
}
