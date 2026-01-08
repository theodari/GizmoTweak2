cmake_minimum_required(VERSION 3.16)

project(GizmoTweak2
    VERSION 0.1.0
    DESCRIPTION "GizmoTweak2 - Node-based laser composition"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 6 obligatoire
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2 QuickDialogs2 Test)

# Options de build
option(BUILD_TESTS "Build tests" ON)

# Sous-projets
add_subdirectory(lib)
add_subdirectory(app)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
