#include "frameprinter.h"
#include "frame.h"

namespace FramePrinter {

QString toPlainText(const FramePtr& frame)
{
    QString text;
    text += QString("=").repeated(8) + "\n";
    text += QString("%0 [%1 bytes]\n")
            .arg(frame->datetime().toString(Qt::ISODate))
            .arg(frame->info().size());

    QStringList addresses;
    for (const Frame::Address& addr : frame->addresses()) {
        QString addr_str = QString("%0/%1").arg(addr.callsign).arg(addr.ssid);
        if (addr.repeated) addr_str += "R";
        addresses += addr_str;
    }
    text += QString("%0\n\n").arg(addresses.join(" < "));
    text += QString("%0\n\n").arg(frame->printableInfo());
    text += frame->info().toHex() + "\n";

    return text;
}

QString toHtmlText(const FramePtr& frame)
{
    QString html;
    html += "<font color=\"gray\"><h4>";
    html += QString("%0 [%1 bytes]")
            .arg(frame->datetime().toString(Qt::ISODate))
            .arg(frame->info().size());

    QStringList addresses;
    for (const Frame::Address& addr : frame->addresses()) {
        QString addr_str = QString("%0/%1").arg(addr.callsign).arg(addr.ssid);
        if (addr.repeated) addr_str += "R";
        addresses += addr_str;
    }
    html += QString("<br>%0").arg(addresses.join(" < "));
    html += "</h4></font>";

    html += "<div><br><font color=\"#dd0000\">" + QString("%0").arg(frame->printableInfo()) + "<br></font></div>";
    html += "<div>" + frame->info().toHex() + "</div>";
    html += "<hr>";

    return html;
}

}
