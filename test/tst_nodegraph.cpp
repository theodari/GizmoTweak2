#include <QtTest>
#include <QSignalSpy>
#include "core/Node.h"
#include "core/Port.h"
#include "core/Connection.h"
#include "core/NodeGraph.h"
#include "core/GraphEvaluator.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/GizmoNode.h"
#include "nodes/GroupNode.h"
#include "nodes/MirrorNode.h"
#include "nodes/PositionTweak.h"
#include "nodes/ScaleTweak.h"
#include "nodes/RotationTweak.h"
#include "nodes/ColorTweak.h"
#include "nodes/TimeShiftNode.h"

using namespace gizmotweak2;

// Test node implementation
class TestNode : public Node
{
public:
    explicit TestNode(Port::DataType inputType, Port::DataType outputType, QObject* parent = nullptr)
        : Node(parent)
    {
        addInput("in", inputType);
        addOutput("out", outputType);
    }

    QString type() const override { return "TestNode"; }
    Category category() const override { return Category::Utility; }
};

class TestNodeGraph : public QObject
{
    Q_OBJECT

private slots:
    // Existing tests
    void testNodeCreation();
    void testNodeProperties();
    void testPortCreation();
    void testConnectionValid();
    void testConnectionInvalid();
    void testNodeGraphAddRemove();
    void testNodeGraphConnections();
    void testNodeGraphSelection();
    void testToJson();
    void testInputNode();
    void testOutputNode();
    void testInputOutputConnection();

    // Selection tests
    void testSelectAll();
    void testClearSelection();
    void testSelectionSignals();

    // Align/Distribute tests
    void testAlignLeft();
    void testAlignCenter();
    void testAlignRight();
    void testAlignTop();
    void testAlignMiddle();
    void testAlignBottom();
    void testDistributeHorizontal();
    void testDistributeVertical();
    void testAlignWithLessThanTwo();
    void testDistributeWithLessThanThree();

    // Clipboard tests
    void testCopyPaste();
    void testCutPaste();
    void testCopyDoesNotCopyIO();
    void testPasteWithoutCopy();
    void testDuplicateSelected();

    // Model interface tests
    void testRowCount();
    void testRoleNames();
    void testDataAccess();
    void testSetDataPosition();
    void testSetDataDisplayName();
    void testSetDataSelected();

    // Node factory tests
    void testCreateNode();
    void testCreateAllNodeTypes();
    void testCreateInvalidType();

    // Connection edge cases
    void testDisconnectPort();
    void testConnectionForPort();
    void testConnectionForPortUnconnected();
    void testRemoveNodeRemovesConnections();

    // Graph validation
    void testIsGraphCompleteEmpty();
    void testIsGraphCompleteMinimal();
    void testIsGraphIncompleteDisconnected();

    // Modified state
    void testModifiedOnCreate();
    void testSetClean();
    void testMarkAsModified();

    // Clear
    void testClear();

    // Signals
    void testNodeAddedSignal();
    void testNodeRemovedSignal();
    void testConnectionSignals();
};

// --- Existing tests ---

void TestNodeGraph::testNodeCreation()
{
    TestNode node(Port::DataType::Frame, Port::DataType::Frame);

    QVERIFY(!node.uuid().isEmpty());
    QCOMPARE(node.type(), QString("TestNode"));
    QCOMPARE(node.category(), Node::Category::Utility);
    QCOMPARE(node.inputCount(), 1);
    QCOMPARE(node.outputCount(), 1);
}

void TestNodeGraph::testNodeProperties()
{
    TestNode node(Port::DataType::Frame, Port::DataType::Frame);

    node.setDisplayName("Mon Node");
    QCOMPARE(node.displayName(), QString("Mon Node"));

    node.setPosition(QPointF(100, 200));
    QCOMPARE(node.position(), QPointF(100, 200));

    node.setSelected(true);
    QVERIFY(node.isSelected());
}

