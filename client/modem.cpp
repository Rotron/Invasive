#include "modem.h"
#include "abstractframedetector.h"
#include "abstractdemodulator.h"
#include "demodulatorset.h"
#include "frameaudio.h"
#include "frame.h"
#include "wavplayer.h"

namespace {

const int SAMPLING_RATE = 48000;

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
    if (audio_input_) {
        return audio_input_->format();
    }
    else {
        return QAudioFormat();
    }
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
        audio_input_->setNotifyInterval(40);

        audio_device_ = audio_input_->start();
        connect(audio_device_, SIGNAL(readyRead()), this, SLOT(readSoundData()));
        return true;
    }
    else {
        return false;
    }
}

void Modem::decodeWavFile(const QString& path)
{
    WavPlayer *player = new WavPlayer(path);
    if (player->channels() == 1 || player->samplingRate() == SAMPLING_RATE) {
        connect(player, SIGNAL(audioReaded(QByteArray)), this, SLOT(decodeSoundData(QByteArray)));
        player->moveToThread(&detector_thread_);
        player->play();
        player->deleteLater();
    }
    else {
        QMessageBox::warning(qApp->activeWindow(), "Error", "Wav file must be monoral 4.8 kHz audio");
        delete player;
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

    emit audioReceived(audio);
}

void Modem::decodeFrame(const FrameAudioPtr& frame)
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
        connect(detector.get(), SIGNAL(detected(FrameAudioPtr)), this, SLOT(decodeFrame(FrameAudioPtr)));
        connect(detector.get(), SIGNAL(detected(FrameAudioPtr)), this, SIGNAL(frameDetected()));
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
