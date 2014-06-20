#pragma once
#include "forward.h"
#include "stdafx.h"

class DemodulatorInterface
{
public:
    virtual FramePtr decode(const FrameAudio& frame_audio) = 0;

};

class DemodulatorFactoryInterface
{
public:
    virtual DemodulatorInterface* make() const = 0;
};
