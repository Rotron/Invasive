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
    Frame(const QDateTime& datetime, const QByteArray& data);
    bool isValid() const;
    QDateTime datetime() const;
    QString sha1() const;
    QByteArray info() const;
    QString printableInfo() const;
    QList<Address> addresses() const;
    static bool verify(const QByteArray& data);

private:
    bool decode(const QByteArray& data);

private:
    bool valid_;
    QDateTime datetime_;
    QString sha1_;
    QByteArray info_;
    QList<Address> addresses_;

};