void TestNodeGraph::testPortCreation()
{
    TestNode node(Port::DataType::Frame, Port::DataType::Ratio2D);

    auto input = node.inputAt(0);
    QVERIFY(input != nullptr);
    QCOMPARE(input->name(), QString("in"));
    QCOMPARE(input->direction(), Port::Direction::In);
    QCOMPARE(input->dataType(), Port::DataType::Frame);
    QCOMPARE(input->node(), &node);

    auto output = node.outputAt(0);
    QVERIFY(output != nullptr);
    QCOMPARE(output->name(), QString("out"));
    QCOMPARE(output->direction(), Port::Direction::Out);
    QCOMPARE(output->dataType(), Port::DataType::Ratio2D);
}

void TestNodeGraph::testConnectionValid()
{
    TestNode node1(Port::DataType::Frame, Port::DataType::Frame);
    TestNode node2(Port::DataType::Frame, Port::DataType::Frame);

    auto out = node1.outputAt(0);
    auto in = node2.inputAt(0);

    QVERIFY(out->canConnectTo(in));
    QVERIFY(in->canConnectTo(out));

    Connection conn(out, in);
    QVERIFY(!conn.uuid().isEmpty());
    QCOMPARE(conn.sourcePort(), out);
    QCOMPARE(conn.targetPort(), in);
}

void TestNodeGraph::testConnectionInvalid()
{
    TestNode node1(Port::DataType::Frame, Port::DataType::Frame);
    TestNode node2(Port::DataType::Ratio2D, Port::DataType::Ratio2D);

    // Different data types
    QVERIFY(!node1.outputAt(0)->canConnectTo(node2.inputAt(0)));

    // Same direction
    TestNode node3(Port::DataType::Frame, Port::DataType::Frame);
    QVERIFY(!node1.outputAt(0)->canConnectTo(node3.outputAt(0)));

    // Same node
    QVERIFY(!node1.outputAt(0)->canConnectTo(node1.inputAt(0)));
}

void TestNodeGraph::testNodeGraphAddRemove()
{
    NodeGraph graph;

    auto node1 = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto node2 = new TestNode(Port::DataType::Frame, Port::DataType::Frame);

    graph.addNode(node1);
    QCOMPARE(graph.nodeCount(), 1);
    QCOMPARE(graph.rowCount(), 1);

    graph.addNode(node2);
    QCOMPARE(graph.nodeCount(), 2);

    auto uuid1 = node1->uuid();
    QCOMPARE(graph.nodeByUuid(uuid1), node1);

    graph.removeNode(uuid1);
    QCOMPARE(graph.nodeCount(), 1);
    QVERIFY(graph.nodeByUuid(uuid1) == nullptr);
}

void TestNodeGraph::testNodeGraphConnections()
{
    NodeGraph graph;

    auto node1 = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto node2 = new TestNode(Port::DataType::Frame, Port::DataType::Frame);

    graph.addNode(node1);
    graph.addNode(node2);

    auto conn = graph.connect(node1->outputAt(0), node2->inputAt(0));
    QVERIFY(conn != nullptr);
    QCOMPARE(graph.connectionCount(), 1);
    QVERIFY(node1->outputAt(0)->isConnected());
    QVERIFY(node2->inputAt(0)->isConnected());

    // Duplicate connection should fail
    auto conn2 = graph.connect(node1->outputAt(0), node2->inputAt(0));
    QVERIFY(conn2 == nullptr);
    QCOMPARE(graph.connectionCount(), 1);

    graph.disconnect(conn);
    QCOMPARE(graph.connectionCount(), 0);
    QVERIFY(!node1->outputAt(0)->isConnected());
    QVERIFY(!node2->inputAt(0)->isConnected());
}

void TestNodeGraph::testNodeGraphSelection()
{
    NodeGraph graph;

    auto node1 = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto node2 = new TestNode(Port::DataType::Frame, Port::DataType::Frame);

    graph.addNode(node1);
    graph.addNode(node2);

    node1->setSelected(true);
    node2->setSelected(true);

    auto selected = graph.selectedNodes();
    QCOMPARE(selected.size(), 2);

    graph.clearSelection();
    selected = graph.selectedNodes();
    QCOMPARE(selected.size(), 0);
}

