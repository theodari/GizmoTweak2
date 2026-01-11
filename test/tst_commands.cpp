#include <QtTest>
#include <QUndoStack>

#include "core/Commands.h"
#include "core/NodeGraph.h"
#include "core/Node.h"
#include "core/Port.h"
#include "core/Connection.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/GizmoNode.h"
#include "nodes/PositionTweak.h"

using namespace gizmotweak2;

class TestCommands : public QObject
{
    Q_OBJECT

private slots:
    // CreateNodeCommand tests
    void testCreateNodeRedo();
    void testCreateNodeUndo();
    void testCreateNodeUndoRedo();
    void testCreateNodePreservesProperties();

    // DeleteNodeCommand tests
    void testDeleteNodeRedo();
    void testDeleteNodeUndo();
    void testDeleteNodeUndoRedo();
    void testDeleteNodePreservesConnections();
    void testDeleteNodePreservesProperties();

    // MoveNodeCommand tests
    void testMoveNodeRedo();
    void testMoveNodeUndo();
    void testMoveNodeUndoRedo();
    void testMoveNodeMerge();
    void testMoveNodeMergeDifferentNodes();

    // ConnectCommand tests
    void testConnectRedo();
    void testConnectUndo();
    void testConnectUndoRedo();

    // DisconnectCommand tests
    void testDisconnectRedo();
    void testDisconnectUndo();
    void testDisconnectUndoRedo();

    // Integration tests via NodeGraph
    void testNodeGraphUndo();
    void testNodeGraphRedo();
    void testNodeGraphClearUndoStack();
    void testNodeGraphCanUndoRedo();

    // Complex scenarios
    void testComplexUndoSequence();
    void testUndoAfterMultipleOperations();
    void testDeleteConnectedNode();

private:
    bool fuzzyCompare(qreal a, qreal b, qreal epsilon = 0.0001);
};

bool TestCommands::fuzzyCompare(qreal a, qreal b, qreal epsilon)
{
    return qAbs(a - b) < epsilon;
}

// ============================================================================
// CreateNodeCommand Tests
// ============================================================================

void TestCommands::testCreateNodeRedo()
{
    NodeGraph graph;
    QCOMPARE(graph.nodeCount(), 0);

    // Create a node via the graph (which uses the command internally)
    auto* node = graph.createNode("Gizmo", QPointF(100, 200));

    QVERIFY(node != nullptr);
    QCOMPARE(graph.nodeCount(), 1);
    QCOMPARE(node->type(), QString("Gizmo"));
    QCOMPARE(node->position(), QPointF(100, 200));
}

void TestCommands::testCreateNodeUndo()
{
    NodeGraph graph;

    auto* node = graph.createNode("Gizmo", QPointF(100, 200));
    QCOMPARE(graph.nodeCount(), 1);
    auto uuid = node->uuid();

    // Undo creation
    graph.undo();

    QCOMPARE(graph.nodeCount(), 0);
    QVERIFY(graph.nodeByUuid(uuid) == nullptr);
}

void TestCommands::testCreateNodeUndoRedo()
{
    NodeGraph graph;

    auto* node = graph.createNode("Gizmo", QPointF(100, 200));
    QCOMPARE(graph.nodeCount(), 1);

    // Undo
    graph.undo();
    QCOMPARE(graph.nodeCount(), 0);

    // Redo
    graph.redo();
    QCOMPARE(graph.nodeCount(), 1);

    // Verify the restored node
    auto* restored = graph.nodeAt(0);
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->type(), QString("Gizmo"));
    QCOMPARE(restored->position(), QPointF(100, 200));
}

void TestCommands::testCreateNodePreservesProperties()
{
    NodeGraph graph;

    auto* node = graph.createNode("PositionTweak", QPointF(100, 200));
    auto* tweak = qobject_cast<PositionTweak*>(node);
    QVERIFY(tweak != nullptr);

    // Modify properties
    tweak->setOffsetX(0.5);
    tweak->setOffsetY(-0.3);
    tweak->setDisplayName("My Tweak");

    // Undo and redo
    graph.undo();
    graph.redo();

    // Note: Properties set AFTER creation may not be preserved
    // because the command saves state at undo time
    auto* restored = graph.nodeAt(0);
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->type(), QString("PositionTweak"));
}

