#include "notifications.hpp"

#include "systemnotification.h"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QApplication>

void notifications::notify(QString title, QString body, QSystemTrayIcon::MessageIcon icon) {
    if (!MainWindow::inst() || !MainWindow::inst()->valid()) return;
    notifyNolog(title, body, icon);
    MainWindow::inst()->ui->logBox->addItem(title + ": " + body);
}

void notifications::notifyNolog(QString title, QString body, QSystemTrayIcon::MessageIcon icon) {

    if(icon == QSystemTrayIcon::Critical) {
        QApplication::alert(MainWindow::inst());
    }

    SystemNotification().sendMessage(body, title, icon);

    MainWindow::inst()->statusBar()->showMessage(title + ": " + body);
}
