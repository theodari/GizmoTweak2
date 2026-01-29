#include <QtTest>
#include <QSignalSpy>
#include "core/Node.h"
#include "core/Port.h"
#include "core/Connection.h"
#include "core/NodeGraph.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/GizmoNode.h"
#include "nodes/GroupNode.h"
#include "nodes/PositionTweak.h"
#include "nodes/ScaleTweak.h"

using namespace gizmotweak2;

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

class TestPortConnection : public QObject
{
    Q_OBJECT

private slots:
    // Port tests
    void testPortProperties();
    void testPortConnectedState();
    void testPortConnectedSignal();
    void testPortScenePosition();
    void testPortVisibility();
    void testPortRequired();
    void testPortSatisfiedSignal();
    void testCanConnectSameDirection();
    void testCanConnectSameNode();
    void testCanConnectIncompatibleTypes();
    void testCanConnectFrameToFrame();
    void testCanConnectRatio2DToRatio2D();
    void testCanConnectRatio1DToRatio1D();
    void testCanConnectRatio2DToRatioAny();
    void testCanConnectRatio1DToRatioAny();
    void testCanConnectFrameToRatioAny();
    void testEffectiveDataTypeUnconnected();
    void testEffectiveDataTypeConnected();

    // Connection tests
    void testConnectionProperties();
    void testConnectionIsValid();
    void testConnectionIsValidNullPorts();
    void testConnectionIsValidSamePort();

    // Graph connection edge cases
    void testConnectNullPorts();
    void testConnectAlreadyConnectedInput();
    void testDisconnectPort();
    void testConnectionForPort();
    void testConnectionForPortNull();
    void testRemoveNodeCleansConnections();
    void testConnectionCountProperty();

    // RatioAny resolution
    void testRatioAnyPortResolution();
};

// --- Port tests ---

void TestPortConnection::testPortProperties()
{
    TestNode node(Port::DataType::Frame, Port::DataType::Ratio2D);

    auto* inPort = node.inputAt(0);
    QVERIFY(inPort != nullptr);
    QCOMPARE(inPort->name(), QStringLiteral("in"));
    QCOMPARE(inPort->direction(), Port::Direction::In);
    QCOMPARE(inPort->dataType(), Port::DataType::Frame);
    QCOMPARE(inPort->node(), &node);
    QCOMPARE(inPort->index(), 0);

    auto* outPort = node.outputAt(0);
    QVERIFY(outPort != nullptr);
    QCOMPARE(outPort->name(), QStringLiteral("out"));
    QCOMPARE(outPort->direction(), Port::Direction::Out);
    QCOMPARE(outPort->dataType(), Port::DataType::Ratio2D);
    QCOMPARE(outPort->node(), &node);
    QCOMPARE(outPort->index(), 0);
}

void TestPortConnection::testPortConnectedState()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeB = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);

    auto* outPort = nodeA->outputAt(0);
    auto* inPort = nodeB->inputAt(0);

    QVERIFY(!outPort->isConnected());
    QVERIFY(!inPort->isConnected());

    auto* conn = graph.connect(outPort, inPort);
    QVERIFY(conn != nullptr);
    QVERIFY(outPort->isConnected());
    QVERIFY(inPort->isConnected());

    graph.disconnect(conn);
    QVERIFY(!outPort->isConnected());
    QVERIFY(!inPort->isConnected());
}

void TestPortConnection::testPortConnectedSignal()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeB = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);

    auto* outPort = nodeA->outputAt(0);
    auto* inPort = nodeB->inputAt(0);

    QSignalSpy spyOut(outPort, &Port::connectedChanged);
    QSignalSpy spyIn(inPort, &Port::connectedChanged);

    auto* conn = graph.connect(outPort, inPort);
    QCOMPARE(spyOut.count(), 1);
    QCOMPARE(spyIn.count(), 1);

    graph.disconnect(conn);
    QCOMPARE(spyOut.count(), 2);
    QCOMPARE(spyIn.count(), 2);
}

