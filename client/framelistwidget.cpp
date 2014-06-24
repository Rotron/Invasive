#include "framelistwidget.h"
#include "frame.h"
#include "frameprinter.h"
#include <QClipboard>

Q_DECLARE_METATYPE(FramePtr);

FrameListWidget::FrameListWidget(QWidget *parent) :
    QListWidget(parent),
    auto_scroll_(true)
{
#ifdef Q_OS_MAC
    setAttribute(Qt::WA_MacShowFocusRect, false);
#endif
    setItemDelegate(new FrameListDelegate);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollChanged()));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(const QPoint&)));
}

QList<FramePtr> FrameListWidget::getSelectedFrames()
{
    QList<FramePtr> list;
    foreach (const QListWidgetItem* item, selectedItems()) {
        list.push_back(qvariant_cast<FramePtr>(item->data(Qt::UserRole)));
    }
    return list;
}

QList<FramePtr> FrameListWidget::getAllFrames()
{
    QList<FramePtr> list;
    for (int i = 0; i < count(); ++i) {
        list.push_back(qvariant_cast<FramePtr>(item(i)->data(Qt::UserRole)));
    }
    return list;
}

void FrameListWidget::addFrame(const FramePtr& frame)
{
    if (frame) {
        QListWidgetItem* item = new QListWidgetItem(this);
        item->setData(Qt::UserRole, QVariant::fromValue(frame));
        if (auto_scroll_) {
            QTimer::singleShot(100, this, SLOT(scrollToBottom()));
        }
    }
}

void FrameListWidget::showContextMenu(const QPoint &pos)
{
    QList<QListWidgetItem*> list = selectedItems();
    if (!list.isEmpty()) {
        QMenu *menu = new QMenu();

        QAction *copy_text_action = new QAction(tr("Copy Text"), this);
        connect(copy_text_action, SIGNAL(triggered()), this, SLOT(copyText()));
        menu->addAction(copy_text_action);

        QAction *copy_json_action = new QAction(tr("Copy JSON"), this);
        connect(copy_json_action, SIGNAL(triggered()), this, SLOT(copyJson()));
        menu->addAction(copy_json_action);

        if (list.size() == 1) {
            menu->addSeparator();

            QAction *copy_ascii_info_action = new QAction(tr("Copy ASCII Info"), this);
            connect(copy_ascii_info_action, SIGNAL(triggered()), this, SLOT(copyAsciiInfo()));
            menu->addAction(copy_ascii_info_action);

            QAction *copy_hex_info_action = new QAction(tr("Copy Hex Info"), this);
            connect(copy_hex_info_action, SIGNAL(triggered()), this, SLOT(copyHexInfo()));
            menu->addAction(copy_hex_info_action);
        }

        menu->exec(mapToGlobal(pos));
        delete menu;
    }
}

void FrameListWidget::scrollChanged()
{
    QScrollBar *v = verticalScrollBar();
    auto_scroll_ = (verticalScrollBar()->maximum() - v->value() < 1);
}

void FrameListWidget::scrollToBottom()
{
    QScrollBar *v = verticalScrollBar();
    v->setValue(v->maximum());
}

void FrameListWidget::copyText()
{
    QString text;
    foreach (const QListWidgetItem* item, selectedItems()) {
        if (FramePtr frame = qvariant_cast<FramePtr>(item->data(Qt::UserRole))) {
            text += FramePrinter::toPlainText(frame) + "\n";
        }
    }
    QApplication::clipboard()->setText(text);
}

void FrameListWidget::copyJson()
{
    QString json;
    foreach (const QListWidgetItem* item, selectedItems()) {
        if (FramePtr frame = qvariant_cast<FramePtr>(item->data(Qt::UserRole))) {
            json += FramePrinter::toJson(frame) + "\n";
        }
    }
    QApplication::clipboard()->setText(json);
}

void FrameListWidget::copyAsciiInfo()
{
    if (FramePtr frame = qvariant_cast<FramePtr>(selectedItems().first()->data(Qt::UserRole))) {
        QApplication::clipboard()->setText(FramePrinter::asciiInfo(frame));
    }
}

void FrameListWidget::copyHexInfo()
{
    if (FramePtr frame = qvariant_cast<FramePtr>(selectedItems().first()->data(Qt::UserRole))) {
        QApplication::clipboard()->setText(frame->info().toHex());
    }
}

void FrameListWidget::resizeEvent(QResizeEvent*)
{

}