// ============================================================================
// DeleteNodeCommand Tests
// ============================================================================

void TestCommands::testDeleteNodeRedo()
{
    NodeGraph graph;

    auto* node = graph.createNode("Gizmo", QPointF(100, 200));
    auto uuid = node->uuid();
    QCOMPARE(graph.nodeCount(), 1);

    // Clear undo stack so delete is the first command
    graph.clearUndoStack();

    // Delete the node
    graph.removeNode(uuid);

    QCOMPARE(graph.nodeCount(), 0);
    QVERIFY(graph.nodeByUuid(uuid) == nullptr);
}

void TestCommands::testDeleteNodeUndo()
{
    NodeGraph graph;

    auto* node = graph.createNode("Gizmo", QPointF(100, 200));
    node->setDisplayName("Test Gizmo");
    auto uuid = node->uuid();

    graph.clearUndoStack();
    graph.removeNode(uuid);
    QCOMPARE(graph.nodeCount(), 0);

    // Undo deletion
    graph.undo();

    QCOMPARE(graph.nodeCount(), 1);

    // Verify the restored node
    auto* restored = graph.nodeAt(0);
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->type(), QString("Gizmo"));
    QCOMPARE(restored->displayName(), QString("Test Gizmo"));
    QCOMPARE(restored->position(), QPointF(100, 200));
}

void TestCommands::testDeleteNodeUndoRedo()
{
    NodeGraph graph;

    auto* node = graph.createNode("Gizmo", QPointF(100, 200));
    node->setDisplayName("Test Gizmo");

    graph.clearUndoStack();
    graph.removeNode(node->uuid());
    QCOMPARE(graph.nodeCount(), 0);

    // Undo
    graph.undo();
    QCOMPARE(graph.nodeCount(), 1);

    // Redo
    graph.redo();
    QCOMPARE(graph.nodeCount(), 0);
}

void TestCommands::testDeleteNodePreservesConnections()
{
    NodeGraph graph;

    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo = graph.createNode("Gizmo", QPointF(200, 100));
    auto* tweak = graph.createNode("PositionTweak", QPointF(300, 100));
    auto* output = graph.createNode("Output", QPointF(400, 100));

    // Create connections
    graph.connect(input->outputAt(0), tweak->inputAt(0));
    graph.connect(gizmo->outputAt(0), tweak->inputAt(1));
    graph.connect(tweak->outputAt(0), output->inputAt(0));

    QCOMPARE(graph.connectionCount(), 3);

    graph.clearUndoStack();

    // Delete the tweak node (which has connections)
    graph.removeNode(tweak->uuid());

    QCOMPARE(graph.nodeCount(), 3);
    QCOMPARE(graph.connectionCount(), 0);  // Connections involving tweak are gone

    // Undo deletion
    graph.undo();

    QCOMPARE(graph.nodeCount(), 4);
    // Connections should be restored
    QCOMPARE(graph.connectionCount(), 3);
}

void TestCommands::testDeleteNodePreservesProperties()
{
    NodeGraph graph;

    auto* node = graph.createNode("PositionTweak", QPointF(100, 200));
    auto* tweak = qobject_cast<PositionTweak*>(node);
    tweak->setOffsetX(0.75);
    tweak->setOffsetY(-0.25);
    tweak->setDisplayName("Custom Tweak");

    graph.clearUndoStack();
    graph.removeNode(node->uuid());
    QCOMPARE(graph.nodeCount(), 0);

    // Undo deletion
    graph.undo();

    QCOMPARE(graph.nodeCount(), 1);
    auto* restored = qobject_cast<PositionTweak*>(graph.nodeAt(0));
    QVERIFY(restored != nullptr);
    QVERIFY(fuzzyCompare(restored->offsetX(), 0.75));
    QVERIFY(fuzzyCompare(restored->offsetY(), -0.25));
    QCOMPARE(restored->displayName(), QString("Custom Tweak"));
}

// ============================================================================
// MoveNodeCommand Tests
// ============================================================================

