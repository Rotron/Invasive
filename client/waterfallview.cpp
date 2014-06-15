#include "waterfallview.h"

namespace {

const int VIEW_HEIGHT = 140;

}

WaterfallView::WaterfallView(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    count_(0),
    decorded_packets_(0)
{
    setAutoFillBackground(false);
    setMinimumHeight(VIEW_HEIGHT);
    setMaximumHeight(VIEW_HEIGHT);

    connect(&timer_, SIGNAL(timeout()), this, SLOT(animate()));
    timer_.start(24);
}

void WaterfallView::updateAudioSpectrum(const QVector<float>& data)
{
    line_buffer_ = data;
}

void WaterfallView::setDecordedPackets(int count)
{
    decorded_packets_ = count;
}

void WaterfallView::animate()
{
    update();
}

void WaterfallView::paintEvent(QPaintEvent *event)
{
    count_++;

    makeCurrent();
    glLoadIdentity();
    glViewport(0, 0, rect().width(), rect().height());

    glBindTexture(GL_TEXTURE_1D, line_texture_);

    if (line_buffer_.size() > 0) {
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, line_buffer_.size(),
                     0, GL_RED, GL_FLOAT, line_buffer_.data());
    }
    else {
        unsigned char buf[1] = {0};
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 1,
                     0, GL_RED, GL_UNSIGNED_BYTE, buf);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_[count_ % 2]);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, 1024, 1024);
    glEnable(GL_TEXTURE_1D);
    glBegin(GL_QUADS);
    glTexCoord1d(0.0);
    glVertex2d(-1.0, -1.0);
    glTexCoord1d(1.0);
    glVertex2d(1.0, -1.0);
    glTexCoord1d(1.0);
    glVertex2d(1.0,  1.0);
    glTexCoord1d(0.0);
    glVertex2d(-1.0,  1.0);
    glEnd();
    glDisable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, 0);

    glBindTexture(GL_TEXTURE_2D, textures_[1 - count_ % 2]);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2d(0.0, 0.0);
    glVertex2d(-1.0, -1.0 + 0.01);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(1.0, -1.0 + 0.01);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(1.0,  1.0 + 0.01);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(-1.0,  1.0 + 0.01);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, rect().width(), rect().height());
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, textures_[count_ % 2]);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(-1.0, -1.0);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(1.0, -1.0);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(1.0,  1.0);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(-1.0,  1.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::white);
    QRect text_rect = rect();
    text_rect.setTop(text_rect.top() + 8);
    text_rect.setRight(text_rect.right() - 10);
    painter.drawText(text_rect, Qt::AlignRight, QString("Decorded packets: %0").arg(decorded_packets_));
    painter.end();
}

void WaterfallView::initializeGL()
{
    QGLWidget::initializeGL();

    makeCurrent();
    static QColor white(0, 0, 0);
    qglClearColor(white);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_FRAMEBUFFER);

    glGenTextures(1, &line_texture_);
    glGenTextures(2, textures_);
    glGenFramebuffers(2, framebuffers_);

    glBindTexture(GL_TEXTURE_1D, line_texture_);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_[0]);
    glBindTexture(GL_TEXTURE_2D, textures_[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures_[0], 0);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_[1]);
    glBindTexture(GL_TEXTURE_2D, textures_[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures_[1], 0);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_[0]);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_[1]);
    glClear(GL_COLOR_BUFFER_BIT);
}
