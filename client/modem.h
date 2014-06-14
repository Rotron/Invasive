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

private:
    static QList<QAudioDeviceInfo> availableDevices();

signals:
    void frameDecorded(const FramePtr& frame);

private slots:
    void readSoundData();
    void frameDetected(const FrameAudioPtr& frame);
    void decoded(const FramePtr& frame);

private:
    QScopedPointer<QAudioInput> audio_input_;
    QIODevice* audio_device_;
    SocketServer* server_;

    QList<FrameDetectorPtr> detectors_;
    QList<DemodulatorFactoryPtr> factories_;
    QThreadPool thread_pool_;
    QThread detector_thread_;

    QMap<QString, QDateTime> last_frame_map_;

};
