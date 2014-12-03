#include "mainwindow.h"
#include "framelistwidget.h"
#include "demodulator.h"
#include "framedetector.h"
#include "frame.h"
#include "modem.h"
#include "frameprinter.h"
#include "configdialog.h"
#include "socketserver.h"
#include "waterfallview.h"
#include "framelogger.h"
#include "window.h"

namespace {

const int DEFAULT_LISTEN_PORT = 19780;

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    settings_("Invasive", "settings")
{
    initializeSettings();

    restoreGeometry(settings_.value("mainwindow/geometry").toByteArray());
    restoreState(settings_.value("mainwindow/windowState").toByteArray());

    config_dialog_ = new ConfigDialog(this);

    QVBoxLayout* vlayout = new QVBoxLayout();
    QWidget *widget = new QWidget();
    widget->setLayout(vlayout);
    setCentralWidget(widget);

    QSplitter *splitter = new QSplitter(parent);
    splitter->setOrientation(Qt::Horizontal);
    vlayout->addWidget(splitter);

    waterfall_ = new WaterfallView(this);
    vlayout->addWidget(waterfall_);

    frame_list_ = new FrameListWidget(this);
    splitter->addWidget(frame_list_);
    connect(frame_list_, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
    frame_list_->setVisible(false);

    QVBoxLayout* right_vlayout = new QVBoxLayout();
    right_vlayout->setContentsMargins(0, 0, 0, 0);

    frame_browser_ = new QTextBrowser(this);
    QFont font;
    font.setStyleHint(QFont::Monospace);
    frame_browser_->setFontFamily(font.defaultFamily());
    frame_browser_->setWordWrapMode(QTextOption::WrapAnywhere);
    right_vlayout->addWidget(frame_browser_);

    QFormLayout* form = new QFormLayout();
    right_vlayout->addLayout(form);

    QWidget *layout_widget = new QWidget(this);
    layout_widget->setLayout(right_vlayout);
    splitter->addWidget(layout_widget);

    splitter->setStretchFactor(0, 5);
    splitter->setStretchFactor(1, 3);

    modem_ = new Modem(this);

    connect(modem_, SIGNAL(frameDecoded(FramePtr)), frame_list_, SLOT(addFrame(FramePtr)));
    connect(modem_, SIGNAL(frameDecoded(FramePtr)), server_, SLOT(writeFrame(FramePtr)));
    connect(modem_, SIGNAL(frameDecoded(FramePtr)), this, SLOT(frameDecoded()));
    connect(modem_, SIGNAL(audioReceived(QByteArray)),
            waterfall_, SLOT(updateAudio(QByteArray)));
    connect(modem_, SIGNAL(decodeRatioUpdated(double)),
            waterfall_, SLOT(setDecodeRatio(double)));
    connect(modem_, SIGNAL(frameDetected()), waterfall_, SLOT(detected()));
    connect(modem_, SIGNAL(frameDecoded(FramePtr)), waterfall_, SLOT(decorded()));
    connect(modem_, SIGNAL(bitDecoded(QString)), this, SLOT(bitDecoded(QString)));

    QString log_path = settings_.value("log/path").toString();
    logger_ = new FrameLogger(log_path, this);
    connect(modem_, SIGNAL(frameDecoded(FramePtr)), logger_, SLOT(writeFrame(FramePtr)));

    initializeMenuBar();

    modem_->addFrameDetector(std::make_shared<FrameDetector>(modem_->audioFormat()));

    modem_->addDemodulatorFacory(std::make_shared<DefaultDemodulatorFactory>(
                                     Demodulator::Settings({true, KaiserBessel<220>()})));
    modem_->addDemodulatorFacory(std::make_shared<DefaultDemodulatorFactory>(
                                     Demodulator::Settings({true, Flattop()})));
    modem_->addDemodulatorFacory(std::make_shared<DefaultDemodulatorFactory>(
                                     Demodulator::Settings({true, KaiserBessel<0>()})));
    modem_->addDemodulatorFacory(std::make_shared<DefaultDemodulatorFactory>(
                                     Demodulator::Settings({false, KaiserBessel<0>()})));
}

MainWindow::~MainWindow()
{

}

void MainWindow::bitDecoded(const QString& str)
{
    frame_browser_->append(str);
}

void MainWindow::frameDecoded()
{
    int count = 0;
    foreach (const FramePtr& frame, frame_list_->getAllFrames()) {
        if (frame->isValid()) count++;
    }
    waterfall_->setCompletePackets(count);
    waterfall_->setDecodedPackets(frame_list_->count());
    save_file_act_->setEnabled(frame_list_->count() > 0);
}

void MainWindow::selectionChanged()
{
    QString html;
    foreach (const FramePtr frame, frame_list_->getSelectedFrames()) {
        html += FramePrinter::toHtmlText(frame);
    }
    frame_browser_->setHtml(html);

    save_selected_act_->setEnabled(!frame_list_->getSelectedFrames().isEmpty());
}

void MainWindow::setInputDevice(int index)
{
    if (modem_->setAudioDeviceIndex(index)) {
        input_action_group_->actions().at(index)->setChecked(true);
        settings_.setValue("input_device", index);
    }
}

void MainWindow::saveAsFile(bool selected)
{
    QString path = QFileDialog::getSaveFileName(
                this, "Save as file", QDir::homePath() + "/log.txt", "Text (*.txt)");

    QFile file(path);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QList<FramePtr> frames;
        if (selected) {
            frames = frame_list_->getSelectedFrames();
        }
        else {
            frames = frame_list_->getAllFrames();
        }
        foreach (const FramePtr frame, frames) {
            file.write((FramePrinter::toPlainText(frame) + "\n").toUtf8());
        }
    }
}

void MainWindow::saveSelectedAsFile()
{
    saveAsFile(true);
}

void MainWindow::openLogFile()
{
    QString log_path = settings_.value("log/path").toString();
    QUrl url;
    url.setScheme("file");
    url.setPath(log_path);
    QDesktopServices::openUrl(url);
}

void MainWindow::openWavFile()
{
    QString path = QFileDialog::getOpenFileName(
                this, "Open wav file", QDir::homePath(), "Wav (*.wav)");

    if (!path.isEmpty()) modem_->decodeWavFile(path);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings_.setValue("mainwindow/geometry", saveGeometry());
    settings_.setValue("mainwindow/windowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::initializeSettings()
{
    quint16 port = settings_.value("server/listen_port", DEFAULT_LISTEN_PORT).toUInt();
    settings_.setValue("server/listen_port", port);
    server_ = new SocketServer(port, this);

    QString default_path = QDir::homePath() + "/invasive.log.txt";
    QString log_path = settings_.value("log/path", default_path).toString();
    settings_.setValue("log/path", log_path);
}

void MainWindow::initializeMenuBar()
{
    QMenuBar* menu = new QMenuBar;
    setMenuBar(menu);

    QMenu* file_menu = new QMenu("File");
    menu->addMenu(file_menu);

    save_file_act_ = new QAction("Save as file...", this);
    connect(save_file_act_, SIGNAL(triggered()), this, SLOT(saveAsFile()));
    save_file_act_->setEnabled(false);
    save_file_act_->setShortcut(QKeySequence("Ctrl+S"));
    file_menu->addAction(save_file_act_);

    save_selected_act_ = new QAction("Save selected as file...", this);
    connect(save_selected_act_, SIGNAL(triggered()), this, SLOT(saveSelectedAsFile()));
    save_selected_act_->setEnabled(false);
    file_menu->addAction(save_selected_act_);

    QAction *open_logfile_act = new QAction("Open log file", this);
    connect(open_logfile_act, SIGNAL(triggered()), this, SLOT(openLogFile()));
    file_menu->addAction(open_logfile_act);

    file_menu->addSeparator();
    QAction *open_wav_act = new QAction(tr("Decode wav file..."), this);
    connect(open_wav_act, SIGNAL(triggered()), this, SLOT(openWavFile()));
    file_menu->addAction(open_wav_act);

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

    QMenu *tool_menu = menu->addMenu(tr("Tools"));
    QAction *configAct = new QAction(tr("Preferences..."), this);
    configAct->setShortcut(QKeySequence("Ctrl+K"));
    configAct->setMenuRole(QAction::PreferencesRole);
    connect(configAct, SIGNAL(triggered()), config_dialog_, SLOT(show()));
    tool_menu->addAction(configAct);

    QMenu *help_menu = menu->addMenu(tr("Help"));
    QAction *about_qt_act = new QAction(tr("About Qt..."), this);
    about_qt_act->setMenuRole(QAction::AboutQtRole);
    connect(about_qt_act, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    help_menu->addAction(about_qt_act);

    setInputDevice(settings_.value("input_device", 0).toUInt());
}
