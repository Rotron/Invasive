#pragma once
#include <memory>

class FrameDetector;
class FrameListWidget;
class ConfigDialog;
class Modem;

class AbstractDemodulator;
class DemodulatorFactoryInterface;
typedef std::shared_ptr<DemodulatorFactoryInterface> DemodulatorFactoryPtr;

class AbstractFrameDetector;
typedef std::shared_ptr<AbstractFrameDetector> FrameDetectorPtr;

class Frame;
typedef std::shared_ptr<Frame> FramePtr;

class FrameAudio;
typedef std::shared_ptr<FrameAudio> FrameAudioPtr;
