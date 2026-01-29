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

# Test Node Formulas (mathematical transformations)
add_executable(tst_node_formulas
    tst_node_formulas.cpp
)

target_link_libraries(tst_node_formulas
    PRIVATE
        GizmoTweakLib2
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

add_test(NAME NodeFormulasTests COMMAND tst_node_formulas)

# Test Node Persistence (JSON roundtrip)
add_executable(tst_node_persistence
    tst_node_persistence.cpp
)

target_link_libraries(tst_node_persistence
    PRIVATE
        GizmoTweakLib2
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

add_test(NAME NodePersistenceTests COMMAND tst_node_persistence)

# Test GraphEvaluator
add_executable(tst_graph_evaluator
    tst_graph_evaluator.cpp
)

target_link_libraries(tst_graph_evaluator
    PRIVATE
        GizmoTweakLib2
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

add_test(NAME GraphEvaluatorTests COMMAND tst_graph_evaluator)

# Test Commands (Undo/Redo)
add_executable(tst_commands
    tst_commands.cpp
)

target_link_libraries(tst_commands
    PRIVATE
        GizmoTweakLib2
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

add_test(NAME CommandsTests COMMAND tst_commands)

# Test Automation (KeyFrame, AutomationTrack)
add_executable(tst_automation
    tst_automation.cpp
)

target_link_libraries(tst_automation
    PRIVATE
        GizmoTweakLib2
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

add_test(NAME AutomationTests COMMAND tst_automation)

# Test Zoom (zoom-to-cursor math)
add_executable(tst_zoom
    tst_zoom.cpp
)

target_link_libraries(tst_zoom
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(NAME ZoomTests COMMAND tst_zoom)

# Test Port & Connection edge cases
add_executable(tst_port_connection
    tst_port_connection.cpp
)

target_link_libraries(tst_port_connection
    PRIVATE
        GizmoTweakLib2
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

add_test(NAME PortConnectionTests COMMAND tst_port_connection)

# Test evaluateUpTo (partial graph evaluation)
add_executable(tst_evaluate_up_to
    tst_evaluate_up_to.cpp
)

target_link_libraries(tst_evaluate_up_to
    PRIVATE
        GizmoTweakLib2
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

add_test(NAME EvaluateUpToTests COMMAND tst_evaluate_up_to)