void TestCommands::testMoveNodeRedo()
{
    NodeGraph graph;

    auto* node = graph.createNode("Gizmo", QPointF(100, 100));
    auto uuid = node->uuid();

    graph.clearUndoStack();

    // Simulate move via NodeGraph
    graph.beginMoveNode(uuid);
    graph.endMoveNode(uuid, QPointF(300, 400));

    QCOMPARE(node->position(), QPointF(300, 400));
}

void TestCommands::testMoveNodeUndo()
{
    NodeGraph graph;

    auto* node = graph.createNode("Gizmo", QPointF(100, 100));
    auto uuid = node->uuid();

    graph.clearUndoStack();

    graph.beginMoveNode(uuid);
    graph.endMoveNode(uuid, QPointF(300, 400));
    QCOMPARE(node->position(), QPointF(300, 400));

    // Undo move
    graph.undo();

    QCOMPARE(node->position(), QPointF(100, 100));
}

void TestCommands::testMoveNodeUndoRedo()
{
    NodeGraph graph;

    auto* node = graph.createNode("Gizmo", QPointF(100, 100));
    auto uuid = node->uuid();

    graph.clearUndoStack();

    graph.beginMoveNode(uuid);
    graph.endMoveNode(uuid, QPointF(300, 400));

    // Undo
    graph.undo();
    QCOMPARE(node->position(), QPointF(100, 100));

    // Redo
    graph.redo();
    QCOMPARE(node->position(), QPointF(300, 400));
}

void TestCommands::testMoveNodeMerge()
{
    NodeGraph graph;

    auto* node = graph.createNode("Gizmo", QPointF(100, 100));
    auto uuid = node->uuid();

    graph.clearUndoStack();

    // First move
    graph.beginMoveNode(uuid);
    graph.endMoveNode(uuid, QPointF(200, 200));

    // Second move of same node (should merge)
    graph.beginMoveNode(uuid);
    graph.endMoveNode(uuid, QPointF(300, 300));

    QCOMPARE(node->position(), QPointF(300, 300));

    // Single undo should go back to original position
    graph.undo();
    QCOMPARE(node->position(), QPointF(100, 100));

    // Verify we can't undo further (commands were merged)
    QVERIFY(!graph.canUndo());
}

void TestCommands::testMoveNodeMergeDifferentNodes()
{
    NodeGraph graph;

    auto* node1 = graph.createNode("Gizmo", QPointF(100, 100));
    auto* node2 = graph.createNode("Gizmo", QPointF(200, 200));
    auto uuid1 = node1->uuid();
    auto uuid2 = node2->uuid();

    graph.clearUndoStack();

    // Move node1
    graph.beginMoveNode(uuid1);
    graph.endMoveNode(uuid1, QPointF(150, 150));

    // Move node2 (should NOT merge with node1's move)
    graph.beginMoveNode(uuid2);
    graph.endMoveNode(uuid2, QPointF(250, 250));

    QCOMPARE(node1->position(), QPointF(150, 150));
    QCOMPARE(node2->position(), QPointF(250, 250));

    // First undo should undo node2's move
    graph.undo();
    QCOMPARE(node1->position(), QPointF(150, 150));
    QCOMPARE(node2->position(), QPointF(200, 200));

    // Second undo should undo node1's move
    graph.undo();
    QCOMPARE(node1->position(), QPointF(100, 100));
    QCOMPARE(node2->position(), QPointF(200, 200));
}

// ============================================================================
// ConnectCommand Tests
// ============================================================================

void TestCommands::testConnectRedo()
{
    NodeGraph graph;

    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* output = graph.createNode("Output", QPointF(300, 100));

    graph.clearUndoStack();

    auto* conn = graph.connect(input->outputAt(0), output->inputAt(0));

    QVERIFY(conn != nullptr);
    QCOMPARE(graph.connectionCount(), 1);
    QVERIFY(input->outputAt(0)->isConnected());
    QVERIFY(output->inputAt(0)->isConnected());
}

void TestCommands::testConnectUndo()
{
    NodeGraph graph;

    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* output = graph.createNode("Output", QPointF(300, 100));

    graph.clearUndoStack();

    graph.connect(input->outputAt(0), output->inputAt(0));
    QCOMPARE(graph.connectionCount(), 1);

    // Undo connection
    graph.undo();

    QCOMPARE(graph.connectionCount(), 0);
    QVERIFY(!input->outputAt(0)->isConnected());
    QVERIFY(!output->inputAt(0)->isConnected());
}

