project(GizmoTweak2Tests)

# Test version
add_executable(tst_gizmotweaklib2
    tst_gizmotweaklib2.cpp
)

target_link_libraries(tst_gizmotweaklib2
    PRIVATE
        GizmoTweakLib2
        Qt6::Core
        Qt6::Test
)

add_test(NAME GizmoTweakLib2Tests COMMAND tst_gizmotweaklib2)

# Test NodeGraph
add_executable(tst_nodegraph
    tst_nodegraph.cpp
)

target_link_libraries(tst_nodegraph
    PRIVATE
        GizmoTweakLib2
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

add_test(NAME NodeGraphTests COMMAND tst_nodegraph)
