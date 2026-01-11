import QtQuick
import QtQuick.Controls
import GizmoTweak2

ComboBox {
    id: control

    font.pixelSize: Theme.propFontSize

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: Theme.propSpinBoxHeight
        radius: 4
        color: control.pressed ? Theme.surfacePressed
             : control.hovered ? Theme.surfaceHover
             : Theme.surface
        border.color: control.hovered ? Theme.accent : Theme.border
        border.width: 1

        Behavior on color { ColorAnimation { duration: 100 } }
        Behavior on border.color { ColorAnimation { duration: 100 } }
    }

    contentItem: Text {
        leftPadding: 8
        rightPadding: control.indicator.width + 8
        text: control.displayText
        font: control.font
        color: control.enabled ? Theme.text : Theme.textMuted
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Canvas {
        id: canvas
        x: control.width - width - 8
        y: control.height / 2 - height / 2
        width: 10
        height: 6
        contextType: "2d"

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.moveTo(0, 0)
            ctx.lineTo(width, 0)
            ctx.lineTo(width / 2, height)
            ctx.closePath()
            ctx.fillStyle = control.enabled ? Theme.text : Theme.textMuted
            ctx.fill()
        }
    }

    popup: Popup {
        y: control.height
        width: control.width
        implicitHeight: contentItem.implicitHeight + 2
        padding: 1

        background: Rectangle {
            color: Theme.surface
            border.color: Theme.border
            radius: 4
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }

    delegate: ItemDelegate {
        width: control.width
        height: Theme.propSpinBoxHeight

        contentItem: Text {
            text: modelData
            color: highlighted ? Theme.textOnHighlight : Theme.text
            font: control.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            leftPadding: 8
        }

        background: Rectangle {
            color: highlighted ? Theme.menuHighlight : "transparent"
        }

        highlighted: control.highlightedIndex === index
    }
}
