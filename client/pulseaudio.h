#pragma once
#include "stdafx.h"
#include <pulse/simple.h>
#include <pulse/error.h>

class PulseAudio : public QObject
{
    Q_OBJECT
public:
    explicit PulseAudio(const QAudioFormat& format, const QString& device, QObject *parent = 0);
    ~PulseAudio();
    QAudioFormat format() const;

signals:
    void received(const QByteArray& audio);

private slots:
    void readData();

private:
    pa_simple* audio_;
    char buffer_[2048];
    QTimer timer_;
    QAudioFormat format_;

};