void TestPortConnection::testPortScenePosition()
{
    TestNode node(Port::DataType::Frame, Port::DataType::Frame);
    auto* port = node.inputAt(0);

    QSignalSpy spy(port, &Port::scenePositionChanged);

    QPointF pos(100.0, 200.0);
    port->setScenePosition(pos);
    QCOMPARE(port->scenePosition(), pos);
    QCOMPARE(spy.count(), 1);
}

void TestPortConnection::testPortVisibility()
{
    TestNode node(Port::DataType::Frame, Port::DataType::Frame);
    auto* port = node.inputAt(0);

    QVERIFY(port->isVisible());

    QSignalSpy spy(port, &Port::visibleChanged);

    port->setVisible(false);
    QVERIFY(!port->isVisible());
    QCOMPARE(spy.count(), 1);
}

void TestPortConnection::testPortRequired()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeB = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);

    auto* inPort = nodeB->inputAt(0);

    QVERIFY(!inPort->isRequired());

    inPort->setRequired(true);
    QVERIFY(inPort->isRequired());
    QVERIFY(!inPort->isSatisfied());

    auto* conn = graph.connect(nodeA->outputAt(0), inPort);
    Q_UNUSED(conn)
    QVERIFY(inPort->isSatisfied());
}

void TestPortConnection::testPortSatisfiedSignal()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeB = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);

    auto* inPort = nodeB->inputAt(0);
    inPort->setRequired(true);

    QSignalSpy spy(inPort, &Port::satisfiedChanged);

    auto* conn = graph.connect(nodeA->outputAt(0), inPort);
    QVERIFY(spy.count() >= 1);

    graph.disconnect(conn);
    QVERIFY(spy.count() >= 2);
}

void TestPortConnection::testCanConnectSameDirection()
{
    TestNode nodeA(Port::DataType::Frame, Port::DataType::Frame);
    TestNode nodeB(Port::DataType::Frame, Port::DataType::Frame);

    auto* outA = nodeA.outputAt(0);
    auto* outB = nodeB.outputAt(0);

    QVERIFY(!outA->canConnectTo(outB));
}

void TestPortConnection::testCanConnectSameNode()
{
    TestNode node(Port::DataType::Frame, Port::DataType::Frame);

    auto* out = node.outputAt(0);
    auto* in = node.inputAt(0);

    QVERIFY(!out->canConnectTo(in));
}

void TestPortConnection::testCanConnectIncompatibleTypes()
{
    TestNode nodeA(Port::DataType::Frame, Port::DataType::Frame);
    TestNode nodeB(Port::DataType::Ratio2D, Port::DataType::Ratio2D);

    auto* outA = nodeA.outputAt(0);
    auto* inB = nodeB.inputAt(0);

    QVERIFY(!outA->canConnectTo(inB));
}

void TestPortConnection::testCanConnectFrameToFrame()
{
    TestNode nodeA(Port::DataType::Frame, Port::DataType::Frame);
    TestNode nodeB(Port::DataType::Frame, Port::DataType::Frame);

    QVERIFY(nodeA.outputAt(0)->canConnectTo(nodeB.inputAt(0)));
}

void TestPortConnection::testCanConnectRatio2DToRatio2D()
{
    TestNode nodeA(Port::DataType::Ratio2D, Port::DataType::Ratio2D);
    TestNode nodeB(Port::DataType::Ratio2D, Port::DataType::Ratio2D);

    QVERIFY(nodeA.outputAt(0)->canConnectTo(nodeB.inputAt(0)));
}

void TestPortConnection::testCanConnectRatio1DToRatio1D()
{
    TestNode nodeA(Port::DataType::Ratio1D, Port::DataType::Ratio1D);
    TestNode nodeB(Port::DataType::Ratio1D, Port::DataType::Ratio1D);

    QVERIFY(nodeA.outputAt(0)->canConnectTo(nodeB.inputAt(0)));
}

