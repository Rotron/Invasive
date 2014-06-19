#include "defaultdemodulator.h"
#include "frame.h"
#include "frameaudio.h"
#include "firfilter.h"
#include "cfloat"
#include "ringbuffer.h"
#include <iostream>

namespace {

const int THRESHOLD_RESOLUTION  = 50;
const int MINIMUM_FARME_BYTES   = 16;
const int SAMPLING_RATE         = 13200;
const int BITS_PER_BYTE         = 11;
const double THRESHOLD_WIDTH_RATIO = 2.0;

}

DefaultDemodulator::DefaultDemodulator(const FrameAudioPtr& frame_audio) :
    AbstractDemodulator(frame_audio)
{

}

FramePtr DefaultDemodulator::exec(const FrameAudio& frame_audio)
{
    QVector<double> buffer;
    buffer.reserve(frame_audio.count());

    FIRFilter bandpass(FIRFilter::FIR_BANDPASS, SAMPLING_RATE, 220, 900, 2500);
    FIRFilter lowpass(FIRFilter::FIR_LOWPASS, 528000, 12000, SAMPLING_RATE / 2);
    int index = 0;
    for (double value : frame_audio) {
        double low = lowpass.process(value);
        if (index % 40 == 0) {
            buffer.push_back(bandpass.process(low));
        }
        index++;

        for (int i = 0; i < 10; ++i) {
            low = lowpass.process(0);
            if (index % 40 == 0) {
                buffer.push_back(bandpass.process(low));
            }
            index++;
        }
    }

    RingBuffer<double> bufs1200(BITS_PER_BYTE);
    RingBuffer<double> bufc1200(BITS_PER_BYTE);
    RingBuffer<double> bufs2200(BITS_PER_BYTE);
    RingBuffer<double> bufc2200(BITS_PER_BYTE);

    QVector<double> diff_buff;

    double max = -FLT_MAX;
    double min = FLT_MAX;
    for (int i = 0; i < buffer.size(); ++i) {
      bufs1200.push_back(sin(M_PI * 2 / SAMPLING_RATE * 1200.0 * i) * buffer[i]);
      bufc1200.push_back(cos(M_PI * 2 / SAMPLING_RATE * 1200.0 * i) * buffer[i]);
      bufs2200.push_back(sin(M_PI * 2 / SAMPLING_RATE * 2200.0 * i) * buffer[i]);
      bufc2200.push_back(cos(M_PI * 2 / SAMPLING_RATE * 2200.0 * i) * buffer[i]);
      double low = std::hypotf(bufs1200.sum(), bufc1200.sum());
      double high = std::hypotf(bufs2200.sum() , bufc2200.sum());
      diff_buff.push_back(low - high);
      max = std::max(max, low - high);
      min = std::min(min, low - high);
    }

    double avg = (max + min) / 2.0;

    for (int i = 0; i <= THRESHOLD_RESOLUTION; ++i) {
        FramePtr frame;
        process(true, avg - std::fabs(avg - min / THRESHOLD_WIDTH_RATIO) *
                (1.0 * i / THRESHOLD_RESOLUTION), diff_buff, &frame);
        if (frame) {
            return frame;
        }
        process(true, avg + std::fabs(avg - max / THRESHOLD_WIDTH_RATIO) *
                (1.0 * i / THRESHOLD_RESOLUTION), diff_buff, &frame);
        if (frame) {
            return frame;
        }
    }

    for (int i = 0; i <= THRESHOLD_RESOLUTION; ++i) {
        FramePtr frame;
        process(false, avg - std::fabs(avg - min / THRESHOLD_WIDTH_RATIO) *
                (1.0 * i / THRESHOLD_RESOLUTION), diff_buff, &frame);
        if (frame) return frame;
        process(false, avg + std::fabs(avg - max / THRESHOLD_WIDTH_RATIO) *
                (1.0 * i / THRESHOLD_RESOLUTION), diff_buff, &frame);
        if (frame) return frame;
    }

    return FramePtr();
}

void DefaultDemodulator::process(bool verify_fcs, double center, const QVector<double>& diff, FramePtr* result)
{
    QVector<bool> bit_buffer;
    RingBuffer<double> cb(BITS_PER_BYTE);

    bool prev_sym = false;
    bool sym = false;
    int bit_timer = 15;

    bool prev_s = false;
    int r = 0;
    for (int i = 0; i < diff.size(); ++i) {

        if (diff[i] >= center) {
            cb.push_back(1);
        }
        else if (diff[i] < center) {
            cb.push_back(-1);
        }

        sym = (cb.sum() < 0);

        --bit_timer;

        if (sym != prev_sym) {
            prev_sym = sym;
            if (bit_timer == 7 || bit_timer == 8) {
                bit_timer--;
            }
            else if (bit_timer == 0 || bit_timer == 1) {
                bit_timer++;
            }
        }

        if (bit_timer == 0) {
            bit_timer = BITS_PER_BYTE;

            if (sym == prev_s) {
                bit_buffer.push_back(1);
                r++;
            }
            else {
                // Bit Stuffing
                if (r != 6) {
                    bit_buffer.push_back(0);
                }
                prev_s = sym;
                r = 1;
            }
        }
    }

    bool sync = false;
    uchar symbol = 0;

    QByteArray frame_data;

    for (auto it = bit_buffer.begin(); it < bit_buffer.end() - 8;) {
        if (sync) {
            unsigned char c = 0;
            c |= (*(++it) << 0);
            c |= (*(++it) << 1);
            c |= (*(++it) << 2);
            c |= (*(++it) << 3);
            c |= (*(++it) << 4);
            c |= (*(++it) << 5);
            c |= (*(++it) << 6);
            c |= (*(++it) << 7);

            if (c == '~') {
                if (frame_data.size() > MINIMUM_FARME_BYTES) {
                    if (FramePtr frame = Frame::create(frame_audio_->startTime(), frame_data, verify_fcs)) {
                        *result = frame;
                        return;
                    }
                    else {
                        return;
                    }
                }
            }
            else {
                frame_data += c;
            }
        }
        else {
            symbol <<= 1;
            symbol |= *(++it);
            if (symbol == '~') {
                sync = true;
            }
        }
    }

    return;
}

AbstractDemodulator* DefaultDemodulatorFactory::make(const FrameAudioPtr& frame_audio) const
{
    return new DefaultDemodulator(frame_audio);
}

