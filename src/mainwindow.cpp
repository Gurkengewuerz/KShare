#include "mainwindow.hpp"
#include "aboutbox.hpp"
#include "screenshotter.hpp"
#include "settingsdialog.hpp"
#include "ui_mainwindow.h"
#include "utils.hpp"
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QShortcut>
#include <colorpicker/colorpickerscene.hpp>
#include <formats.hpp>
#include <hotkeying.hpp>
#include <logger.hpp>
#include <platformbackend.hpp>
#include <recording/recordingformats.hpp>
#include <settings.hpp>
#include <uploaders/uploadersingleton.hpp>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QStandardPaths>
#include <QDesktopServices>
#include <logs/requestlogging.hpp>
#include "io/ioutils.hpp"
#include <monospacetextdialog.hpp>
#include <clipboard/clipboardcopy.hpp>
#include <logs/screenshotfile.h>
#include <QDateTime>

MainWindow *MainWindow::instance;
qint8 trayIconClicks = 0;
bool wasDoubleCLick = false;

using requestlogging::LoggedRequest;

void MainWindow::rec() {
    if (controller->isRunning()) return;
    auto f = static_cast<formats::Recording>(
    settings::settings().value("recording/format", static_cast<int>(formats::Recording::None)).toInt());
    if (f >= formats::Recording::None) {
        logger::warn(tr("Recording format not set in settings. Aborting."));
        return;
    }
    RecordingContext *ctx = new RecordingContext;
    RecordingFormats *format = new RecordingFormats(f);
    ctx->consumer = format->getConsumer();
    ctx->finalizer = format->getFinalizer();
    ctx->validator = format->getValidator();
    ctx->format = format->getFormat();
    ctx->postUploadTask = format->getPostUploadTask();
    ctx->anotherFormat = format->getAnotherFormat();
    controller->start(ctx);
}

#define ACTION(english, menu)                                                                                          \
    [&]() -> QAction * {                                                                                               \
        QAction *a = menu->addAction(english);                                                                         \
        return a;                                                                                                      \
    }()

