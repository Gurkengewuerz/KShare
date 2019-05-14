#include "notifications.hpp"

#include "systemnotification.h"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QMediaPlayer>

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

void notifications::playSound(notifications::Sound soundType) {
    QMediaPlayer*mediaPlayer = new QMediaPlayer(MainWindow::inst());

    switch (soundType) {
    case notifications::Sound::CAPTURE:
        mediaPlayer->setMedia(QUrl("qrc:/capturesound.wav"));
        break;

    case notifications::Sound::SUCCESS:
        mediaPlayer->setMedia(QUrl("qrc:/successsound.wav"));
        break;

    case notifications::Sound::ERROR:
        mediaPlayer->setMedia(QUrl("qrc:/errorsound.wav"));
        break;

    default:
        break;
    }

    mediaPlayer->setVolume(25);
    mediaPlayer->play();

    if(mediaPlayer->error() != QMediaPlayer::NoError && mediaPlayer->error() != QMediaPlayer::ServiceMissingError)
        notifications::notify(QString::number(mediaPlayer->error()), mediaPlayer->errorString(), QSystemTrayIcon::Warning);

}