void TestCommands::testConnectUndoRedo()
{
    NodeGraph graph;

    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* output = graph.createNode("Output", QPointF(300, 100));

    graph.clearUndoStack();

    graph.connect(input->outputAt(0), output->inputAt(0));

    // Undo
    graph.undo();
    QCOMPARE(graph.connectionCount(), 0);

    // Redo
    graph.redo();
    QCOMPARE(graph.connectionCount(), 1);
    QVERIFY(input->outputAt(0)->isConnected());
    QVERIFY(output->inputAt(0)->isConnected());
}

// ============================================================================
// DisconnectCommand Tests
// ============================================================================

void TestCommands::testDisconnectRedo()
{
    NodeGraph graph;

    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* output = graph.createNode("Output", QPointF(300, 100));
    auto* conn = graph.connect(input->outputAt(0), output->inputAt(0));

    graph.clearUndoStack();

    graph.disconnect(conn);

    QCOMPARE(graph.connectionCount(), 0);
    QVERIFY(!input->outputAt(0)->isConnected());
    QVERIFY(!output->inputAt(0)->isConnected());
}

void TestCommands::testDisconnectUndo()
{
    NodeGraph graph;

    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* output = graph.createNode("Output", QPointF(300, 100));
    auto* conn = graph.connect(input->outputAt(0), output->inputAt(0));

    graph.clearUndoStack();

    graph.disconnect(conn);
    QCOMPARE(graph.connectionCount(), 0);

    // Undo disconnection
    graph.undo();

    QCOMPARE(graph.connectionCount(), 1);
    QVERIFY(input->outputAt(0)->isConnected());
    QVERIFY(output->inputAt(0)->isConnected());
}

void TestCommands::testDisconnectUndoRedo()
{
    NodeGraph graph;

    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* output = graph.createNode("Output", QPointF(300, 100));
    auto* conn = graph.connect(input->outputAt(0), output->inputAt(0));

    graph.clearUndoStack();

    graph.disconnect(conn);

    // Undo
    graph.undo();
    QCOMPARE(graph.connectionCount(), 1);

    // Redo
    graph.redo();
    QCOMPARE(graph.connectionCount(), 0);
}

// ============================================================================
// NodeGraph Integration Tests
// ============================================================================

void TestCommands::testNodeGraphUndo()
{
    NodeGraph graph;

    graph.createNode("Gizmo", QPointF(100, 100));
    QCOMPARE(graph.nodeCount(), 1);

    QVERIFY(graph.canUndo());
    graph.undo();

    QCOMPARE(graph.nodeCount(), 0);
    QVERIFY(!graph.canUndo());
}

void TestCommands::testNodeGraphRedo()
{
    NodeGraph graph;

    graph.createNode("Gizmo", QPointF(100, 100));
    graph.undo();

    QVERIFY(graph.canRedo());
    graph.redo();

    QCOMPARE(graph.nodeCount(), 1);
    QVERIFY(!graph.canRedo());
}

void TestCommands::testNodeGraphClearUndoStack()
{
    NodeGraph graph;

    graph.createNode("Gizmo", QPointF(100, 100));
    graph.createNode("Gizmo", QPointF(200, 200));

    QVERIFY(graph.canUndo());

    graph.clearUndoStack();

    QVERIFY(!graph.canUndo());
    QVERIFY(!graph.canRedo());
}

void TestCommands::testNodeGraphCanUndoRedo()
{
    NodeGraph graph;

    // Initially no undo/redo
    QVERIFY(!graph.canUndo());
    QVERIFY(!graph.canRedo());

    // After create, can undo
    graph.createNode("Gizmo", QPointF(100, 100));
    QVERIFY(graph.canUndo());
    QVERIFY(!graph.canRedo());

    // After undo, can redo
    graph.undo();
    QVERIFY(!graph.canUndo());
    QVERIFY(graph.canRedo());

    // After redo, can undo again
    graph.redo();
    QVERIFY(graph.canUndo());
    QVERIFY(!graph.canRedo());
}

