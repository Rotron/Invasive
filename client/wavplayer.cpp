#include "wavplayer.h"

WavPlayer::WavPlayer(const QString& filename, QObject *parent) :
    QObject(parent),
    file_(filename)
{
    if (file_.open(QFile::ReadOnly) && file_.size() > 42) {
        file_.read(22);
        channels_ = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(file_.read(2).data()));
        sampling_rate_ = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(file_.read(4).data()));
        file_.read(14);
    }
}

void WavPlayer::play()
{
    emit audioReaded(file_.readAll());
}

int WavPlayer::channels() const
{
    return channels_;
}

int WavPlayer::samplingRate() const
{
    return sampling_rate_;
}
