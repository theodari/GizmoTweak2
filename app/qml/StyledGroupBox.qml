import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GizmoTweak2

GroupBox {
    id: control

    topPadding: 26
    leftPadding: 8
    rightPadding: 8
    bottomPadding: 8

    background: Rectangle {
        y: control.topPadding - control.padding
        width: parent.width
        height: parent.height - control.topPadding + control.padding
        color: "transparent"
        border.color: Theme.propGroupBorder
        border.width: 1
        radius: 6
    }

    label: Label {
        x: control.leftPadding
        width: control.availableWidth
        text: control.title
        color: Theme.propGroupTitle
        font.pixelSize: Theme.propFontSizeTitle
        font.bold: true
        elide: Text.ElideRight
    }
}
