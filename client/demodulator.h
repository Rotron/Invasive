#pragma once
#include "stdafx.h"
#include "abstractdemodulator.h"

class Demodulator : public DemodulatorInterface
{
public:
    struct Settings {
        bool verfy_fcs;
        std::function<double(int,int)> window;
    };

public:
    explicit Demodulator(const Settings& settings);
    FramePtr decode(const FrameAudio& frame_audio);

private:
    void process(bool verify_fcs, double center, const QVector<double>& diff, FramePtr* result);

private:
    Settings settings_;

};

class DefaultDemodulatorFactory : public DemodulatorFactoryInterface
{
public:
    DefaultDemodulatorFactory(const Demodulator::Settings& settings);
    DemodulatorInterface* make() const;

private:
    Demodulator::Settings settings_;

};
