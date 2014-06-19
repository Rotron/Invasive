#pragma once
#include "stdafx.h"
#include "abstractdemodulator.h"

class DefaultDemodulator : public AbstractDemodulator
{
    Q_OBJECT
public:
    explicit DefaultDemodulator(const FrameAudioPtr& frame_audio);

protected:
    FramePtr exec(const FrameAudio& frame_audio);

private:
    void process(bool verify_fcs, double center, const QVector<double>& diff, FramePtr* result);

};

class DefaultDemodulatorFactory : public DemodulatorFactoryInterface
{
public:
    AbstractDemodulator* make(const FrameAudioPtr& frame_audio) const;

};
