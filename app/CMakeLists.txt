project(GizmoTweak2App)

qt_add_executable(GizmoTweak2 WIN32
    src/main.cpp
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(GizmoTweak2
    URI GizmoTweak2
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/NodeCanvas.qml
        qml/NodeItem.qml
        qml/PortItem.qml
        qml/ConnectionItem.qml
        qml/ConnectionPreview.qml
)

target_link_libraries(GizmoTweak2
    PRIVATE
        GizmoTweakLib2
        GizmoTweakLib2plugin
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickControls2
)
