#include "modem.h"
#include "abstractframedetector.h"
#include "abstractdemodulator.h"
#include "demodulatorset.h"
#include "frameaudio.h"
#include "frame.h"

extern "C" void rdft(int, int, double *, int *, double *);

namespace {

const int SAMPLING_RATE = 48000;
const int FFT_SIZE      = 1024;
const int FFT_GAIN      = 20000;

}

Modem::Modem(QObject *parent) :
    QObject(parent),
    audio_input_(nullptr),
    detected_count_(0),
    decoded_count_(0)
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
#if defined(Q_OS_LINUX)
    if (pulse_audio_) {
        return pulse_audio_->format();
    }
    else {
        return QAudioFormat();
    }
#else
    if (audio_input_) {
        return audio_input_->format();
    }
    else {
        return QAudioFormat();
    }
#endif
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
#if defined(Q_OS_LINUX)
        pulse_audio_.reset(new PulseAudio(format, device.deviceName()));
        connect(pulse_audio_.data(), SIGNAL(received(QByteArray)), this, SLOT(decodeSoundData(QByteArray)));
#else
        if (!device.isFormatSupported(format)) {
            format = device.nearestFormat(format);
        }

        qDebug() << device.deviceName();
        qDebug() << "sample rate: " << format.sampleRate();
        qDebug() << "channels   : " << format.channelCount();
        qDebug() << "sample size: " << format.sampleSize();
        qDebug() << "threads:     " << thread_pool_.maxThreadCount();

        audio_input_.reset(new QAudioInput(device, format));
        audio_input_->setNotifyInterval(40);

        connect(audio_input_.data(), SIGNAL(notify()), this, SLOT(readSoundData()));
        audio_device_ = audio_input_->start();
#endif
        return true;
    }
    else {
        return false;
    }
}

void Modem::decodeWavFile(const QString& path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly) && file.size() > 42) {
        file.read(22);
        int channels = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(file.read(2).data()));
        int rate = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(file.read(4).data()));
        file.read(14);
        if (channels != 1 || rate != SAMPLING_RATE) {
            return;
        }
        else {
            decodeSoundData(file.readAll());
        }
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
        decodeSoundData(audio);
    }
}

void Modem::decodeSoundData(const QByteArray& audio)
{
    for (const auto& d : detectors_) {
        QMetaObject::invokeMethod(d.get(), "processAudio", Qt::QueuedConnection, Q_ARG(QByteArray, audio));
    }

    QVector<float> spectrum(FFT_SIZE, 0);
    for (int i = 0; i < audio.size() / FFT_SIZE / 2; ++i) {
        QVector<double> signal(FFT_SIZE, 0);
        for (int j = 0; j < FFT_SIZE; ++j) {
            signal[j] = ((short*)audio.data())[j + FFT_SIZE * i]
                    * 0.5 * (1.0 - cos(2 * M_PI * (j / 1024.0)));
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

void Modem::frameDetected(const FrameAudioPtr& frame)
{
    DemodulatorSet *set = new DemodulatorSet(factories_, frame);
    connect(set, SIGNAL(decoded(FramePtr)), this, SLOT(decoded(FramePtr)));
    thread_pool_.start(set);
    detected_count_++;
    emit decodeRatioUpdated(1.0 * decoded_count_ / detected_count_);
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
    if (frame) {
        if (frame->isValid()) decoded_count_++;
        emit decodeRatioUpdated(1.0 * decoded_count_ / detected_count_);
        emit frameDecoded(frame);
    }
}
