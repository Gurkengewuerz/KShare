#include "notifications.hpp"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QStatusBar>
#include <QApplication>

#ifdef Q_OS_LINUX
#undef signals
extern "C"
{
#include <libnotify/notify.h>
}
#define signals public
#endif

void notifications::init() {
#ifdef Q_OS_LINUX
    notify_init("KShare");
#endif
}

void notifications::notify(QString title, QString body, QSystemTrayIcon::MessageIcon icon) {
    if (!MainWindow::inst() || !MainWindow::inst()->valid()) return;
    notifyNolog(title, body, icon);
    MainWindow::inst()->ui->logBox->addItem(title + ": " + body);
}

void notifications::notifyNolog(QString title, QString body, QSystemTrayIcon::MessageIcon icon) {

    if(icon == QSystemTrayIcon::Critical) {
        QApplication::alert(MainWindow::inst());
    }

#ifdef Q_OS_LINUX
    NotifyNotification *n = notify_notification_new(title.toLocal8Bit(), body.toLocal8Bit(), 0);
    notify_notification_set_timeout(n, 5000);

    NotifyUrgency urgency;
    switch(icon) {
        case QSystemTrayIcon::Warning:
            urgency = NotifyUrgency::NOTIFY_URGENCY_NORMAL;
            break;

        case QSystemTrayIcon::Critical:
            urgency = NotifyUrgency::NOTIFY_URGENCY_CRITICAL;
            break;

        default:
            urgency = NotifyUrgency::NOTIFY_URGENCY_LOW;
            break;
    }

    notify_notification_set_urgency(n, urgency);
    notify_notification_show(n, 0);
#else
    if (!MainWindow::inst()) return;
    MainWindow::inst()->tray->showMessage(title, body, icon, 5000);
#endif
    MainWindow::inst()->statusBar()->showMessage(title + ": " + body);
}
