#pragma once
#if defined(Q_OS_WIN32) || defined(Q_OS_LINUX)
#include <GL/glew.h>
#endif
#include <QGLWidget>
#include "stdafx.h"

class WaterfallView : public QGLWidget
{
    Q_OBJECT
public:
    explicit WaterfallView(QWidget *parent = 0);

private:
    void paintEvent(QPaintEvent *event);
    void initializeGL();
    void setupShader();

public slots:
    void updateAudio(const QByteArray& audio);
    void setDecodedPackets(int count);
    void setCompletePackets(int count);
    void setDecodeRatio(double ratio);
    void detected();
    void decorded();

private slots:
    void animate();

private:
    QTimer timer_;
    GLuint line_texture_;
    GLuint textures_[2];
    GLuint framebuffers_[2];
    GLuint program_;
    int count_;
    QVector<float> line_buffer_;

    int decoded_packets_;
    int complete_packets_;
    double decode_ratio_;

    double detected_brightness_;
    double decorded_brightness_;
};
