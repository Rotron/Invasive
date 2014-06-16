#include "modem.h"
#include "abstractframedetector.h"
#include "abstractdemodulator.h"
#include "frameaudio.h"
#include "frame.h"

extern "C" void rdft(int, int, double *, int *, double *);

namespace {

const int SAMPLING_RATE = 48000;
const int FFT_SIZE      = 2048;
const int FFT_GAIN      = 2000;

}

Modem::Modem(QObject *parent) :
    QObject(parent),
    audio_input_(nullptr)
{
    qRegisterMetaType<FramePtr>("FramePtr");
    qRegisterMetaType<FrameAudioPtr>("FrameAudioPtr");
    detector_thread_.start();
}

Modem::~Modem()
{
    detector_thread_.exit();
    detector_thread_.wait();
    audio_input_->stop();
    thread_pool_.releaseThread();
    thread_pool_.waitForDone();
}

QAudioFormat Modem::audioFormat() const
{
    return audio_input_->format();
}

bool Modem::setAudioDeviceIndex(int index)
{
    if (index >= 0 && index < availableDevices().size()) {
        QAudioDeviceInfo device = availableDevices()[index];

        QAudioFormat format;
        format.setSampleRate(SAMPLING_RATE);
        format.setChannelCount(1);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setSampleType(QAudioFormat::SignedInt);
        format.setByteOrder(QAudioFormat::LittleEndian);

        if (!device.isFormatSupported(format)) {
            format = device.nearestFormat(format);
        }

        qDebug() << device.deviceName();
        qDebug() << "sample rate: " << format.sampleRate();
        qDebug() << "channels   : " << format.channelCount();
        qDebug() << "sample size: " << format.sampleSize();
        qDebug() << "threads:     " << thread_pool_.maxThreadCount();

        audio_input_.reset(new QAudioInput(device, format));
        audio_input_->setNotifyInterval(20);

        connect(audio_input_.data(), SIGNAL(notify()), this, SLOT(readSoundData()));
        audio_device_ = audio_input_->start();
        return true;
    }
    else {
        return false;
    }
}

QList<QString> Modem::availableDeviceName()
{
    QList<QString> devices;
    foreach (const QAudioDeviceInfo& info, Modem::availableDevices()) {
        devices += info.deviceName();
    }
    return devices;
}

QList<QAudioDeviceInfo> Modem::availableDevices()
{
    QList<QAudioDeviceInfo> list;
    foreach (const QAudioDeviceInfo& info, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        QAudioFormat format;
        format.setSampleRate(SAMPLING_RATE);
        format.setSampleSize(16);
        format = info.nearestFormat(format);
        if (format.sampleRate() == SAMPLING_RATE && format.sampleSize() == 16) {
            list += info;
        }
    }
    return list;
}

void Modem::readSoundData()
{
    if (audio_device_) {
        QByteArray audio = audio_device_->read(audio_input_->bytesReady());
        for (const auto& d : detectors_) {
            QMetaObject::invokeMethod(d.get(), "processAudio", Qt::QueuedConnection, Q_ARG(QByteArray, audio));
        }

        QVector<float> spectrum(FFT_SIZE, 0);
        for (int i = 0; i < audio.size() / FFT_SIZE / 2; ++i) {
            QVector<double> signal(FFT_SIZE, 0);
            for (int j = 0; j < FFT_SIZE; ++j) {
                signal[j] = ((short*)audio.data())[j + FFT_SIZE * i] ;
            }
            static int fft_ip[FFT_SIZE / 2] = {0};
            static double fft_w[FFT_SIZE / 2] = {0};
            rdft(FFT_SIZE, -1, signal.data(), fft_ip, fft_w);
            for (int j = 0; j < FFT_SIZE; ++j) {
                spectrum[j] += signal[j] / (audio.size() / FFT_SIZE) / FFT_GAIN;
            }
        }
        emit audioSpectrumUpdated(spectrum.mid(0, FFT_SIZE / 2));
    }
}

void Modem::frameDetected(const FrameAudioPtr& frame)
{
    for (const auto& f : factories_) {
        if (AbstractDemodulator *demodulator = f->make(std::make_shared<FrameAudio>(*frame))) {
            connect(demodulator, SIGNAL(decorded(FramePtr)), this, SLOT(decoded(FramePtr)));
            thread_pool_.start(demodulator);
        }
    }
}

void Modem::addFrameDetector(const FrameDetectorPtr& detector)
{
    if (detector) {
        connect(detector.get(), SIGNAL(detected(FrameAudioPtr)), this, SLOT(frameDetected(FrameAudioPtr)));
        detector->moveToThread(&detector_thread_);
        detectors_.push_back(detector);
    }
}

void Modem::addDemodulatorFacory(const DemodulatorFactoryPtr& factory)
{
    if (factory) {
        factories_.push_back(factory);
    }
}

void Modem::decoded(const FramePtr& frame)
{
    QDateTime last_decorded = last_frame_map_[frame->sha1()];
    if (last_decorded.isNull() || last_decorded.secsTo(frame->datetime()) > 5) {
        last_frame_map_[frame->sha1()] = frame->datetime();
        emit frameDecorded(frame);
    }
}
