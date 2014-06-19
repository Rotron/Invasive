#include "frameprinter.h"
#include "frame.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace FramePrinter {

QString asciiInfo(const FramePtr& frame)
{
    QByteArray text;
    foreach (char c, frame->info()) {
        // ascii
        if (c >= 0x20 && c <= 0x7e) {
            text += c;
        }
        else {
            text += ".";
        }
    }
    return QString::fromUtf8(text);
}

QString toPlainText(const FramePtr& frame)
{
    QString text;
    text += QString("=").repeated(8) + "\n";
    text += QString("%0 [%1 bytes]%2\n")
            .arg(frame->datetime().toString(Qt::ISODate))
            .arg(frame->info().size())
            .arg(frame->validFcs() ? "" : " BROKEN");

    QStringList addresses;
    for (const Frame::Address& addr : frame->addresses()) {
        QString addr_str = QString("%0/%1").arg(addr.callsign).arg(addr.ssid);
        if (addr.repeated) addr_str += "R";
        addresses += addr_str;
    }
    text += QString("%0\n\n").arg(addresses.join(" < "));
    text += QString("%0\n\n").arg(FramePrinter::asciiInfo(frame));
    text += frame->info().toHex() + "\n";

    return text;
}

QString toHtmlText(const FramePtr& frame)
{
    QString html;
    html += "<font color=\"gray\"><h4>";
    html += QString("%0 [%1 bytes]%2")
            .arg(frame->datetime().toString(Qt::ISODate))
            .arg(frame->info().size())
            .arg(frame->validFcs() ? "" : " <font color=\"red\">BROKEN</font>");

    QStringList addresses;
    for (const Frame::Address& addr : frame->addresses()) {
        QString addr_str = QString("%0/%1").arg(addr.callsign).arg(addr.ssid);
        if (addr.repeated) addr_str += "R";
        addresses += addr_str;
    }
    html += QString("<br>%0").arg(addresses.join(QString(" < ").toHtmlEscaped()));
    html += "</h4></font>";

    html += "<div><br><font color=\"#dd0000\">" + QString("%0").arg(FramePrinter::asciiInfo(frame)) + "<br></font></div>";
    html += "<div>" + frame->info().toHex() + "</div>";
    html += "<hr>";

    return html;
}

QString toJson(const FramePtr& frame)
{
    QJsonObject obj;
    obj["datetime"] = frame->datetime().toString(Qt::ISODate);
    obj["info-ascii"] = asciiInfo(frame);
    obj["info-hex"] = QString::fromUtf8(frame->info().toHex());
    obj["broken"] = !frame->validFcs();

    QJsonArray addrs;
    for (const Frame::Address& addr : frame->addresses()) {
        QJsonObject addrobj;
        addrobj["callsign"] = addr.callsign;
        addrobj["repeated"] = addr.repeated;
        addrobj["ssid"] = addr.ssid;
        addrs.push_back(addrobj);
    }
    obj["addresses"] = addrs;

    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

}
