#pragma once
#include "stdafx.h"
#include "abstractdemodulator.h"

class DefaultDemodulator : public DemodulatorInterface
{
public:
    explicit DefaultDemodulator(bool verify_fcs = true);
    FramePtr decode(const FrameAudio& frame_audio);

private:
    void process(bool verify_fcs, double center, const QVector<double>& diff, FramePtr* result);

private:
    bool verify_fcs_;

};

class DefaultDemodulatorFactory : public DemodulatorFactoryInterface
{
public:
    DefaultDemodulatorFactory(bool verify_fcs = true);
    DemodulatorInterface* make() const;

private:
    bool verify_fcs_;

};
