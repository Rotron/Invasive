#include "pulseaudio.h"

PulseAudio::PulseAudio(const QAudioFormat& format, const QString& device, QObject *parent) :
    QObject(parent),
    format_(format)
{
    const pa_sample_spec ss = {
        PA_SAMPLE_S16LE,
        format.sampleRate(),
        format.channelCount()
    };

    int error;
    if (!(audio_ = pa_simple_new(NULL, "Invasive", PA_STREAM_RECORD, device.toUtf8().data(), "record", &ss, NULL, NULL, &error))) {
        qDebug() << pa_strerror(error);
    }

    int interval = 1000.0 / (format.bytesForDuration(1000000) / sizeof(buffer_));
    connect(&timer_, SIGNAL(timeout()), this, SLOT(readData()));
    timer_.start(interval);
}

PulseAudio::~PulseAudio()
{
    pa_simple_free(audio_);
}

QAudioFormat PulseAudio::format() const
{
    return format_;
}

void PulseAudio::readData()
{
    int error;
    if (pa_simple_read(audio_, buffer_, sizeof(buffer_), &error) == 0) {
        emit received(QByteArray(buffer_, sizeof(buffer_)));
    }
}
