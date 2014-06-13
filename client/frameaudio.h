#pragma once
#include "stdafx.h"

class FrameAudio
{
public:
    FrameAudio(int sample_rate, const QDateTime& start_time, const QVector<double>& data);

    int sampleRate() const;
    QDateTime startTime() const;
    size_t count() const;
    double duration() const;
    QVector<double>::const_iterator begin() const;
    QVector<double>::const_iterator end() const;

private:
    int sample_rate_;
    QDateTime start_time_;
    QVector<double> audio_buffer_;

};
