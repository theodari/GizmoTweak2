project(GizmoTweak2App)

# Set Qt policy to suppress QTP0001 warning about QML resource prefix
qt_policy(SET QTP0001 NEW)

set(APP_NAME "GizmoTweak2")

message(STATUS "Building ${APP_NAME}")

set(ENGINE_SOURCES
    src/ExcaliburEngine.h
    src/ExcaliburEngine.cpp
)

qt_add_executable(${APP_NAME} WIN32
    src/main.cpp
    src/FileIO.h
    src/FileIO.cpp
    ${ENGINE_SOURCES}
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(${APP_NAME}
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
        qml/PreviewPanel.qml
        qml/StyledMenuItem.qml
        qml/StyledCheckBox.qml
        qml/StyledComboBox.qml
        qml/StyledSpinBox.qml
        qml/StyledGroupBox.qml
        qml/NodeToolbar.qml
        qml/TimelinePanel.qml
        qml/TrackItem.qml
        qml/GearButton.qml
        qml/ParameterRow.qml
        qml/ParameterGroup.qml
        qml/KeyframeParameterRow.qml
        qml/ReadOnlyParameterRow.qml
        qml/CurveIconButton.qml
        qml/Toast.qml
        qml/KeyboardShortcutsDialog.qml
    SOURCES
        src/FileIO.h
        src/FileIO.cpp
        src/NodePreviewItem.h
        src/NodePreviewItem.cpp
        src/FramePreviewItem.h
        src/FramePreviewItem.cpp
        src/RecentFilesManager.h
        src/RecentFilesManager.cpp
        src/PatternImageProvider.h
        src/PatternImageProvider.cpp
        ${ENGINE_SOURCES}
    RESOURCES
        resources/gizmoTweakPatterns.ild
        resources/templates/basic_chain.gt2
        resources/templates/rotating_circle.gt2
        resources/templates/wave_effect.gt2
        resources/templates/multi_gizmo.gt2
        resources/templates/mirror_effect.gt2
        resources/icons/gizmo_rectangle.png
        resources/icons/gizmo_ellipse.png
        resources/icons/gizmo_angle.png
        resources/icons/gizmo_linear_waves.png
        resources/icons/gizmo_circular_waves.png
)

target_include_directories(${APP_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${APP_NAME}
    PRIVATE
        GizmoTweakLib2
        GizmoTweakLib2plugin
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickDialogs2
)

# Debug postfix
set_target_properties(${APP_NAME} PROPERTIES
    DEBUG_POSTFIX "d"
)
