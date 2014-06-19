#include "framedetector.h"
#include "frameaudio.h"

namespace {

const int AUDIO_BUFFER_SIZE       = 262144;
const int SAMPLING_RATE           = 48000;
const int BITS_PER_BYTE           = 40;
const int SN_LEVEL_BUFFER_SIZE    = 512;
const int FRAME_FLAG_LENGTH       = 25;
const int FRAME_FLAG_BUFFER_SIZE  = (FRAME_FLAG_LENGTH + 1) * BITS_PER_BYTE;
const int THRESHOLD_BUFFER_SIZE   = 8;
const int DELAY_SAMPLES           = SAMPLING_RATE / 1200 * 200;
const int MINIMUM_BIT_LENGTH      = 2000;
const int MINIMUM_LEVEL           = 10.0;

const double LEVEL_DECREASE_SPEED = 1.0;
const double FRAME_STOP_SN_RATIO  = 2.0;
const double FRAME_START_SN_RATIO = 1.0;

}

FrameDetector::FrameDetector(const QAudioFormat& format, QObject *parent) :
    AbstractFrameDetector(parent),
    format_(format),
    audio_buffer_(AUDIO_BUFFER_SIZE),
    bufs1200_(BITS_PER_BYTE),
    bufc1200_(BITS_PER_BYTE),
    bufs2200_(BITS_PER_BYTE),
    bufc2200_(BITS_PER_BYTE),
    bufsignal_(SN_LEVEL_BUFFER_SIZE),
    bufflag_(FRAME_FLAG_BUFFER_SIZE),
    bufthreshold_(THRESHOLD_BUFFER_SIZE),
    lowpass_(FIRFilter::FIR_LOWPASS,   SAMPLING_RATE, 1200, 600.0),
    bandpass_(FIRFilter::FIR_BANDPASS, SAMPLING_RATE, 900,  1200.0, 2200.0),
    bandstop_(FIRFilter::FIR_BANDSTOP, SAMPLING_RATE, 900,  1200.0, 2200.0),
    total_count_(0),
    frame_sequence_(0),
    prev_count_(0),
    level_(0.0),
    signal_level_(0.0),
    input_level_(0.0),
    threshold_(0)
{

}

void FrameDetector::processAudio(const QByteArray& data)
{
    int channels = format_.channelCount();
    int sample_bytes = format_.bytesPerFrame();
    int sample_rate = format_.sampleRate();
    int samples = data.size() / sample_bytes;

    const int16_t* ptr = reinterpret_cast<const int16_t*>(data.data());

    auto ST = QDateTime::currentDateTime();
    for (int i = 0; i < samples; ++i) {

        double sample = 0.0;
        for (int j = 0; j < channels; ++j) {
            sample += ptr[i * channels + j];
        }

        audio_buffer_.push_back(sample);

        uint64_t count = i + total_count_;

        bufs1200_.push_back(sin(M_PI * 2 / sample_rate * 1200.0 * count) * sample);
        bufc1200_.push_back(cos(M_PI * 2 / sample_rate * 1200.0 * count) * sample);
        bufs2200_.push_back(sin(M_PI * 2 / sample_rate * 2200.0 * count) * sample);
        bufc2200_.push_back(cos(M_PI * 2 / sample_rate * 2200.0 * count) * sample);

        double ss = std::fabs(bandpass_.process(sample));
        double ns = std::fabs(sample);

        if (signal_level_ >= LEVEL_DECREASE_SPEED) signal_level_ -= LEVEL_DECREASE_SPEED;
        if (input_level_  >= LEVEL_DECREASE_SPEED) input_level_  -= LEVEL_DECREASE_SPEED;

        signal_level_ = std::max(signal_level_, ss);
        input_level_ = std::max(input_level_, ns);

        double ratio = (input_level_ > 1.0) ? signal_level_ / input_level_ : 1.0;
        bufsignal_.push_back(ratio);

        double low  = std::hypotf(bufs1200_.sum(), bufc1200_.sum());
        double high = std::hypotf(bufs2200_.sum(), bufc2200_.sum());
        double level = low - high;

        bufflag_.push_back(level);

        if (input_level_ < MINIMUM_LEVEL) {
            continue;
        }

        if (count % SN_LEVEL_BUFFER_SIZE == 0) {
            double signal = bufsignal_.sum();
            if (threshold_ > 0) {
                if (signal < threshold_ / FRAME_STOP_SN_RATIO) {
                    int length = count - prev_count_;
                    if (length > MINIMUM_BIT_LENGTH) {
                        auto begin = audio_buffer_.size() < static_cast<size_t>(DELAY_SAMPLES + length) ?
                                    audio_buffer_.begin() : audio_buffer_.end() - DELAY_SAMPLES - length;

                        clipFrame(begin, audio_buffer_.end());

                        prev_count_ = count;
                        threshold_ = 0;
                    }
                }
            }
        }
        if (count % (BITS_PER_BYTE) == 0) {
            if (bufflag_.size() == bufflag_.capacity()) {
                for (int j = 0; j < BITS_PER_BYTE / 2 + 1; ++j) {
                    uint32_t frag_sequence = 0;
                    const uint32_t frag_sequence_begin = 0x1010101;
                    const uint32_t frag_sequence_end   = 0xFEFEFE;
                    for (int k = 0; k < FRAME_FLAG_LENGTH; ++k) {
                        double level = 0;
                        for (auto it = bufflag_.begin() + j + k * BITS_PER_BYTE;
                             it != bufflag_.begin() + j + (k + 1) * BITS_PER_BYTE; ++it) {
                            level += *it;
                        }
                        bool bit = level  < 0;
                        frag_sequence <<= 1;
                        if (bit) {
                            frag_sequence |= 1;
                        }
                        if (frag_sequence != (frag_sequence_begin >> (FRAME_FLAG_LENGTH - k - 1)) &&
                                frag_sequence != (frag_sequence_end >> (FRAME_FLAG_LENGTH - k - 1))) {
                            break;
                        }
                    }

                    // Flag detected
                    double signal = bufsignal_.sum();
                    if ((frag_sequence & 0x1FFFFFF) == 0x1010101 || (frag_sequence & 0x1FFFFFF) == 0xFEFEFE) {
                        if (threshold_ > 0) {
                            int length = count - prev_count_;
                            if (length > MINIMUM_BIT_LENGTH) {

                                auto begin = audio_buffer_.size() < static_cast<size_t>(DELAY_SAMPLES + length) ?
                                            audio_buffer_.begin() : audio_buffer_.end() - DELAY_SAMPLES - length;

                                clipFrame(begin, audio_buffer_.end());

                                prev_count_ = count;
                                threshold_ = 0;
                            }
                        }
                        threshold_ = signal;
                        bufthreshold_.push_back(threshold_);
                        prev_count_ = count;
                    }
                    else if (threshold_ == 0 && signal > bufthreshold_.average() / FRAME_START_SN_RATIO) {
                        threshold_ = signal;
                        prev_count_ = count;
                    }
                }
            }
        }
    }

    total_count_ += samples;
}

void FrameDetector::clipFrame(const RingBufferIterator& begin, const RingBufferIterator& end)
{
    QVector<double> buffer;
    for (auto it = begin; it != end; ++it) {
        buffer.push_back(*it);
    }

    FrameAudioPtr ptr = std::make_shared<FrameAudio>(
                format_.sampleRate(),
                QDateTime::currentDateTime(), buffer);

    emit detected(ptr);
}
