#include "demodulator.h"
#include "frame.h"
#include "frameaudio.h"
#include "firfilter.h"
#include "cfloat"
#include "ringbuffer.h"
#include "window.h"
#include <iostream>

namespace {

const int THRESHOLD_RESOLUTION  = 50;
const int MINIMUM_FARME_BYTES   = 16;
const int SAMPLING_RATE         = 13200;
const int BITS_PER_BYTE         = 11;
const double THRESHOLD_WIDTH_RATIO = 2.0;

const char addr_recovery_table[] = {
    ' ', ' ', ' ', '3', ' ', '5', '6', '7', ' ', '9', ' ', '3', ' ',
    '5', '6', '7', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '2', '3', '4', '5', '6', '7', ' ', ' ', ' ', '3', ' ', '5', '6',
    '7', ' ', '9', ' ', '3', ' ', '5', '6', '7', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '2', '3', '4', '5', '6', '7', ' ',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '3', '4', '5', '6', '7', ' ', ' ', ' ', '3', ' ', '5', '6', '7',
    ' ', '9', ' ', '3', ' ', '5', '6', '7', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '2', '3', '4', '5', '6', '7', ' ', ' ',
    ' ', '3', ' ', '5', '6', '7', ' ', '9', ' ', '3', ' ', '5', '6',
    '7', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '2', '3',
    '4', '5', '6', '7', ' ', ' ', ' ', '3', ' ', '5', '6', '7', ' ',
    '9', ' ', '3', ' ', '5', '6', '7', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '2', '3', '4', '5', '6', '7', ' ', ' ', ' ',
    '3', ' ', '5', '6', '7', ' ', '9', ' ', '3', ' ', '5', '6', '7',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '2', '3', '4',
    '5', '6', '7', ' ', ' ', ' ', '3', ' ', '5', '6', '7', ' ', '9',
    ' ', '3', ' ', '5', '6', '7', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '2', '3', '4', '5', '6', '7'
};

}

Demodulator::Demodulator(const Settings& settings) :
    settings_(settings)
{

}

FramePtr Demodulator::decode(const FrameAudio& frame_audio)
{
    QVector<double> buffer;
    buffer.reserve(frame_audio.count());

    FIRFilter bandpass(FIRFilter::FIR_BANDPASS, settings_.window, SAMPLING_RATE, 220, 900, 2500);
    FIRFilter lowpass(FIRFilter::FIR_LOWPASS, settings_.window, 528000, 12000, SAMPLING_RATE / 2);
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

    for (int i = 0; i < buffer.size(); ++i) {
      bufs1200.push_back(sin(M_PI * 2 / SAMPLING_RATE * 1200.0 * i) * buffer[i]);
      bufc1200.push_back(cos(M_PI * 2 / SAMPLING_RATE * 1200.0 * i) * buffer[i]);
      bufs2200.push_back(sin(M_PI * 2 / SAMPLING_RATE * 2200.0 * i) * buffer[i]);
      bufc2200.push_back(cos(M_PI * 2 / SAMPLING_RATE * 2200.0 * i) * buffer[i]);
      double low = std::hypotf(bufs1200.sum(), bufc1200.sum());
      double high = std::hypotf(bufs2200.sum() , bufc2200.sum());
      diff_buff.push_back(low - high);
    }

    QVector<double> diff_buff_center = diff_buff;
    qSort(diff_buff_center);
    double min = diff_buff_center[diff_buff_center.size() / 4];
    double max = diff_buff_center[diff_buff_center.size() / 4 + diff_buff_center.size() / 2];
    double center = diff_buff_center[diff_buff_center.size() / 2];

    for (int i = 0; i <= THRESHOLD_RESOLUTION; ++i) {
        FramePtr frame;
        process(settings_.verfy_fcs, center - std::fabs(center - min / THRESHOLD_WIDTH_RATIO) *
                (1.0 * i / THRESHOLD_RESOLUTION), diff_buff, &frame);
        if (frame) {
            return frame;
        }
        process(settings_.verfy_fcs, center + std::fabs(center - max / THRESHOLD_WIDTH_RATIO) *
                (1.0 * i / THRESHOLD_RESOLUTION), diff_buff, &frame);
        if (frame) {
            return frame;
        }
    }

    return FramePtr();
}

void Demodulator::process(bool verify_fcs, double center, const QVector<double>& diff, FramePtr* result)
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
                    addressRecovery(&frame_data);
                    if (FramePtr frame = Frame::create(QDateTime::currentDateTime(), frame_data)) {
                        if (!verify_fcs || frame->isValid()) {
                            *result = frame;
                        }
                    }
                    return;
                }
                frame_data.clear();
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

void Demodulator::addressRecovery(QByteArray* frame_data)
{
    const uchar* data_ptr = reinterpret_cast<const uchar*>(frame_data->data());
    const uchar* data_ptr_end = data_ptr + frame_data->size();

    QByteArray address;
    do {
        if (data_ptr >= data_ptr_end) return;
        address += (*data_ptr) >> 1;
    } while (!(*(data_ptr++) & 0x1));

    if (address.size() % 7 != 0) return;

    for (int i = 0; i < address.size(); ++i) {
        QChar c(address[i]);
        if ((i + 1) % 7 && !c.isUpper() && !c.isDigit() && !c.isSpace()) {
            c = addr_recovery_table[static_cast<int>(c.toLatin1())];
            (*frame_data)[i] = c.toLatin1() << 1;
        }
    }
    (*frame_data)[address.size() - 1] = (*frame_data)[address.size() - 1] | 0x1;
}

DefaultDemodulatorFactory::DefaultDemodulatorFactory(const Demodulator::Settings& settings) :
    settings_(settings)
{

}

DemodulatorInterface* DefaultDemodulatorFactory::make() const
{
    return new Demodulator(settings_);
}

