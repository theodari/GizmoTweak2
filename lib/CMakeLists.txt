project(GizmoTweakLib2)

# Set Qt policy to suppress QTP0001 warning about QML resource prefix
qt_policy(SET QTP0001 NEW)

set(LIB_SOURCES
    src/gizmotweaklib2.cpp
    src/core/Port.cpp
    src/core/Node.cpp
    src/core/Connection.cpp
    src/core/NodeGraph.cpp
    src/nodes/InputNode.cpp
    src/nodes/OutputNode.cpp
    src/nodes/GizmoNode.cpp
    src/nodes/GroupNode.cpp
    src/nodes/PositionTweak.cpp
    src/nodes/ScaleTweak.cpp
    src/nodes/RotationTweak.cpp
    src/nodes/ColorTweak.cpp
    src/nodes/TimeShiftNode.cpp
    src/nodes/SurfaceFactoryNode.cpp
)

set(LIB_HEADERS
    src/gizmotweaklib2.h
    src/core/Port.h
    src/core/Node.h
    src/core/Connection.h
    src/core/NodeGraph.h
    src/nodes/InputNode.h
    src/nodes/OutputNode.h
    src/nodes/GizmoNode.h
    src/nodes/GroupNode.h
    src/nodes/PositionTweak.h
    src/nodes/ScaleTweak.h
    src/nodes/RotationTweak.h
    src/nodes/ColorTweak.h
    src/nodes/TimeShiftNode.h
    src/nodes/SurfaceFactoryNode.h
)

qt_add_library(GizmoTweakLib2 STATIC
    ${LIB_SOURCES}
    ${LIB_HEADERS}
)

qt_add_qml_module(GizmoTweakLib2
    URI GizmoTweakLib2
    VERSION 1.0
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/GizmoTweakLib2
    SOURCES
        ${LIB_HEADERS}
)

target_include_directories(GizmoTweakLib2
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/nodes
)

target_link_libraries(GizmoTweakLib2
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
)

# Suffixe 'd' pour la version debug
set_target_properties(GizmoTweakLib2 PROPERTIES
    DEBUG_POSTFIX "d"
)
