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

namespace {
QString addr2str(const Frame::Address& addr) {
    return QString("%0/%1%2")
            .arg(addr.callsign)
            .arg(addr.ssid)
            .arg(addr.repeated ? "R" : "");
}
}

QString toPlainText(const FramePtr& frame)
{
    QString text;
    text += QString("=").repeated(8) + "\n";
    text += QString("%0 [%1 bytes]%2\n")
            .arg(frame->datetime().toString(Qt::ISODate))
            .arg(frame->info().size())
            .arg(frame->isValid() ? "" : " Broken");

    QString addresses;
    if (frame->addresses().size() >= 2) {
        addresses += addr2str(frame->addresses()[1]);
        addresses += " > ";
        if (frame->addresses().size() >= 3) {
            QStringList repeaters;
            for (int i = 2; i < frame->addresses().size(); ++i) {
                repeaters.push_back(addr2str(frame->addresses()[i]));
            }
            addresses += repeaters.join(", ");
            addresses += " > ";
        }
        addresses += addr2str(frame->addresses()[0]);
    }

    text += QString("%0\n\n").arg(addresses);
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
            .arg(frame->isValid() ? "" : " <font color=\"red\">Broken</font>");

    QString addresses;
    if (frame->addresses().size() >= 2) {
        addresses += addr2str(frame->addresses()[1]);
        addresses += " > ";
        if (frame->addresses().size() >= 3) {
            QStringList repeaters;
            for (int i = 2; i < frame->addresses().size(); ++i) {
                repeaters.push_back(addr2str(frame->addresses()[i]));
            }
            addresses += repeaters.join(", ");
            addresses += " > ";
        }
        addresses += addr2str(frame->addresses()[0]);
    }

    html += QString("<br>%0").arg(addresses.toHtmlEscaped());
    html += "</h4></font>";

    html += "<div><br><font color=\"#dd0000\">" + FramePrinter::asciiInfo(frame).toHtmlEscaped() + "<br></font></div>";
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
    obj["broken"] = !frame->isValid();

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
