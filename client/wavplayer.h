#pragma once
#include "stdafx.h"
#include <QObject>

class WavPlayer : public QObject
{
    Q_OBJECT
public:
    explicit WavPlayer(const QString& filename, QObject *parent = 0);
    int channels() const;
    int samplingRate() const;

signals:
    void audioReaded(QByteArray);

public slots:
    void play();

private:
    QFile file_;
    int channels_;
    int sampling_rate_;

};
