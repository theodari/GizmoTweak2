project(GizmoTweak2App)

# Set Qt policy to suppress QTP0001 warning about QML resource prefix
qt_policy(SET QTP0001 NEW)

qt_add_executable(GizmoTweak2 WIN32
    src/main.cpp
    src/FileIO.h
    src/FileIO.cpp
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
        qml/PropertiesPanel.qml
        qml/StyledMenuItem.qml
    SOURCES
        src/FileIO.h
        src/FileIO.cpp
)

target_include_directories(GizmoTweak2
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(GizmoTweak2
    PRIVATE
        GizmoTweakLib2
        GizmoTweakLib2plugin
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickDialogs2
)
