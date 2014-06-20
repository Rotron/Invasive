#pragma once
#include "forward.h"
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QList>

class Frame
{
public:
    struct Address {
        QString callsign;
        int ssid;
        bool repeated;
    };

public:
    static FramePtr create(const QDateTime& datetime, const QByteArray& data);
    QDateTime datetime() const;
    QString sha1() const;
    QByteArray info() const;
    QList<Address> addresses() const;
    QByteArray data() const;
    uint16_t fcs() const;
    uint16_t actualFcs() const;
    bool isValid() const;

private:
    Frame(const QDateTime& datetime);

private:
    bool decode(const QByteArray& data);

private:
    QDateTime datetime_;
    QString sha1_;
    QByteArray info_;
    QList<Address> addresses_;
    QByteArray data_;
    uint16_t fcs_;
    uint16_t actual_fcs_;

};
