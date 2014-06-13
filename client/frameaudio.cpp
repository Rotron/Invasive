#include "frameaudio.h"

FrameAudio::FrameAudio(int sample_rate, const QDateTime& start_time, const QVector<double>& data) :
    sample_rate_(sample_rate),
    start_time_(start_time),
    audio_buffer_(data)
{

}

int FrameAudio::sampleRate() const
{
    return sample_rate_;
}

QDateTime FrameAudio::startTime() const
{
    return start_time_;
}

size_t FrameAudio::count() const
{
    return audio_buffer_.size();
}

double FrameAudio::duration() const
{
    return 1.0 * audio_buffer_.size() / sample_rate_;
}

QVector<double>::const_iterator FrameAudio::begin() const
{
   return audio_buffer_.begin();
}

QVector<double>::const_iterator FrameAudio::end() const
{
    return audio_buffer_.end();
}
