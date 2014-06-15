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
    bool process(double center, const QVector<double>& diff, FramePtr* result);

};

class DefaultDemodulatorFactory : public DemodulatorFactoryInterface
{
public:
    AbstractDemodulator* make(const FrameAudioPtr& frame_audio) const;

};