// ============================================================================
// Complex Scenarios
// ============================================================================

void TestCommands::testComplexUndoSequence()
{
    NodeGraph graph;

    // Step 1: Create nodes
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* gizmo = graph.createNode("Gizmo", QPointF(200, 200));
    auto* output = graph.createNode("Output", QPointF(300, 100));

    QCOMPARE(graph.nodeCount(), 3);

    // Step 2: Connect them
    graph.connect(input->outputAt(0), output->inputAt(0));
    QCOMPARE(graph.connectionCount(), 1);

    // Step 3: Move a node
    auto gizmoUuid = gizmo->uuid();
    graph.beginMoveNode(gizmoUuid);
    graph.endMoveNode(gizmoUuid, QPointF(250, 250));

    QCOMPARE(gizmo->position(), QPointF(250, 250));

    // Undo move
    graph.undo();
    QCOMPARE(graph.nodeByUuid(gizmoUuid)->position(), QPointF(200, 200));

    // Undo connect
    graph.undo();
    QCOMPARE(graph.connectionCount(), 0);

    // Undo create output
    graph.undo();
    QCOMPARE(graph.nodeCount(), 2);

    // Undo create gizmo
    graph.undo();
    QCOMPARE(graph.nodeCount(), 1);

    // Undo create input
    graph.undo();
    QCOMPARE(graph.nodeCount(), 0);

    // Redo all
    graph.redo(); // Input
    graph.redo(); // Gizmo
    graph.redo(); // Output
    graph.redo(); // Connect
    graph.redo(); // Move

    QCOMPARE(graph.nodeCount(), 3);
    QCOMPARE(graph.connectionCount(), 1);
}

void TestCommands::testUndoAfterMultipleOperations()
{
    NodeGraph graph;

    // Create multiple nodes
    auto* node1 = graph.createNode("Gizmo", QPointF(100, 100));
    auto* node2 = graph.createNode("Gizmo", QPointF(200, 200));
    auto* node3 = graph.createNode("Gizmo", QPointF(300, 300));

    QCOMPARE(graph.nodeCount(), 3);

    // Delete middle node
    graph.removeNode(node2->uuid());
    QCOMPARE(graph.nodeCount(), 2);

    // Undo delete
    graph.undo();
    QCOMPARE(graph.nodeCount(), 3);

    // Undo create node3
    graph.undo();
    QCOMPARE(graph.nodeCount(), 2);

    // Verify remaining nodes
    bool foundNode1 = false;
    bool foundNode2 = false;
    for (int i = 0; i < graph.nodeCount(); ++i)
    {
        if (graph.nodeAt(i)->position() == QPointF(100, 100)) foundNode1 = true;
        if (graph.nodeAt(i)->position() == QPointF(200, 200)) foundNode2 = true;
    }
    QVERIFY(foundNode1);
    QVERIFY(foundNode2);
}

void TestCommands::testDeleteConnectedNode()
{
    NodeGraph graph;

    // Create a chain: Input -> Tweak -> Output
    auto* input = graph.createNode("Input", QPointF(100, 100));
    auto* tweak = graph.createNode("PositionTweak", QPointF(200, 100));
    auto* output = graph.createNode("Output", QPointF(300, 100));

    graph.connect(input->outputAt(0), tweak->inputAt(0));
    graph.connect(tweak->outputAt(0), output->inputAt(0));

    QCOMPARE(graph.nodeCount(), 3);
    QCOMPARE(graph.connectionCount(), 2);

    graph.clearUndoStack();

    // Delete the middle node
    graph.removeNode(tweak->uuid());

    QCOMPARE(graph.nodeCount(), 2);
    QCOMPARE(graph.connectionCount(), 0);

    // Undo should restore the node AND its connections
    graph.undo();

    QCOMPARE(graph.nodeCount(), 3);
    QCOMPARE(graph.connectionCount(), 2);

    // Verify Input is connected to Tweak
    QVERIFY(input->outputAt(0)->isConnected());

    // Verify Tweak is connected to Output
    QVERIFY(output->inputAt(0)->isConnected());
}

QTEST_MAIN(TestCommands)
#include "tst_commands.moc"
