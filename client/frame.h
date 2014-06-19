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
    static FramePtr create(const QDateTime& datetime, const QByteArray& data, bool verify_fcs);
    QDateTime datetime() const;
    QString sha1() const;
    QByteArray info() const;
    QList<Address> addresses() const;
    bool validFcs() const;

private:
    Frame(const QDateTime& datetime);

private:
    bool decode(const QByteArray& data);

private:
    QDateTime datetime_;
    QString sha1_;
    QByteArray info_;
    QList<Address> addresses_;
    bool valid_fcs_;

};
