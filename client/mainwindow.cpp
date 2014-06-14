#include "mainwindow.h"
#include "framelistwidget.h"
#include "defaultdemodulator.h"
#include "framedetector.h"
#include "frame.h"
#include "modem.h"
#include "frameprinter.h"
#include "configdialog.h"
#include "socketserver.h"

namespace {

const int DEFAULT_LISTEN_PORT = 19780;

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    settings_("Invasive", "settings")
{
    restoreGeometry(settings_.value("mainwindow/geometry").toByteArray());
    restoreState(settings_.value("mainwindow/windowState").toByteArray());

    quint16 port = settings_.value("server/listen_port", DEFAULT_LISTEN_PORT).toUInt();
    settings_.setValue("server/listen_port", port);
    server_ = new SocketServer(port, this);

    config_dialog_ = new ConfigDialog(this);

    QVBoxLayout* vlayout = new QVBoxLayout();
    QWidget *widget = new QWidget();
    widget->setLayout(vlayout);
    setCentralWidget(widget);

    QSplitter *splitter = new QSplitter(parent);
    splitter->setOrientation(Qt::Horizontal);
    vlayout->addWidget(splitter);

    frame_list_ = new FrameListWidget(this);
    splitter->addWidget(frame_list_);
    connect(frame_list_, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));

    QVBoxLayout* right_vlayout = new QVBoxLayout();
    right_vlayout->setContentsMargins(0, 0, 0, 10);

    frame_browser_ = new QTextBrowser(this);
    QFont font;
    font.setStyleHint(QFont::Monospace);
    frame_browser_->setFontFamily(font.defaultFamily());
    frame_browser_->setWordWrapMode(QTextOption::WrapAnywhere);
    right_vlayout->addWidget(frame_browser_);

    QFormLayout* form = new QFormLayout();
    right_vlayout->addLayout(form);

    decorded_packets_label_ = new QLabel("Decorded packets: 0", this);
    form->addRow(decorded_packets_label_);

    QWidget *layout_widget = new QWidget(this);
    layout_widget->setLayout(right_vlayout);
    splitter->addWidget(layout_widget);

    splitter->setStretchFactor(0, 5);
    splitter->setStretchFactor(1, 3);

    modem_ = new Modem(this);
    connect(modem_, SIGNAL(frameDecorded(FramePtr)), frame_list_, SLOT(addFrame(FramePtr)));
    connect(modem_, SIGNAL(frameDecorded(FramePtr)), server_, SLOT(writeFrame(FramePtr)));
    connect(modem_, SIGNAL(frameDecorded(FramePtr)), this, SLOT(frameDecorded()));

    initializeMenuBar();
}

MainWindow::~MainWindow()
{

}

void MainWindow::frameDecorded()
{
    decorded_packets_label_->setText(QString("Decorded packets: %0").arg(frame_list_->count()));
}

void MainWindow::selectionChanged()
{
    QString html;
    foreach (const FramePtr frame, frame_list_->getFrames()) {
        html += FramePrinter::toHtmlText(frame);
    }
    frame_browser_->setHtml(html);
}

void MainWindow::setInputDevice(int index)
{
    input_action_group_->actions().at(index)->setChecked(true);
    modem_->setAudioDeviceIndex(index);
    settings_.setValue("input_device", index);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings_.setValue("mainwindow/geometry", saveGeometry());
    settings_.setValue("mainwindow/windowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::initializeMenuBar()
{
    QMenuBar* menu = new QMenuBar;
    setMenuBar(menu);

    QMenu* input_menu = new QMenu("Input");
    menu->addMenu(input_menu);

    QSignalMapper* mapper = new QSignalMapper(this);

    input_action_group_ = new QActionGroup(this);
    input_action_group_->setExclusive(true);

    int i = 0;
    foreach (const QString& name, Modem::availableDeviceName()) {
       QAction *inputAction = new QAction(name, this);
       connect(inputAction, SIGNAL(triggered()), mapper, SLOT(map()));
       inputAction->setCheckable(true);
       input_action_group_->addAction(inputAction);
       input_menu->addAction(inputAction);
       mapper->setMapping(inputAction, i);
       i++;
    }
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(setInputDevice(int)));

    QMenu *help_menu = menu->addMenu(tr("Help"));
    QAction *about_qt_act = new QAction(tr("About Qt..."), this);
    about_qt_act->setMenuRole(QAction::AboutQtRole);
    connect(about_qt_act, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    help_menu->addAction(about_qt_act);

    setInputDevice(settings_.value("input_device", 0).toUInt());
    modem_->addDemodulatorFacory(std::make_shared<DefaultDemodulatorFactory>());
    modem_->addFrameDetector(std::make_shared<FrameDetector>(modem_->audioFormat()));

    QMenu *tool_menu = menu->addMenu(tr("Tools"));
    QAction *configAct = new QAction(tr("Preferences..."), this);
    configAct->setShortcut(QKeySequence("Ctrl+K"));
    configAct->setMenuRole(QAction::PreferencesRole);
    connect(configAct, SIGNAL(triggered()), config_dialog_, SLOT(show()));
    tool_menu->addAction(configAct);
}