void TestNodeGraph::testToJson()
{
    NodeGraph graph;

    auto node1 = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    node1->setDisplayName("Node 1");
    node1->setPosition(QPointF(100, 200));

    auto node2 = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    node2->setDisplayName("Node 2");
    node2->setPosition(QPointF(300, 200));

    graph.addNode(node1);
    graph.addNode(node2);
    graph.connect(node1->outputAt(0), node2->inputAt(0));

    auto json = graph.toJson();

    QCOMPARE(json["version"].toInt(), 1);
    QCOMPARE(json["nodes"].toArray().size(), 2);
    QCOMPARE(json["connections"].toArray().size(), 1);
}

void TestNodeGraph::testInputNode()
{
    InputNode input;

    QCOMPARE(input.type(), QString("Input"));
    QCOMPARE(input.category(), Node::Category::IO);
    QCOMPARE(input.displayName(), QString("Input"));

    // Input has no inputs, one Frame output
    QCOMPARE(input.inputCount(), 0);
    QCOMPARE(input.outputCount(), 1);

    auto output = input.outputAt(0);
    QVERIFY(output != nullptr);
    QCOMPARE(output->name(), QString("frame"));
    QCOMPARE(output->dataType(), Port::DataType::Frame);
    QCOMPARE(output->direction(), Port::Direction::Out);
}

void TestNodeGraph::testOutputNode()
{
    OutputNode output;

    QCOMPARE(output.type(), QString("Output"));
    QCOMPARE(output.category(), Node::Category::IO);
    QCOMPARE(output.displayName(), QString("Output"));

    // Output has one Frame input, no outputs
    QCOMPARE(output.inputCount(), 1);
    QCOMPARE(output.outputCount(), 0);

    auto input = output.inputAt(0);
    QVERIFY(input != nullptr);
    QCOMPARE(input->name(), QString("frame"));
    QCOMPARE(input->dataType(), Port::DataType::Frame);
    QCOMPARE(input->direction(), Port::Direction::In);
}

void TestNodeGraph::testInputOutputConnection()
{
    NodeGraph graph;

    auto input = new InputNode();
    auto output = new OutputNode();

    graph.addNode(input);
    graph.addNode(output);

    // Connect Input's output to Output's input
    auto conn = graph.connect(input->outputAt(0), output->inputAt(0));
    QVERIFY(conn != nullptr);
    QCOMPARE(graph.connectionCount(), 1);

    QVERIFY(input->outputAt(0)->isConnected());
    QVERIFY(output->inputAt(0)->isConnected());

    // Verify JSON serialization
    auto json = graph.toJson();
    QCOMPARE(json["nodes"].toArray().size(), 2);
    QCOMPARE(json["connections"].toArray().size(), 1);
}

// --- Selection tests ---

void TestNodeGraph::testSelectAll()
{
    NodeGraph graph;

    graph.createNode("PositionTweak", QPointF(0, 0));
    graph.createNode("ScaleTweak", QPointF(100, 0));
    graph.createNode("RotationTweak", QPointF(200, 0));

    graph.selectAll();

    QVERIFY(graph.hasSelection());
    auto selected = graph.selectedNodes();
    QCOMPARE(selected.size(), 3);
}

void TestNodeGraph::testClearSelection()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 0));
    auto n2 = graph.createNode("ScaleTweak", QPointF(100, 0));

    n1->setSelected(true);
    n2->setSelected(true);
    QCOMPARE(graph.selectedNodes().size(), 2);

    graph.clearSelection();

    QVERIFY(!graph.hasSelection());
    QCOMPARE(graph.selectedNodes().size(), 0);
}

void TestNodeGraph::testSelectionSignals()
{
    NodeGraph graph;
    QSignalSpy spy(&graph, &NodeGraph::hasSelectionChanged);

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 0));

    // Select
    n1->setSelected(true);
    // Deselect
    graph.clearSelection();

    QVERIFY(spy.count() >= 2);
}

// --- Align/Distribute tests ---

void TestNodeGraph::testAlignLeft()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(50, 0));
    auto n2 = graph.createNode("ScaleTweak", QPointF(200, 50));
    auto n3 = graph.createNode("RotationTweak", QPointF(350, 100));

    graph.selectAll();
    graph.alignSelected("left");

    QCOMPARE(n1->position().x(), 50.0);
    QCOMPARE(n2->position().x(), 50.0);
    QCOMPARE(n3->position().x(), 50.0);
}

