#pragma once

#include <QDialog>

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigDialog(QWidget *parent = 0);

signals:

public slots:
    void accept();
    void reject();

private:
    QSettings settings_;
    QSpinBox* portSpinBox_;

};
