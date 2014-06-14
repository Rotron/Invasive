#include "socketserver.h"
#include "frameprinter.h"

SocketServer::SocketServer(quint16 port, QObject *parent) :
    QObject(parent)
{
    signal_mapper_ = new QSignalMapper(this);
    connect(signal_mapper_, SIGNAL(mapped(QObject*)), this, SLOT(disconnectedSlot(QObject*)));

    tcp_server_ = new QTcpServer(this);
    connect(tcp_server_, SIGNAL(newConnection()), this, SLOT(newConnectionSlot()));

    tcp_server_->listen(QHostAddress::Any, port);
}

SocketServer::~SocketServer()
{
    tcp_server_->close();
}

void SocketServer::writeFrame(const FramePtr& frame)
{
    if (frame) {
        QByteArray json = FramePrinter::toJson(frame).toUtf8() + "\r\n";
        foreach (QTcpSocket* socket, sockets_) {
           socket->write(json);
        }
    }
}

void SocketServer::newConnectionSlot()
{
    if (QTcpSocket* socket = tcp_server_->nextPendingConnection()) {
        sockets_.insert(socket);
        connect(socket, SIGNAL(disconnected()), signal_mapper_, SLOT(map()));
        signal_mapper_->setMapping(socket, socket);
    }
}

void SocketServer::disconnectedSlot(QObject* object)
{
    if (QTcpSocket* socket = dynamic_cast<QTcpSocket*>(object)) {
        qDebug() << "disconnected";
        sockets_.remove(socket);
    }
}
