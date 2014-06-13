#pragma once

#include <QString>
#include <QtTest>

class Test : public QObject
{
    Q_OBJECT

public:
    Test();

private Q_SLOTS:
    void testRingBuffer();
    void testFrame();
};

