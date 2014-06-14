#include "configdialog.h"
#include "stdafx.h"

namespace {

const int WINDOW_WIDTH = 320;
const int WINDOW_HEIGHT = 100;

}

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    settings_("Invasive", "settings")
{
    setModal(true);
    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | Qt::Dialog);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QFormLayout *layout = new QFormLayout(this);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    setLayout(layout);

    portSpinBox_ = new QSpinBox(this);
    portSpinBox_->setMinimum(1);
    portSpinBox_->setMaximum(65535);
    portSpinBox_->setValue(settings_.value("server/listen_port").toUInt());
    layout->addRow("Listen port", portSpinBox_);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);
}

void ConfigDialog::accept()
{
    settings_.setValue("server/listen_port", portSpinBox_->value());
    QDialog::accept();
}

void ConfigDialog::reject()
{
    portSpinBox_->setValue(settings_.value("server/listen_port").toUInt());
    QDialog::reject();
}
