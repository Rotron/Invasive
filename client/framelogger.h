#pragma once
#include "stdafx.h"
#include "forward.h"

class FrameLogger : public QObject
{
    Q_OBJECT
public:
    explicit FrameLogger(const QString& path, QObject *parent = 0);

public slots:
    void writeFrame(const QString& frame);

private:
    QFile file_;

};