void TestPortConnection::testCanConnectRatio2DToRatioAny()
{
    TestNode nodeA(Port::DataType::RatioAny, Port::DataType::Ratio2D);
    TestNode nodeB(Port::DataType::RatioAny, Port::DataType::RatioAny);

    QVERIFY(nodeA.outputAt(0)->canConnectTo(nodeB.inputAt(0)));
}

void TestPortConnection::testCanConnectRatio1DToRatioAny()
{
    TestNode nodeA(Port::DataType::RatioAny, Port::DataType::Ratio1D);
    TestNode nodeB(Port::DataType::RatioAny, Port::DataType::RatioAny);

    QVERIFY(nodeA.outputAt(0)->canConnectTo(nodeB.inputAt(0)));
}

void TestPortConnection::testCanConnectFrameToRatioAny()
{
    TestNode nodeA(Port::DataType::Frame, Port::DataType::Frame);
    TestNode nodeB(Port::DataType::RatioAny, Port::DataType::RatioAny);

    QVERIFY(!nodeA.outputAt(0)->canConnectTo(nodeB.inputAt(0)));
}

void TestPortConnection::testEffectiveDataTypeUnconnected()
{
    TestNode node(Port::DataType::RatioAny, Port::DataType::RatioAny);
    auto* port = node.inputAt(0);

    QCOMPARE(port->effectiveDataType(), Port::DataType::RatioAny);
}

void TestPortConnection::testEffectiveDataTypeConnected()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Ratio2D);
    auto* nodeB = new TestNode(Port::DataType::RatioAny, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);

    auto* outPort = nodeA->outputAt(0);
    auto* inPort = nodeB->inputAt(0);

    auto* conn = graph.connect(outPort, inPort);
    QVERIFY(conn != nullptr);
    QCOMPARE(inPort->effectiveDataType(), Port::DataType::Ratio2D);
}

// --- Connection tests ---

void TestPortConnection::testConnectionProperties()
{
    TestNode nodeA(Port::DataType::Frame, Port::DataType::Frame);
    TestNode nodeB(Port::DataType::Frame, Port::DataType::Frame);

    auto* source = nodeA.outputAt(0);
    auto* target = nodeB.inputAt(0);

    Connection conn(source, target);

    QVERIFY(!conn.uuid().isEmpty());
    QCOMPARE(conn.sourcePort(), source);
    QCOMPARE(conn.targetPort(), target);
}

void TestPortConnection::testConnectionIsValid()
{
    TestNode nodeA(Port::DataType::Frame, Port::DataType::Frame);
    TestNode nodeB(Port::DataType::Frame, Port::DataType::Frame);

    auto* source = nodeA.outputAt(0);
    auto* target = nodeB.inputAt(0);

    QVERIFY(Connection::isValid(source, target));
}

void TestPortConnection::testConnectionIsValidNullPorts()
{
    TestNode node(Port::DataType::Frame, Port::DataType::Frame);
    auto* port = node.inputAt(0);

    QVERIFY(!Connection::isValid(nullptr, port));
    QVERIFY(!Connection::isValid(port, nullptr));
    QVERIFY(!Connection::isValid(nullptr, nullptr));
}

void TestPortConnection::testConnectionIsValidSamePort()
{
    TestNode node(Port::DataType::Frame, Port::DataType::Frame);
    auto* port = node.outputAt(0);

    QVERIFY(!Connection::isValid(port, port));
}

// --- Graph connection edge cases ---

void TestPortConnection::testConnectNullPorts()
{
    NodeGraph graph;

    auto* node = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(node);

    auto* port = node->inputAt(0);

    QCOMPARE(graph.connect(nullptr, port), nullptr);
    QCOMPARE(graph.connect(port, nullptr), nullptr);
    QCOMPARE(graph.connect(nullptr, nullptr), nullptr);
}

