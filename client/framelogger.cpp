#include "framelogger.h"
#include "frameprinter.h"

FrameLogger::FrameLogger(const QString& path, QObject *parent) :
    QObject(parent),
    file_(path)
{
    QFile::OpenMode mode = (QIODevice::Append | QIODevice::Text);
    if (!file_.exists()) mode = QIODevice::WriteOnly;
    if (!file_.open(mode)) {
        qDebug() << "failed to open log file.";
    }
}

void FrameLogger::writeFrame(const QString& frame)
{
    QString text = frame + "\n";
    file_.write(text.toUtf8());
    file_.flush();
}
