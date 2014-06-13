#pragma once
#include "forward.h"
#include "stdafx.h"

class AbstractFrameDetector : public QObject
{
    Q_OBJECT
public:
    AbstractFrameDetector(QObject *parent = 0);
    virtual ~AbstractFrameDetector();

signals:
    void detected(FrameAudioPtr);

public slots:
    virtual void processAudio(const QByteArray& audio) = 0;

};
