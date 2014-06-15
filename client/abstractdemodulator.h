#pragma once
#include "forward.h"
#include "stdafx.h"

class AbstractDemodulator : public QObject, public QRunnable
{
    Q_OBJECT
public:
    AbstractDemodulator(const FrameAudioPtr& frame_audio, QObject *parent = 0);
    virtual ~AbstractDemodulator();
    void run();

signals:
    void decorded(FramePtr);

protected:
    virtual FramePtr exec(const FrameAudio& frame_audio) = 0;

protected:
    FrameAudioPtr frame_audio_;

};

class DemodulatorFactoryInterface
{
public:
    virtual AbstractDemodulator* make(const FrameAudioPtr& frame_audio) const = 0;
};
