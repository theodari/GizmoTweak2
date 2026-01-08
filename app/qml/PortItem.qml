import QtQuick
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

    Rectangle {
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
        radius: width / 2

        // Base color based on data type
        property color baseColor: !port ? Theme.border : (port.dataType === Port.DataType.Frame ? Theme.portFrame : (port.dataType === Port.DataType.Ratio2D ? Theme.portRatio2D : (port.dataType === Port.DataType.Ratio1D ? Theme.portRatio1D : Theme.border)))

        // Color lightens on hover
        color: isHovering ? Qt.lighter(baseColor, 1.3) : baseColor

        // Border on hover
        border.color: isHovering ? Theme.text : "transparent"
        border.width: 2

        Behavior on width { NumberAnimation { duration: 80 } }
        Behavior on height { NumberAnimation { duration: 80 } }

        // Update scene position for connections
        onXChanged: Qt.callLater(updateScenePosition)
        onYChanged: Qt.callLater(updateScenePosition)
        Component.onCompleted: Qt.callLater(updateScenePosition)

        function updateScenePosition() {
            if (port && visible) {
                // Map to Flickable's contentItem for correct coordinates
                var targetItem = flickableRef ? flickableRef.contentItem : null
                var pos = mapToItem(targetItem, width / 2, height / 2)
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

        // Find the Flickable ancestor to use as coordinate reference
        function findFlickable() {
            var p = root.parent
            while (p) {
                if (p.contentItem !== undefined && p.contentX !== undefined) return p
                p = p.parent
            }
            return null
        }

        property Item nodeItem: findNodeItem()
        property var flickableRef: findFlickable()

        Connections {
            target: portCircle.nodeItem
            function onXChanged() { Qt.callLater(portCircle.updateScenePosition) }
            function onYChanged() { Qt.callLater(portCircle.updateScenePosition) }
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
                var targetItem = portCircle.flickableRef ? portCircle.flickableRef.contentItem : null
                var pos = mapToItem(targetItem, mouse.x, mouse.y)
                root.dragStarted(port, pos)
            }

            onPositionChanged: function(mouse) {
                if (isDragging) {
                    var targetItem = portCircle.flickableRef ? portCircle.flickableRef.contentItem : null
                    var pos = mapToItem(targetItem, mouse.x, mouse.y)
                    root.dragUpdated(pos)
                }
            }

            onReleased: function(mouse) {
                if (isDragging) {
                    isDragging = false
                    var targetItem = portCircle.flickableRef ? portCircle.flickableRef.contentItem : null
                    var pos = mapToItem(targetItem, mouse.x, mouse.y)
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