void TestNodeGraph::testAlignCenter()
{
    NodeGraph graph;
    const double nodeWidth = 112.0;

    auto n1 = graph.createNode("PositionTweak", QPointF(50, 0));
    auto n2 = graph.createNode("ScaleTweak", QPointF(200, 50));
    auto n3 = graph.createNode("RotationTweak", QPointF(350, 100));

    graph.selectAll();
    graph.alignSelected("center");

    double minX = 50.0;
    double maxX = 350.0;
    double centerX = (minX + maxX + nodeWidth) / 2.0 - nodeWidth / 2.0;

    QCOMPARE(n1->position().x(), centerX);
    QCOMPARE(n2->position().x(), centerX);
    QCOMPARE(n3->position().x(), centerX);
}

void TestNodeGraph::testAlignRight()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(50, 0));
    auto n2 = graph.createNode("ScaleTweak", QPointF(200, 50));
    auto n3 = graph.createNode("RotationTweak", QPointF(350, 100));

    graph.selectAll();
    graph.alignSelected("right");

    QCOMPARE(n1->position().x(), 350.0);
    QCOMPARE(n2->position().x(), 350.0);
    QCOMPARE(n3->position().x(), 350.0);
}

void TestNodeGraph::testAlignTop()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 30));
    auto n2 = graph.createNode("ScaleTweak", QPointF(100, 150));
    auto n3 = graph.createNode("RotationTweak", QPointF(200, 280));

    graph.selectAll();
    graph.alignSelected("top");

    QCOMPARE(n1->position().y(), 30.0);
    QCOMPARE(n2->position().y(), 30.0);
    QCOMPARE(n3->position().y(), 30.0);
}

void TestNodeGraph::testAlignMiddle()
{
    NodeGraph graph;
    const double nodeHeight = 78.0;

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 30));
    auto n2 = graph.createNode("ScaleTweak", QPointF(100, 150));
    auto n3 = graph.createNode("RotationTweak", QPointF(200, 280));

    graph.selectAll();
    graph.alignSelected("middle");

    double minY = 30.0;
    double maxY = 280.0;
    double middleY = (minY + maxY + nodeHeight) / 2.0 - nodeHeight / 2.0;

    QCOMPARE(n1->position().y(), middleY);
    QCOMPARE(n2->position().y(), middleY);
    QCOMPARE(n3->position().y(), middleY);
}

void TestNodeGraph::testAlignBottom()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 30));
    auto n2 = graph.createNode("ScaleTweak", QPointF(100, 150));
    auto n3 = graph.createNode("RotationTweak", QPointF(200, 280));

    graph.selectAll();
    graph.alignSelected("bottom");

    QCOMPARE(n1->position().y(), 280.0);
    QCOMPARE(n2->position().y(), 280.0);
    QCOMPARE(n3->position().y(), 280.0);
}

void TestNodeGraph::testDistributeHorizontal()
{
    NodeGraph graph;
    const double nodeWidth = 112.0;

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 0));
    auto n2 = graph.createNode("ScaleTweak", QPointF(100, 0));
    auto n3 = graph.createNode("RotationTweak", QPointF(200, 0));
    auto n4 = graph.createNode("ColorTweak", QPointF(600, 0));

    graph.selectAll();
    graph.distributeSelected("horizontal");

    // First and last (by x) should stay in place
    // Nodes sorted by x: n1(0), n2(100), n3(200), n4(600)
    QCOMPARE(n1->position().x(), 0.0);
    QCOMPARE(n4->position().x(), 600.0);

    // Middle nodes should be evenly spaced
    double totalSpace = 600.0 - 0.0;
    double spacing = (totalSpace - 3 * nodeWidth) / 3.0;
    double expected2 = 0.0 + nodeWidth + spacing;
    double expected3 = expected2 + nodeWidth + spacing;

    QVERIFY(qAbs(n2->position().x() - expected2) < 1.0);
    QVERIFY(qAbs(n3->position().x() - expected3) < 1.0);
}

