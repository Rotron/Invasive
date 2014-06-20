#include "demodulatorset.h"
#include "abstractdemodulator.h"
#include "frameaudio.h"

DemodulatorSet::DemodulatorSet(DemodulatorFactoryList& factories, const FrameAudioPtr& frame_audio, QObject *parent) :
    QObject(parent),
    factories_(factories),
    frame_audio_(std::make_shared<FrameAudio>(*frame_audio))
{
    setAutoDelete(true);
}

void DemodulatorSet::run()
{
    for (const auto& f : factories_) {
        if (DemodulatorInterface *demodulator = f->make()) {
            if (FramePtr frame = demodulator->decode(*frame_audio_)) {
                emit decoded(frame);
                return;
            }
        }
    }
}
