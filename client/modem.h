#pragma once
#include "stdafx.h"
#include "forward.h"

class Modem : public QObject
{
    Q_OBJECT
public:
    explicit Modem(QObject *parent = 0);
    ~Modem();

    void addFrameDetector(const FrameDetectorPtr& detector);
    void addDemodulatorFacory(const DemodulatorFactoryPtr& factory);
    QAudioFormat audioFormat() const;
    static QList<QString> availableDeviceName();
    bool setAudioDeviceIndex(int index);
    void decodeWavFile(const QString& path);

private:
    static QList<QAudioDeviceInfo> availableDevices();

signals:
    void frameDecoded(const FramePtr& frame);
    void frameDetected();
    void audioReceived(const QByteArray& data);
    void decodeRatioUpdated(double ratio);
    void bitDecoded(QString);

private slots:
    void readSoundData();
    void decodeSoundData(const QByteArray& audio);
    void decodeFrame(const FrameAudioPtr& frame);
    void decoded(const FramePtr& frame);

private:
    QScopedPointer<QAudioInput> audio_input_;
    QIODevice* audio_device_;

    QList<FrameDetectorPtr> detectors_;
    QList<DemodulatorFactoryPtr> factories_;
    QThreadPool thread_pool_;
    QThread detector_thread_;

    int detected_count_;
    int decoded_count_;

};