void TestNodeGraph::testDistributeVertical()
{
    NodeGraph graph;
    const double nodeHeight = 78.0;

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 0));
    auto n2 = graph.createNode("ScaleTweak", QPointF(0, 100));
    auto n3 = graph.createNode("RotationTweak", QPointF(0, 200));
    auto n4 = graph.createNode("ColorTweak", QPointF(0, 500));

    graph.selectAll();
    graph.distributeSelected("vertical");

    // First and last (by y) should stay in place
    QCOMPARE(n1->position().y(), 0.0);
    QCOMPARE(n4->position().y(), 500.0);

    // Middle nodes should be evenly spaced
    double totalSpace = 500.0 - 0.0;
    double spacing = (totalSpace - 3 * nodeHeight) / 3.0;
    double expected2 = 0.0 + nodeHeight + spacing;
    double expected3 = expected2 + nodeHeight + spacing;

    QVERIFY(qAbs(n2->position().y() - expected2) < 1.0);
    QVERIFY(qAbs(n3->position().y() - expected3) < 1.0);
}

void TestNodeGraph::testAlignWithLessThanTwo()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(100, 200));
    n1->setSelected(true);

    auto posBefore = n1->position();
    graph.alignSelected("left");

    QCOMPARE(n1->position(), posBefore);
}

void TestNodeGraph::testDistributeWithLessThanThree()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 0));
    auto n2 = graph.createNode("ScaleTweak", QPointF(300, 300));

    n1->setSelected(true);
    n2->setSelected(true);

    auto pos1Before = n1->position();
    auto pos2Before = n2->position();

    graph.distributeSelected("horizontal");

    QCOMPARE(n1->position(), pos1Before);
    QCOMPARE(n2->position(), pos2Before);
}

// --- Clipboard tests ---

void TestNodeGraph::testCopyPaste()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(100, 100));
    n1->setSelected(true);

    graph.copySelected();
    QVERIFY(graph.canPaste());

    int countBefore = graph.nodeCount();
    graph.pasteAtPosition(QPointF(200, 200));

    QCOMPARE(graph.nodeCount(), countBefore + 1);
}

void TestNodeGraph::testCutPaste()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(100, 100));
    n1->setSelected(true);

    int countBefore = graph.nodeCount();
    graph.cutSelected();

    QCOMPARE(graph.nodeCount(), countBefore - 1);
    QVERIFY(graph.canPaste());

    graph.pasteAtPosition(QPointF(200, 200));
    QCOMPARE(graph.nodeCount(), countBefore);
}

void TestNodeGraph::testCopyDoesNotCopyIO()
{
    NodeGraph graph;

    graph.createNode("Input", QPointF(0, 0));
    graph.createNode("Output", QPointF(300, 0));
    graph.createNode("PositionTweak", QPointF(150, 0));

    graph.selectAll();
    graph.copySelected();

    int countBefore = graph.nodeCount();
    graph.pasteAtPosition(QPointF(0, 200));

    // Only the PositionTweak should be pasted, not Input/Output
    QCOMPARE(graph.nodeCount(), countBefore + 1);
}

void TestNodeGraph::testPasteWithoutCopy()
{
    NodeGraph graph;

    QVERIFY(!graph.canPaste());

    int countBefore = graph.nodeCount();
    graph.pasteAtPosition(QPointF(100, 100));

    QCOMPARE(graph.nodeCount(), countBefore);
}

void TestNodeGraph::testDuplicateSelected()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(100, 100));
    n1->setSelected(true);

    int countBefore = graph.nodeCount();
    graph.duplicateSelected();

    QCOMPARE(graph.nodeCount(), countBefore + 1);
}

// --- Model interface tests ---

void TestNodeGraph::testRowCount()
{
    NodeGraph graph;

    QCOMPARE(graph.rowCount(), 0);

    graph.createNode("PositionTweak", QPointF(0, 0));
    QCOMPARE(graph.rowCount(), 1);

    graph.createNode("ScaleTweak", QPointF(100, 0));
    QCOMPARE(graph.rowCount(), 2);

    QCOMPARE(graph.rowCount(), graph.nodeCount());
}