void TestPortConnection::testConnectAlreadyConnectedInput()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeB = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeC = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);
    graph.addNode(nodeC);

    auto* inPort = nodeC->inputAt(0);

    // Connect A -> C
    auto* conn1 = graph.connect(nodeA->outputAt(0), inPort);
    QVERIFY(conn1 != nullptr);
    QCOMPARE(graph.connectionCount(), 1);

    // Connect B -> C (should replace A -> C)
    auto* conn2 = graph.connect(nodeB->outputAt(0), inPort);
    QVERIFY(conn2 != nullptr);
    QCOMPARE(graph.connectionCount(), 1);

    // Verify the new connection is B -> C
    auto* current = graph.connectionForPort(inPort);
    QVERIFY(current != nullptr);
    QCOMPARE(current->sourcePort(), nodeB->outputAt(0));
}

void TestPortConnection::testDisconnectPort()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeB = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);

    graph.connect(nodeA->outputAt(0), nodeB->inputAt(0));
    QCOMPARE(graph.connectionCount(), 1);

    graph.disconnectPort(nodeB->inputAt(0));
    QCOMPARE(graph.connectionCount(), 0);
}

void TestPortConnection::testConnectionForPort()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeB = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);

    auto* conn = graph.connect(nodeA->outputAt(0), nodeB->inputAt(0));

    QCOMPARE(graph.connectionForPort(nodeA->outputAt(0)), conn);
    QCOMPARE(graph.connectionForPort(nodeB->inputAt(0)), conn);
}

void TestPortConnection::testConnectionForPortNull()
{
    NodeGraph graph;
    QCOMPARE(graph.connectionForPort(nullptr), nullptr);
}

void TestPortConnection::testRemoveNodeCleansConnections()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeB = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeC = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);
    graph.addNode(nodeC);

    // A -> B -> C
    graph.connect(nodeA->outputAt(0), nodeB->inputAt(0));
    graph.connect(nodeB->outputAt(0), nodeC->inputAt(0));
    QCOMPARE(graph.connectionCount(), 2);

    // Remove B: both connections should be removed
    auto bUuid = nodeB->uuid();
    graph.removeNode(bUuid);
    QCOMPARE(graph.connectionCount(), 0);
}

void TestPortConnection::testConnectionCountProperty()
{
    NodeGraph graph;

    auto* nodeA = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    auto* nodeB = new TestNode(Port::DataType::Frame, Port::DataType::Frame);
    graph.addNode(nodeA);
    graph.addNode(nodeB);

    QCOMPARE(graph.connectionCount(), 0);
    QCOMPARE(graph.connections().size(), 0);

    auto* conn = graph.connect(nodeA->outputAt(0), nodeB->inputAt(0));
    QCOMPARE(graph.connectionCount(), 1);
    QCOMPARE(graph.connections().size(), 1);

    graph.disconnect(conn);
    QCOMPARE(graph.connectionCount(), 0);
    QCOMPARE(graph.connections().size(), 0);
}

// --- RatioAny resolution ---

void TestPortConnection::testRatioAnyPortResolution()
{
    NodeGraph graph;

    auto* gizmo = new GizmoNode();
    auto* posTweak = new PositionTweak();
    graph.addNode(gizmo);
    graph.addNode(posTweak);

    // GizmoNode outputs Ratio2D on port 0
    auto* gizmoOut = gizmo->outputAt(0);
    QCOMPARE(gizmoOut->dataType(), Port::DataType::Ratio2D);

    // PositionTweak has RatioAny input at index 1 (index 0 is Frame)
    auto* ratioIn = posTweak->inputAt(1);
    QCOMPARE(ratioIn->dataType(), Port::DataType::RatioAny);
    QCOMPARE(ratioIn->effectiveDataType(), Port::DataType::RatioAny);

    // Connect Gizmo -> PositionTweak ratio input
    auto* conn = graph.connect(gizmoOut, ratioIn);
    QVERIFY(conn != nullptr);
    QCOMPARE(ratioIn->effectiveDataType(), Port::DataType::Ratio2D);

    // Disconnect: should revert to RatioAny
    graph.disconnect(conn);
    QCOMPARE(ratioIn->effectiveDataType(), Port::DataType::RatioAny);
}

QTEST_MAIN(TestPortConnection)
#include "tst_port_connection.moc"
