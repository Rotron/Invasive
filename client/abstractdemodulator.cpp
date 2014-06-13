#include "abstractdemodulator.h"
#include "frameaudio.h"

AbstractDemodulator::AbstractDemodulator(const FrameAudioPtr& frame_audio, QObject *parent) :
    QObject(parent),
    frame_audio_(frame_audio)
{
    setAutoDelete(true);
}

AbstractDemodulator::~AbstractDemodulator()
{

}

void AbstractDemodulator::run()
{
    if (frame_audio_) {
        if (FramePtr frame = exec(*frame_audio_)) {
            emit decorded(frame);
        }
    }
}
