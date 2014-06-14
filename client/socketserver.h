#pragma once
#include "forward.h"
#include "stdafx.h"

class SocketServer : public QObject
{
    Q_OBJECT
public:
    explicit SocketServer(quint16 port, QObject *parent = 0);
    ~SocketServer();

public slots:
    void writeFrame(const FramePtr& frame);

private slots:
    void newConnectionSlot();
    void disconnectedSlot(QObject* object);

private:
    QSignalMapper* signal_mapper_;
    QTcpServer* tcp_server_;
    QSet<QTcpSocket*> sockets_;

};