void TestNodeGraph::testRoleNames()
{
    NodeGraph graph;
    auto roles = graph.roleNames();

    QVERIFY(roles.contains(NodeGraph::UuidRole));
    QVERIFY(roles.contains(NodeGraph::TypeRole));
    QVERIFY(roles.contains(NodeGraph::CategoryRole));
    QVERIFY(roles.contains(NodeGraph::PositionRole));
    QVERIFY(roles.contains(NodeGraph::DisplayNameRole));
    QVERIFY(roles.contains(NodeGraph::SelectedRole));
    QVERIFY(roles.contains(NodeGraph::NodeRole));
}

void TestNodeGraph::testDataAccess()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(50, 75));

    auto index = graph.index(0);
    auto nodeVar = graph.data(index, NodeGraph::NodeRole);
    auto node = qvariant_cast<Node*>(nodeVar);

    QVERIFY(node != nullptr);
    QCOMPARE(node, n1);
}

void TestNodeGraph::testSetDataPosition()
{
    NodeGraph graph;

    graph.createNode("PositionTweak", QPointF(0, 0));

    auto index = graph.index(0);
    graph.setData(index, QPointF(123, 456), NodeGraph::PositionRole);

    auto pos = graph.data(index, NodeGraph::PositionRole).toPointF();
    QCOMPARE(pos, QPointF(123, 456));
}

void TestNodeGraph::testSetDataDisplayName()
{
    NodeGraph graph;

    graph.createNode("PositionTweak", QPointF(0, 0));

    auto index = graph.index(0);
    graph.setData(index, "Custom Name", NodeGraph::DisplayNameRole);

    auto name = graph.data(index, NodeGraph::DisplayNameRole).toString();
    QCOMPARE(name, QString("Custom Name"));
}

void TestNodeGraph::testSetDataSelected()
{
    NodeGraph graph;

    graph.createNode("PositionTweak", QPointF(0, 0));

    auto index = graph.index(0);
    graph.setData(index, true, NodeGraph::SelectedRole);

    auto selected = graph.data(index, NodeGraph::SelectedRole).toBool();
    QVERIFY(selected);
}

// --- Node factory tests ---

void TestNodeGraph::testCreateNode()
{
    NodeGraph graph;

    auto node = graph.createNode("PositionTweak", QPointF(150, 250));

    QVERIFY(node != nullptr);
    QCOMPARE(node->type(), QString("PositionTweak"));
    QCOMPARE(node->position(), QPointF(150, 250));
}

void TestNodeGraph::testCreateAllNodeTypes()
{
    NodeGraph graph;

    auto types = graph.availableNodeTypes();
    QVERIFY(!types.isEmpty());

    for (const auto& typeName : types)
    {
        auto node = graph.createNode(typeName, QPointF(0, 0));
        QVERIFY2(node != nullptr, qPrintable(QString("Failed to create node of type: %1").arg(typeName)));
    }
}

void TestNodeGraph::testCreateInvalidType()
{
    NodeGraph graph;

    auto node = graph.createNode("InvalidType", QPointF(0, 0));
    QVERIFY(node == nullptr);
}

// --- Connection edge cases ---

void TestNodeGraph::testDisconnectPort()
{
    NodeGraph graph;

    auto n1 = graph.createNode("Input", QPointF(0, 0));
    auto n2 = graph.createNode("PositionTweak", QPointF(200, 0));

    graph.connect(n1->outputAt(0), n2->inputAt(0));
    QCOMPARE(graph.connectionCount(), 1);

    graph.disconnectPort(n2->inputAt(0));
    QCOMPARE(graph.connectionCount(), 0);
}

void TestNodeGraph::testConnectionForPort()
{
    NodeGraph graph;

    auto n1 = graph.createNode("Input", QPointF(0, 0));
    auto n2 = graph.createNode("PositionTweak", QPointF(200, 0));

    auto conn = graph.connect(n1->outputAt(0), n2->inputAt(0));
    QVERIFY(conn != nullptr);

    auto found = graph.connectionForPort(n2->inputAt(0));
    QCOMPARE(found, conn);
}

void TestNodeGraph::testConnectionForPortUnconnected()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 0));

    auto found = graph.connectionForPort(n1->inputAt(0));
    QVERIFY(found == nullptr);
}

