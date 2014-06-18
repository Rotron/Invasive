#pragma once
#include "stdafx.h"

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
    QSpinBox* port_spinbox_;
    QLineEdit* logpath_edit_;

};
