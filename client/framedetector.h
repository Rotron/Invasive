#pragma once
#include "stdafx.h"
#include "firfilter.h"
#include "forward.h"
#include "abstractframedetector.h"
#include "ringbuffer.h"

class FrameDetector : public AbstractFrameDetector
{
    Q_OBJECT
public:
    FrameDetector(const QAudioFormat& format, QObject *parent = 0);

public slots:
    void processAudio(const QByteArray& data);

private:
    QAudioFormat format_;
    RingBuffer<double> audio_buffer_;
    RingBuffer<double> bufs500_;
    RingBuffer<double> bufc500_;
    RingBuffer<double> bufs2200_;
    RingBuffer<double> bufc2200_;
    RingBuffer<double> bufsignal_;
    RingBuffer<double> bufflag_;
    RingBuffer<double> bufthreshold_;
    FIRFilter lowpass_;
    FIRFilter bandpass_;
    FIRFilter bandstop_;
    uint64_t total_count_;
    uint32_t frame_sequence_;
    uint64_t prev_count_;
    uint64_t seq_count_;
    bool prev_bit_;
    double level_;
    double signal_level_;
    double input_level_;
    double threshold_;

private:
    typedef RingBuffer<double>::const_iterator RingBufferIterator;
    void clipFrame(const RingBufferIterator& begin, const RingBufferIterator& end);

};