void TestNodeGraph::testRemoveNodeRemovesConnections()
{
    NodeGraph graph;

    auto n1 = graph.createNode("Input", QPointF(0, 0));
    auto n2 = graph.createNode("PositionTweak", QPointF(200, 0));
    auto n3 = graph.createNode("Output", QPointF(400, 0));

    graph.connect(n1->outputAt(0), n2->inputAt(0));
    graph.connect(n2->outputAt(0), n3->inputAt(0));
    QCOMPARE(graph.connectionCount(), 2);

    graph.removeNode(n2->uuid());
    QCOMPARE(graph.connectionCount(), 0);
}

// --- Graph validation ---

void TestNodeGraph::testIsGraphCompleteEmpty()
{
    NodeGraph graph;

    QVERIFY(!graph.isGraphComplete());
}

void TestNodeGraph::testIsGraphCompleteMinimal()
{
    NodeGraph graph;

    auto input = graph.createNode("Input", QPointF(0, 0));
    auto output = graph.createNode("Output", QPointF(300, 0));

    graph.connect(input->outputAt(0), output->inputAt(0));

    QVERIFY(graph.isGraphComplete());
}

void TestNodeGraph::testIsGraphIncompleteDisconnected()
{
    NodeGraph graph;

    graph.createNode("Input", QPointF(0, 0));
    graph.createNode("Output", QPointF(300, 0));

    QVERIFY(!graph.isGraphComplete());
}

// --- Modified state ---

void TestNodeGraph::testModifiedOnCreate()
{
    NodeGraph graph;

    graph.createNode("PositionTweak", QPointF(0, 0));

    QVERIFY(graph.isModified());
}

void TestNodeGraph::testSetClean()
{
    NodeGraph graph;

    graph.createNode("PositionTweak", QPointF(0, 0));
    QVERIFY(graph.isModified());

    graph.setClean();
    QVERIFY(!graph.isModified());
}

void TestNodeGraph::testMarkAsModified()
{
    NodeGraph graph;

    graph.createNode("PositionTweak", QPointF(0, 0));
    graph.setClean();
    QVERIFY(!graph.isModified());

    graph.markAsModified();
    QVERIFY(graph.isModified());
}

// --- Clear ---

void TestNodeGraph::testClear()
{
    NodeGraph graph;

    auto n1 = graph.createNode("Input", QPointF(0, 0));
    auto n2 = graph.createNode("PositionTweak", QPointF(200, 0));
    auto n3 = graph.createNode("Output", QPointF(400, 0));

    graph.connect(n1->outputAt(0), n2->inputAt(0));
    graph.connect(n2->outputAt(0), n3->inputAt(0));

    QVERIFY(graph.nodeCount() > 0);
    QVERIFY(graph.connectionCount() > 0);

    graph.clear();

    QCOMPARE(graph.nodeCount(), 0);
    QCOMPARE(graph.connectionCount(), 0);
    QCOMPARE(graph.rowCount(), 0);
}

// --- Signals ---

void TestNodeGraph::testNodeAddedSignal()
{
    NodeGraph graph;
    QSignalSpy spy(&graph, &NodeGraph::nodeAdded);

    graph.createNode("PositionTweak", QPointF(0, 0));

    QCOMPARE(spy.count(), 1);
}

void TestNodeGraph::testNodeRemovedSignal()
{
    NodeGraph graph;

    auto n1 = graph.createNode("PositionTweak", QPointF(0, 0));
    auto uuid = n1->uuid();

    QSignalSpy spy(&graph, &NodeGraph::nodeRemoved);

    graph.removeNode(uuid);

    QCOMPARE(spy.count(), 1);
}

void TestNodeGraph::testConnectionSignals()
{
    NodeGraph graph;

    auto n1 = graph.createNode("Input", QPointF(0, 0));
    auto n2 = graph.createNode("PositionTweak", QPointF(200, 0));

    QSignalSpy addSpy(&graph, &NodeGraph::connectionAdded);
    QSignalSpy removeSpy(&graph, &NodeGraph::connectionRemoved);

    auto conn = graph.connect(n1->outputAt(0), n2->inputAt(0));
    QCOMPARE(addSpy.count(), 1);

    graph.disconnect(conn);
    QCOMPARE(removeSpy.count(), 1);
}

QTEST_MAIN(TestNodeGraph)
#include "tst_nodegraph.moc"
