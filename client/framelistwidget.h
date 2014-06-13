#pragma once
#include "stdafx.h"
#include "forward.h"

class FrameListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit FrameListWidget(QWidget *parent = 0);
    QList<FramePtr> getFrames();

public slots:
    void addFrame(const FramePtr& frame);

private slots:
    void scrollChanged();
    void scrollToBottom();

private:
    void resizeEvent(QResizeEvent* event);

private:
    bool auto_scroll_;

};

class FrameListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    FrameListDelegate(QObject *parent = 0);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
};
