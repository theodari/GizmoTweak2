import QtQuick
import QtQuick.Controls

// Toast notification component for displaying messages to the user
Item {
    id: root

    // Position at top center of parent
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
    anchors.top: parent ? parent.top : undefined
    anchors.topMargin: 20

    width: Math.min(parent ? parent.width - 40 : 400, 500)
    height: column.height

    visible: toastModel.count > 0

    enum Type {
        Info,
        Success,
        Warning,
        Error
    }

    // Show a toast message
    function show(message, type, duration) {
        type = type !== undefined ? type : Toast.Type.Info
        duration = duration !== undefined ? duration : 3000

        toastModel.append({
            message: message,
            type: type,
            duration: duration
        })
    }

    function showError(message) {
        show(message, Toast.Type.Error, 5000)
    }

    function showWarning(message) {
        show(message, Toast.Type.Warning, 4000)
    }

    function showSuccess(message) {
        show(message, Toast.Type.Success, 3000)
    }

    function showInfo(message) {
        show(message, Toast.Type.Info, 3000)
    }

    ListModel {
        id: toastModel
    }

    Column {
        id: column
        width: parent.width
        spacing: 8

        Repeater {
            model: toastModel

            Rectangle {
                id: toastItem
                width: column.width
                height: toastContent.height + 20
                radius: 6

                color: {
                    switch (model.type) {
                        case Toast.Type.Error: return "#D32F2F"
                        case Toast.Type.Warning: return "#F57C00"
                        case Toast.Type.Success: return "#388E3C"
                        default: return "#1976D2"
                    }
                }

                opacity: 0

                Row {
                    id: toastContent
                    anchors.centerIn: parent
                    spacing: 10
                    width: parent.width - 40

                    Text {
                        id: icon
                        text: {
                            switch (model.type) {
                                case Toast.Type.Error: return "\u2716"    // ✖
                                case Toast.Type.Warning: return "\u26A0"  // ⚠
                                case Toast.Type.Success: return "\u2714"  // ✔
                                default: return "\u2139"                  // ℹ
                            }
                        }
                        color: "white"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    Text {
                        text: model.message
                        color: "white"
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        width: parent.width - icon.width - closeBtn.width - 20
                    }
                }

                // Close button
                Text {
                    id: closeBtn
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: "\u2715"  // ✕
                    color: "white"
                    font.pixelSize: 14
                    opacity: closeArea.containsMouse ? 1.0 : 0.6

                    MouseArea {
                        id: closeArea
                        anchors.fill: parent
                        anchors.margins: -5
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            hideAnimation.start()
                        }
                    }
                }

                // Show animation
                Component.onCompleted: {
                    showAnimation.start()
                    if (model.duration > 0) {
                        hideTimer.interval = model.duration
                        hideTimer.start()
                    }
                }

                NumberAnimation {
                    id: showAnimation
                    target: toastItem
                    property: "opacity"
                    from: 0
                    to: 0.95
                    duration: 200
                    easing.type: Easing.OutQuad
                }

                Timer {
                    id: hideTimer
                    onTriggered: hideAnimation.start()
                }

                SequentialAnimation {
                    id: hideAnimation

                    NumberAnimation {
                        target: toastItem
                        property: "opacity"
                        to: 0
                        duration: 200
                        easing.type: Easing.InQuad
                    }

                    ScriptAction {
                        script: toastModel.remove(index)
                    }
                }
            }
        }
    }
}