FrameListDelegate::FrameListDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void FrameListDelegate::paint(
        QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    FramePtr frame = qvariant_cast<FramePtr>(index.data(Qt::UserRole));

    painter->save();

    QTextOption text_option(Qt::AlignLeft);

    QFont font = painter->font();
    painter->setFont(font);

    // datetime
    painter->setPen(QColor("gray"));
    {
        QRect text_rect = option.rect;
        text_rect.setTop(text_rect.top() + 2);
        text_rect.setLeft(160);
        painter->drawText(text_rect, frame->datetime().toString(Qt::ISODate), text_option);
    }

    painter->setPen(QColor("black"));
    painter->setRenderHint(QPainter::Antialiasing);

    // address
    if (!frame->addresses().isEmpty()) {
        QString callsign = frame->addresses().front().callsign;
        QByteArray sha1 = QCryptographicHash::hash(callsign.toUtf8(), QCryptographicHash::Sha1);
        QColor color;
        color.setHsl(static_cast<unsigned char>((sha1[0] + sha1[1] * 256) % 360), 100, 140);
        painter->setPen(color);

        QRect bg_rect = option.rect;
        bg_rect.setLeft(64);
        bg_rect.setWidth(76);
        bg_rect.setTop(bg_rect.top());
        bg_rect.setBottom(bg_rect.bottom());
        QBrush brush(color);
        painter->fillRect(bg_rect, brush);
        painter->setBrush(brush);

        const QPointF points[4] = {
            QPointF(bg_rect.right(), bg_rect.bottom()),
            QPointF(bg_rect.right(), bg_rect.top()),
            QPointF(bg_rect.right() + 6, (bg_rect.top() + bg_rect.bottom()) / 2.0),
            QPointF(bg_rect.right(), bg_rect.bottom())
        };
        painter->drawPolygon(points, 4, Qt::WindingFill);

        QRect text_rect = option.rect;
        text_rect.setLeft(80);
        painter->setPen(QColor("white"));
        QTextOption text_option(Qt::AlignLeft | Qt::AlignVCenter);
        painter->drawText(text_rect, callsign, text_option);
    }

    if (!frame->addresses().isEmpty()) {
        QString callsign = frame->addresses().last().callsign;
        QByteArray sha1 = QCryptographicHash::hash(callsign.toUtf8(), QCryptographicHash::Sha1);
        QColor color;
        color.setHsl(static_cast<unsigned char>((sha1[0] + sha1[1] * 256) % 360), 100, 140);
        painter->setPen(color);

        QRect bg_rect = option.rect;
        bg_rect.setWidth(70);
        bg_rect.setTop(bg_rect.top());
        bg_rect.setBottom(bg_rect.bottom());
        QBrush brush(color);
        painter->fillRect(bg_rect, brush);
        painter->setBrush(brush);

        const QPointF points[4] = {
            QPointF(bg_rect.right(), bg_rect.bottom()),
            QPointF(bg_rect.right(), bg_rect.top()),
            QPointF(bg_rect.right() + 6, (bg_rect.top() + bg_rect.bottom()) / 2.0),
            QPointF(bg_rect.right(), bg_rect.bottom())
        };
        painter->drawPolygon(points, 4, Qt::WindingFill);

        QRect text_rect = option.rect;
        text_rect.setLeft(10);
        painter->setPen(QColor("white"));
        QTextOption text_option(Qt::AlignLeft | Qt::AlignVCenter);
        painter->drawText(text_rect, callsign, text_option);
    }

    painter->setPen(QColor("black"));
    text_option.setWrapMode(QTextOption::NoWrap);

    // ascii
    {
        QRect text_rect = option.rect;
        text_rect.setTop(text_rect.top() + 20);
        text_rect.setLeft(160);
        painter->drawText(text_rect, FramePrinter::asciiInfo(frame), text_option);
    }

    QLinearGradient gradient(option.rect.right() - 150, 0, option.rect.right(), 0);
    gradient.setColorAt(1.0, QColor::fromRgbF(1, 1, 1, 1));
    gradient.setColorAt(0.5, QColor::fromRgbF(1, 1, 1, 1));
    gradient.setColorAt(0.0, QColor::fromRgbF(1, 1, 1, 0));
    QBrush brush(gradient);
    painter->fillRect(option.rect, brush);

    // length
    {
        QRect text_rect = option.rect;
        text_rect.setTop(text_rect.top() + 20);
        text_rect.setRight(text_rect.right() - 5);
        QTextOption text_option(Qt::AlignRight);
        painter->setPen(QColor("gray"));
        painter->drawText(text_rect, QString("[%0 bytes]").arg(frame->info().size()), text_option);
    }

    // length
    if (!frame->isValid()) {
        QRect text_rect = option.rect;
        text_rect.setTop(text_rect.top() + 2);
        text_rect.setRight(text_rect.right() - 5);
        QTextOption text_option(Qt::AlignRight);
        painter->setPen(QColor("red"));
        painter->drawText(text_rect, "Broken", text_option);
    }

#ifdef Q_OS_LINUX
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);
#endif

    painter->setOpacity(0.5);
    QStyledItemDelegate::paint(painter, option, index);
    painter->setOpacity(1.0);

    painter->restore();
}

QSize FrameListDelegate::sizeHint(
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(40);
    return size;
}
