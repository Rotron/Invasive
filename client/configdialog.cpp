#include "configdialog.h"
#include "stdafx.h"

namespace {

const int WINDOW_WIDTH = 320;
const int WINDOW_HEIGHT = 160;

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

    port_spinbox_ = new QSpinBox(this);
    port_spinbox_->setMinimum(1);
    port_spinbox_->setMaximum(65535);
    port_spinbox_->setValue(settings_.value("server/listen_port").toUInt());
    layout->addRow("Listen port", port_spinbox_);

    logpath_edit_ = new QLineEdit(this);
    logpath_edit_->setText(settings_.value("log/path").toString());
    layout->addRow("Logfile", logpath_edit_);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);
}

void ConfigDialog::accept()
{
    settings_.setValue("server/listen_port", port_spinbox_->value());
    settings_.setValue("log/path", logpath_edit_->text());
    QDialog::accept();
}

void ConfigDialog::reject()
{
    port_spinbox_->setValue(settings_.value("server/listen_port").toUInt());
    logpath_edit_->setText(settings_.value("log/path").toString());
    QDialog::reject();
}
