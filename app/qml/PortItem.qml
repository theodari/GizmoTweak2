import QtQuick
import QtQuick.Controls
import GizmoTweakLib2
import GizmoTweak2

Item {
    id: root

    property Port port: null
    property bool isInput: true
    property bool showLabel: true

    // Connection dragging state (set by parent NodeCanvas)
    property bool connectionDragging: false
    property point dragPosition: Qt.point(0, 0)

    signal dragStarted(var port, point mousePos)
    signal dragUpdated(point mousePos)
    signal dragEnded(point mousePos)  // Changed: now passes position, not port

    visible: port !== null
    implicitWidth: showLabel ? portCircle.width + 6 + labelText.implicitWidth : portCircle.width
    implicitHeight: Math.max(portCircle.height, labelText.implicitHeight)

    onPortChanged: Qt.callLater(portCircle.updateScenePosition)
    onVisibleChanged: if (visible) Qt.callLater(portCircle.updateScenePosition)

    // Check if drag is over this port
    readonly property bool isDragOver: {
        if (!connectionDragging || !port) return false
        var portCenter = port.scenePosition
        var dx = dragPosition.x - portCenter.x
        var dy = dragPosition.y - portCenter.y
        var dist = Math.sqrt(dx * dx + dy * dy)
        return dist < Theme.portRadius + 8  // Hit radius
    }

    Item {
        id: portCircle
        anchors.left: isInput ? parent.left : undefined
        anchors.right: isInput ? undefined : parent.right
        anchors.verticalCenter: parent.verticalCenter

        // Hover state
        property bool isHovering: portMouseArea.containsMouse || isDragOver

        // Size increases on hover
        property real baseSize: Theme.portRadius * 2
        property real hoverSize: Theme.portRadius * 2.4
        width: isHovering ? hoverSize : baseSize
        height: isHovering ? hoverSize : baseSize

        // Check if port is unsatisfied (required but not connected)
        property bool isUnsatisfied: port && port.required && !port.satisfied

        // Check if this is a RatioAny port showing dual colors
        property bool isRatioAny: port && port.dataType === Port.DataType.RatioAny
        property bool showDualColors: isRatioAny && port.effectiveDataType === Port.DataType.RatioAny

        // Effective color based on effective data type
        property color effectiveColor: {
            if (!port) return Theme.border
            var dt = port.effectiveDataType
            if (dt === Port.DataType.Frame) return Theme.portFrame
            if (dt === Port.DataType.Ratio2D) return Theme.portRatio2D
            if (dt === Port.DataType.Ratio1D) return Theme.portRatio1D
            if (dt === Port.DataType.RatioAny) return Theme.border  // Will show dual colors
            return Theme.border
        }

        Behavior on width { NumberAnimation { duration: 80 } }
        Behavior on height { NumberAnimation { duration: 80 } }

        // Single color circle (when connected or non-RatioAny)
        Rectangle {
            id: singleColorCircle
            anchors.fill: parent
            radius: width / 2
            visible: !portCircle.showDualColors
            color: portCircle.isHovering ? Qt.lighter(portCircle.effectiveColor, 1.3) : portCircle.effectiveColor
            border.color: portCircle.isUnsatisfied ? Theme.error : (portCircle.isHovering ? Theme.text : "transparent")
            border.width: portCircle.isUnsatisfied ? 2 : 2
            opacity: portCircle.isUnsatisfied ? unsatisfiedAnimation.opacity : 1.0
        }

        // Pulsing animation for unsatisfied ports
        SequentialAnimation {
            id: unsatisfiedAnimation
            running: portCircle.isUnsatisfied
            loops: Animation.Infinite
            property real opacity: 1.0
            NumberAnimation {
                target: unsatisfiedAnimation
                property: "opacity"
                from: 1.0
                to: 0.4
                duration: 600
                easing.type: Easing.InOutSine
            }
            NumberAnimation {
                target: unsatisfiedAnimation
                property: "opacity"
                from: 0.4
                to: 1.0
                duration: 600
                easing.type: Easing.InOutSine
            }
        }

        // Dual color display (when RatioAny and not connected)
        // Top half = Ratio2D (green), Bottom half = Ratio1D (orange)
        Canvas {
            id: dualColorCircle
            anchors.fill: parent
            visible: portCircle.showDualColors

            property color topColor: portCircle.isHovering ? Qt.lighter(Theme.portRatio2D, 1.3) : Theme.portRatio2D
            property color bottomColor: portCircle.isHovering ? Qt.lighter(Theme.portRatio1D, 1.3) : Theme.portRatio1D
            property color borderColor: portCircle.isUnsatisfied ? Theme.error : (portCircle.isHovering ? Theme.text : Theme.border)

            opacity: portCircle.isUnsatisfied ? unsatisfiedAnimation.opacity : 1.0

            onTopColorChanged: requestPaint()
            onBottomColorChanged: requestPaint()
            onBorderColorChanged: requestPaint()
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                var centerX = width / 2
                var centerY = height / 2
                var radius = Math.min(width, height) / 2 - 1

                // Top half (Ratio2D - green)
                ctx.beginPath()
                ctx.arc(centerX, centerY, radius, Math.PI, 0, false)
                ctx.closePath()
                ctx.fillStyle = topColor
                ctx.fill()

                // Bottom half (Ratio1D - orange)
                ctx.beginPath()
                ctx.arc(centerX, centerY, radius, 0, Math.PI, false)
                ctx.closePath()
                ctx.fillStyle = bottomColor
                ctx.fill()

                // Border
                ctx.beginPath()
                ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI, false)
                ctx.lineWidth = 2
                ctx.strokeStyle = borderColor
                ctx.stroke()
            }
        }

        // Update scene position for connections
        onXChanged: Qt.callLater(updateScenePosition)
        onYChanged: Qt.callLater(updateScenePosition)
        Component.onCompleted: Qt.callLater(updateScenePosition)

        function updateScenePosition() {
            if (port && visible) {
                // Map to scaledContent for correct coordinates (same space as connections)
                var pos = mapToItem(scaledContentRef, width / 2, height / 2)
                port.scenePosition = pos
            }
        }

        // Find the NodeItem (has nodeData property) to track its position
        function findNodeItem() {
            var p = root.parent
            while (p) {
                if (p.nodeData !== undefined) return p
                p = p.parent
            }
            return null
        }

        // Find the scaledContent ancestor to use as coordinate reference
        function findScaledContent() {
            var p = root.parent
            while (p) {
                if (p.objectName === "scaledContent") return p
                p = p.parent
            }
            return null
        }

        property Item nodeItem: findNodeItem()
        property var scaledContentRef: findScaledContent()

        Connections {
            target: portCircle.nodeItem
            function onXChanged() { Qt.callLater(portCircle.updateScenePosition) }
            function onYChanged() { Qt.callLater(portCircle.updateScenePosition) }
            function onWidthChanged() { Qt.callLater(portCircle.updateScenePosition) }
            function onHeightChanged() { Qt.callLater(portCircle.updateScenePosition) }
        }

        // Update position when node properties change (e.g., port visibility changes)
        // Use a timer to ensure the layout has recalculated after visibility changes
        Timer {
            id: layoutUpdateTimer
            interval: 16  // One frame delay
            onTriggered: portCircle.updateScenePosition()
        }
        Connections {
            target: portCircle.nodeItem ? portCircle.nodeItem.nodeData : null
            function onPropertyChanged() { layoutUpdateTimer.restart() }
        }

        // Update positions when zoom changes
        Connections {
            target: portCircle.scaledContentRef
            function onScaleChanged() { Qt.callLater(portCircle.updateScenePosition) }
        }

        MouseArea {
            id: portMouseArea
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            preventStealing: true

            property bool isDragging: false

            onPressed: function(mouse) {
                mouse.accepted = true
                if (!port) return
                isDragging = true
                var pos = mapToItem(portCircle.scaledContentRef, mouse.x, mouse.y)
                root.dragStarted(port, pos)
            }

            onPositionChanged: function(mouse) {
                if (isDragging) {
                    var pos = mapToItem(portCircle.scaledContentRef, mouse.x, mouse.y)
                    root.dragUpdated(pos)
                }
            }

            onReleased: function(mouse) {
                if (isDragging) {
                    isDragging = false
                    var pos = mapToItem(portCircle.scaledContentRef, mouse.x, mouse.y)
                    root.dragEnded(pos)
                }
            }

        }
    }

    Text {
        id: labelText
        visible: showLabel
        anchors.left: isInput ? portCircle.right : undefined
        anchors.right: isInput ? undefined : portCircle.left
        anchors.leftMargin: isInput ? 6 : 0
        anchors.rightMargin: isInput ? 0 : 6
        anchors.verticalCenter: parent.verticalCenter
        text: port ? port.name : ""
        color: Theme.text
        font.pixelSize: Theme.fontSizeSmall
    }
}
