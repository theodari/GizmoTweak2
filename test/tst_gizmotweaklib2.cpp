#include <QtTest>
#include "gizmotweaklib2.h"

class TestGizmoTweakLib2 : public QObject
{
    Q_OBJECT

private slots:
    void testVersion();
};

void TestGizmoTweakLib2::testVersion()
{
    auto v = gizmotweak2::version();
    QVERIFY(!v.isEmpty());
    QCOMPARE(v, QString("0.1.0"));
}

QTEST_MAIN(TestGizmoTweakLib2)
#include "tst_gizmotweaklib2.moc"
