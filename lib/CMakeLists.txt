project(GizmoTweakLib2)

# Set Qt policy to suppress QTP0001 warning about QML resource prefix
qt_policy(SET QTP0001 NEW)

# ExcaliburEngine dependency
set(EXCALIBUR_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ExcaliburEngine")

set(LIB_SOURCES
    src/gizmotweaklib2.cpp
    src/core/Port.cpp
    src/core/Node.cpp
    src/core/Connection.cpp
    src/core/NodeGraph.cpp
    src/core/Commands.cpp
    src/core/GraphEvaluator.cpp
    src/automation/KeyFrame.cpp
    src/automation/AutomationTrack.cpp
    src/nodes/InputNode.cpp
    src/nodes/OutputNode.cpp
    src/nodes/GizmoNode.cpp
    src/nodes/GroupNode.cpp
    src/nodes/PositionTweak.cpp
    src/nodes/ScaleTweak.cpp
    src/nodes/RotationTweak.cpp
    src/nodes/ColorTweak.cpp
    src/nodes/PolarTweak.cpp
    src/nodes/SparkleTweak.cpp
    src/nodes/FuzzynessTweak.cpp
    src/nodes/ColorFuzzynessTweak.cpp
    src/nodes/SplitTweak.cpp
    src/nodes/RounderTweak.cpp
    src/nodes/WaveTweak.cpp
    src/nodes/SqueezeTweak.cpp
    src/nodes/TimeShiftNode.cpp
    src/nodes/SurfaceFactoryNode.cpp
    src/nodes/MirrorNode.cpp
)

set(LIB_HEADERS
    src/gizmotweaklib2.h
    src/core/Port.h
    src/core/Node.h
    src/core/Connection.h
    src/core/NodeGraph.h
    src/core/Commands.h
    src/core/GraphEvaluator.h
    src/automation/Param.h
    src/automation/KeyFrame.h
    src/automation/AutomationTrack.h
    src/nodes/InputNode.h
    src/nodes/OutputNode.h
    src/nodes/GizmoNode.h
    src/nodes/GroupNode.h
    src/nodes/PositionTweak.h
    src/nodes/ScaleTweak.h
    src/nodes/RotationTweak.h
    src/nodes/ColorTweak.h
    src/nodes/PolarTweak.h
    src/nodes/SparkleTweak.h
    src/nodes/FuzzynessTweak.h
    src/nodes/ColorFuzzynessTweak.h
    src/nodes/SplitTweak.h
    src/nodes/RounderTweak.h
    src/nodes/WaveTweak.h
    src/nodes/SqueezeTweak.h
    src/nodes/TimeShiftNode.h
    src/nodes/SurfaceFactoryNode.h
    src/nodes/MirrorNode.h
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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
        ${CMAKE_CURRENT_SOURCE_DIR}/src/automation
        ${EXCALIBUR_ENGINE_DIR}
)

# ExcaliburEngine library path (Qt version from find_package)
set(XENGINE_QT_VERSION "${Qt6_VERSION}")
set(XENGINE_BUILD_DIR "${EXCALIBUR_ENGINE_DIR}/bin/Qt_${XENGINE_QT_VERSION}_MSVC2019_64bit")

# Debug/Release suffix
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(XENGINE_LIB_NAME "excalibur_engined")
else()
    set(XENGINE_LIB_NAME "excalibur_engine")
endif()

target_link_libraries(GizmoTweakLib2
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        ${XENGINE_BUILD_DIR}/${XENGINE_LIB_NAME}.lib
)

# Suffixe 'd' pour la version debug
set_target_properties(GizmoTweakLib2 PROPERTIES
    DEBUG_POSTFIX "d"
)
