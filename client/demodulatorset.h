#pragma once
#include "stdafx.h"
#include "forward.h"

class DemodulatorSet : public QObject, public QRunnable
{
    Q_OBJECT
public:
    typedef QList<DemodulatorFactoryPtr> DemodulatorFactoryList;
    explicit DemodulatorSet(DemodulatorFactoryList& factories, const FrameAudioPtr& frame_audio, QObject *parent = 0);
    void run();

signals:
    void decoded(FramePtr);

private:
    QList<DemodulatorFactoryPtr>& factories_;
    FrameAudioPtr frame_audio_;

};
