#include <QtTest>
#include "core/Node.h"
#include "core/Port.h"
#include "core/Connection.h"
#include "core/NodeGraph.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"

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
};

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

    QCOMPARE(json["version"].toString(), QString("0.2.0"));
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

QTEST_MAIN(TestNodeGraph)
#include "tst_nodegraph.moc"
