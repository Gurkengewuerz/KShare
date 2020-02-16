#include "mainwindow.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QScreen>
#include <QtGlobal>
#include <formatter.hpp>
#include <iostream>
#include <ui_mainwindow.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <QListWidget>
#include <QTranslator>
#include <QStyleFactory>
#include <QSettings>
#include <QPalette>
#include <QColor>
#include <notifications.hpp>
#include <worker/worker.hpp>
#include <settings.hpp>

bool verbose = false;

// I've experiments to run
// There is research to be done
// On the people who are
// still alive
bool stillAlive = true;

#define LOGACT(lvl) std::cout << lvl << stdMsg << "\n" << std::flush;
void handler(QtMsgType type, const QMessageLogContext &, const QString &msg) {
    if (!verbose && msg.startsWith("QPixmap::fromWinHBITMAP")) return;
    std::string stdMsg = msg.toStdString();
    switch (type) {
    case QtDebugMsg:
        if (verbose) {
            LOGACT("DEBUG: ")
        }
        break;
    case QtInfoMsg:
        LOGACT("INFO: ")
        break;
    case QtWarningMsg:
        LOGACT("WARN: ")
        break;
    case QtCriticalMsg:
        LOGACT("CRIT: ")
        break;
    case QtFatalMsg:
        LOGACT("FATAL: ")
        break;
    }
}

void loadTranslation(QString locale) {
    QFile resource(":/translations/" + locale + ".qm");
    if (!resource.exists()) return;
    resource.open(QIODevice::ReadOnly);

    QTranslator *translator = new QTranslator;
    QByteArray file = resource.readAll();
    QByteArray *permFile = new QByteArray;
    permFile->swap(file);
    translator->load((const unsigned char *)permFile->constData(), permFile->size());
    QApplication::installTranslator(translator);
}

int main(int argc, char *argv[]) {
    av_register_all();
    qInstallMessageHandler(handler);
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    a.setApplicationName("KShare");
    a.setOrganizationName("ArsenArsen");
    a.setApplicationVersion("5.0.1");

    QString locale = QLocale::system().name();
    if (locale != "en_US") loadTranslation(locale);

    int theme = settings::settings().value("theme", 0).toInt();

    if(theme == 0) {
        // System Default
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",QSettings::NativeFormat);
    if(settings.value("AppsUseLightTheme")==0){
        qApp->setStyle(QStyleFactory::create("Fusion"));
        QPalette darkPalette;
        QColor darkColor = QColor(45,45,45);
        QColor disabledColor = QColor(127,127,127);
        darkPalette.setColor(QPalette::Window, darkColor);
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(18,18,18));
        darkPalette.setColor(QPalette::AlternateBase, darkColor);
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        darkPalette.setColor(QPalette::Button, darkColor);
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

        qApp->setPalette(darkPalette);

        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    }
#endif
    } else {
        QString path = "";

        if(theme == 1) {
            // QDarkStyle
            path = ":qdarkstyle/style.qss";
        } else if(theme == 2) {
            // Breeze Light
            path = ":/light.qss";
        } else if(theme == 3) {
            // Breeze Dark
            path = ":/dark.qss";
        }

        QFile f(path);
        if (!f.exists())
        {
            printf("Unable to set stylesheet, file not found\n");
        }
        else
        {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            qApp->setStyleSheet(ts.readAll());
        }
    }

    QCommandLineParser parser;
    parser.addHelpOption();

    QCommandLineOption h({ "b", "background" }, "Does not show the main window, starts in tray.");
    QCommandLineOption v({ "v", "verbose" }, "Enables QtDebugMsg outputs");
    QCommandLineOption ver({ "ver", "version" }, "Prints KShare version");
    parser.addOption(h);
    parser.addOption(v);
    parser.addOption(ver);
    parser.process(a);

    if (parser.isSet(ver)) {
        printf("%s %s\n", a.applicationName().toLocal8Bit().constData(), a.applicationVersion().toLocal8Bit().constData());
        return 0;
    }

    verbose = parser.isSet(v);
    MainWindow w;
    Worker::init();
    a.connect(&a, &QApplication::aboutToQuit, Worker::end);
    a.connect(&a, &QApplication::aboutToQuit, [] { stillAlive = false; });

    if (!parser.isSet(h)) w.show();
    return a.exec();
}
