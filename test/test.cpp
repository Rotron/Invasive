#include "test.h"
#include "../client/ringbuffer.h"
#include "../client/frame.h"

Test::Test()
{
}

void Test::testRingBuffer()
{
    RingBuffer<int> ring(10);

    for (int i = 0; i < 10; ++i) {
        ring.push_back(i);
    }

    for (int i = 0; i < 10; ++i) {
        QCOMPARE(ring[i], i);
    }

    for (int i = 0; i < 5; ++i) {
        ring.push_back(i);
    }

    for (int i = 0; i < 5; ++i) {
        QCOMPARE(ring[i], i + 5);
    }

    for (int i = 5; i < 10; ++i) {
        QCOMPARE(ring[i], i - 5);
    }

    QCOMPARE(ring.sum(), 45);
    QCOMPARE(ring.average(), 4);
}

void Test::testFrame()
{
    const unsigned char packet[] = {
        0x94, 0xa2, 0x62, 0xb4, 0x96, 0x98, 0x60, 0x94, 0xa2, 0x62,
        0xb4, 0x96, 0x96, 0x61, 0x03, 0xf0, 0x63, 0x2d, 0x70, 0x2d,
        0x67, 0x2d, 0x6d, 0x72, 0x72, 0x20, 0x0d, 0x0a, 0xed, 0x5d
    };

    FramePtr frame = Frame::create(QDateTime::currentDateTime(),
                                   QByteArray(reinterpret_cast<const char*>(packet), sizeof(packet)));
    QVERIFY(static_cast<bool>(frame));
    QVERIFY(frame->isValid());
    QCOMPARE(frame->info(), QByteArray("c-p-g-mrr \r\n"));
    QCOMPARE(frame->sha1(), QString("c36dbfb71a691d4e8498a0f1877c7694a5cf7930"));

    QList<Frame::Address> addrs = frame->addresses();
    QCOMPARE(addrs[0].callsign, QString("JQ1ZKL"));
    QCOMPARE(addrs[0].repeated, false);
    QCOMPARE(addrs[0].ssid,     0);
    QCOMPARE(addrs[1].callsign, QString("JQ1ZKK"));
    QCOMPARE(addrs[1].repeated, false);
    QCOMPARE(addrs[1].ssid,     0);

}