void addHotkey(QString name, std::function<void()> action) {
    hotkeying::load(name, action);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    instance = this;
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/icon.png"));
    tray = new QSystemTrayIcon(windowIcon(), this);
    tray->setToolTip("KShare");
    tray->setVisible(true);
    menu = new QMenu(this);
    QAction *shtoggle = ACTION(tr("Show/Hide"), menu);
    QAction *fullscreen = ACTION(tr("Take fullscreen shot"), menu);
    QAction *area = ACTION(tr("Take area shot"), menu);

#ifdef PLATFORM_CAPABILITY_ACTIVEWINDOW
    QAction *active = ACTION(tr("Screenshot active window"), menu);
    connect(active, &QAction::triggered, this, [] { screenshotter::activeDelayed(); });
#endif
    QAction *copyClipboard = ACTION(tr("Copy from Clipbaord"), menu);
    QAction *picker = ACTION(tr("Show color picker"), menu);
    QAction *rec = ACTION(tr("Record screen"), menu);
    QAction *recoff = ACTION(tr("Stop recording"), menu);
    QAction *recabort = ACTION(tr("Abort recording"), menu);
    menu->addActions({ shtoggle, picker });
    menu->addSeparator();
    menu->addActions({ fullscreen, area, copyClipboard });
#ifdef PLATFORM_CAPABILITY_ACTIVEWINDOW
    menu->addAction(active);
#endif
    menu->addSeparator();
    menu->addActions({ rec, recoff, recabort });
    
    QAction *quit = ACTION(tr("Quit"), menu);
    QAction *about = ACTION(tr("About"), menu);
    menu->addSeparator();
    menu->addActions({ about, quit });
    
    connect(quit, &QAction::triggered, this, &MainWindow::quit);
    connect(shtoggle, &QAction::triggered, this, &MainWindow::toggleVisible);
    connect(picker, &QAction::triggered, [] { ColorPickerScene::showPicker(); });
    connect(tray, &QSystemTrayIcon::messageClicked, this, &QWidget::show);

    connect(tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::MiddleClick) {
            screenshotter::fullscreenDelayed();
            return;
        }


        if (reason == QSystemTrayIcon::DoubleClick) {
            wasDoubleCLick = true;
            toggleVisible();
            return;
        }

        if (reason != QSystemTrayIcon::Trigger) return;

        trayIconClicks++;
        if (trayIconClicks == 1) {
            QTimer::singleShot(QApplication::doubleClickInterval(), [this] {
                if(wasDoubleCLick) {
                    wasDoubleCLick = false;
                    return;
                }
                if(trayIconClicks == 1)  {
                    // Single Click
                    screenshotter::areaDelayed();
                } else {
                    // Double Click
                    toggleVisible();
                }
                trayIconClicks = 0;
            });
        }
    });
    connect(fullscreen, &QAction::triggered, this, [] { screenshotter::fullscreenDelayed(); });
    connect(area, &QAction::triggered, this, [] { screenshotter::areaDelayed(); });
    connect(copyClipboard, &QAction::triggered, this, &clipboardcopy::copyClipboard);
    connect(rec, &QAction::triggered, this, &MainWindow::rec);
    connect(recoff, &QAction::triggered, controller, &RecordingController::end);
    connect(recabort, &QAction::triggered, controller, &RecordingController::abort);
    connect(about, &QAction::triggered, this, &MainWindow::on_actionAbout_triggered);

    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::on_actionSettings_triggered);
    connect(ui->fullscreenButton, &QPushButton::clicked, this, [] { screenshotter::fullscreenDelayed(); });
    connect(ui->areaButton, &QPushButton::clicked, this, [] { screenshotter::areaDelayed(); });
    connect(ui->aboutButton, &QPushButton::clicked, this, &MainWindow::on_actionAbout_triggered);
    connect(ui->screenshotFolderButton, &QPushButton::clicked, this, &MainWindow::openScreenshotFolder);
    connect(ui->clipboardButton, &QPushButton::clicked, this, &clipboardcopy::copyClipboard);
    connect(ui->colorPickerButton, &QPushButton::clicked, this, [] { ColorPickerScene::showPicker(); });

    ui->treeWidget->addAction(ui->actionOpenURL);
    ui->treeWidget->addAction(ui->actionOpenLocalFile);
    ui->treeWidget->addAction(ui->actionOpenRequest);
    ui->treeWidget->addAction(ui->actionCopyLinktoClipboard);

    ui->aboutButton->setFocus();

    tray->setContextMenu(menu);

    addHotkey("fullscreen", [] { screenshotter::fullscreen(); });
    addHotkey("area", [] { screenshotter::area(); });
    addHotkey("active", [] { screenshotter::active(); });
    addHotkey("picker", [] { ColorPickerScene::showPicker(); });
    addHotkey("clipboard", [] { clipboardcopy::copyClipboard(); });
    addHotkey("recordingstop", [&] { controller->end(); });
    addHotkey("recordingabort", [&] { controller->abort(); });
    addHotkey("recordingstart", [&] { this->rec(); });

    auto errors = UploaderSingleton::inst().errors();
    for (auto err : errors) ui->logBox->addItem(QString("ERROR: ") + err.what());
    setWindowTitle("KShare v" + QApplication::applicationVersion());
    val = true;

    QList<LoggedRequest> requests = requestlogging::getRequests();
    for (LoggedRequest req : requests) {
        addResponse(req.getResponseCode(), req.getScreenshotFile(), req.getResult(), req.getUrl(), req.getTime());
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::valid() {
    return val;
}

MainWindow *MainWindow::inst() {
    return instance;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (settings::settings().value("hideOnClose", true).toBool()) {
        event->ignore();
        hide();
    } else
        QApplication::exit(0);
}

void MainWindow::quit() {
    QCoreApplication::quit();
}

void MainWindow::toggleVisible() {
    this->setVisible(!this->isVisible());
    if (this->isVisible()) {
        this->raise();                          // that didn't work
        this->setWindowState(Qt::WindowActive); // maybe that works
        this->activateWindow();                 // maybe that works
    }
}

void MainWindow::on_actionQuit_triggered() {
    quit();
}

void MainWindow::on_actionStart_triggered() {
    rec();
}

void MainWindow::on_actionStop_triggered() {
    controller->end();
}

void MainWindow::on_actionSettings_triggered() {
    SettingsDialog *dialog = new SettingsDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::on_actionAbout_triggered() {
    AboutBox *box = new AboutBox(this);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->show();
}

void MainWindow::on_actionAbort_triggered() {
    controller->abort();
}

void MainWindow::on_actionOpenRequest_triggered() {
    QString file = ui->treeWidget->currentItem()->text(4);
    file = settings::dir().absoluteFilePath("responses/" + file.left(file.length() - 4));

    QFile dataFile(file);
    if (!dataFile.open(QIODevice::ReadOnly)) {
        logger::info(file);
        logger::error(dataFile.errorString());
        return;
    }
    MonospaceTextDialog *dialog = new MonospaceTextDialog(file, dataFile.readAll());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::on_actionOpenURL_triggered() {
    QDesktopServices::openUrl(QUrl(ui->treeWidget->currentItem()->text(2)));
}

void MainWindow::on_actionOpenLocalFile_triggered() {
    // TODO: Cleanup code, because this switch function is used 3 times
    QString file = ui->treeWidget->currentItem()->text(1);
    QDir saveDir;
    switch (settings::settings().value("saveLocation", 1).toInt()) {
        case 0:
            saveDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
            if (QStandardPaths::writableLocation(QStandardPaths::PicturesLocation).isEmpty()) {
                qFatal("%s", tr("Cannot determine location for pictures").toLocal8Bit().constData());
            }
            break;
        case 1:
            if (QStandardPaths::writableLocation(QStandardPaths::HomeLocation).isEmpty()) {
                qFatal("%s", tr("Cannot determine location of your home directory").toLocal8Bit().constData());
            }
            saveDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Screenshots";
            break;
        default:
            qFatal("%s", tr("Invalid config [saveLocation not int or is not in range]").toLocal8Bit().constData());
            return;
        case 2:
            // Do not Save images
            return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(saveDir.absoluteFilePath(file)));
}

void MainWindow::on_actionCopyLinktoClipboard_triggered() {
    QApplication::clipboard()->setText(ui->treeWidget->currentItem()->text(2));
}

void MainWindow::on_treeWidget_doubleClicked(const QModelIndex &) {
    on_actionOpenURL_triggered();
}

void MainWindow::openScreenshotFolder() {
    QDir saveDir;
    switch (settings::settings().value("saveLocation", 1).toInt()) {
        case 0:
            saveDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
            if (QStandardPaths::writableLocation(QStandardPaths::PicturesLocation).isEmpty()) {
                qFatal("%s", tr("Cannot determine location for pictures").toLocal8Bit().constData());
            }
            break;
        case 1:
            if (QStandardPaths::writableLocation(QStandardPaths::HomeLocation).isEmpty()) {
                qFatal("%s", tr("Cannot determine location of your home directory").toLocal8Bit().constData());
            }
            saveDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Screenshots";
            break;
        default:
            qFatal("%s", tr("Invalid config [saveLocation not int or is not in range]").toLocal8Bit().constData());
            return;
        case 2:
            // Do not Save images
            return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(saveDir.absolutePath()));
}

void MainWindow::setTrayIcon(QIcon icon) {
    tray->setIcon(icon);
}

void MainWindow::addResponse(int httpCode, ScreenshotFile sf, QString result, QString url, QString time) {
    QString httpStatus = ioutils::httpString(httpCode);
    QTreeWidgetItem* tw = new QTreeWidgetItem({ QString::number(httpCode) + " " + httpStatus, sf.getSubfolder() + QDir::separator() + sf.getFilename(), result, url, time + " UTC" });

    if(httpCode >= 200 && httpCode < 300) {
        tw->setIcon(0, *(new QIcon(":/icons/checked.png")));
    } else {
        tw->setIcon(0, *(new QIcon(":/icons/error.png")));
    }

    ui->treeWidget->insertTopLevelItem(0, tw);
}
