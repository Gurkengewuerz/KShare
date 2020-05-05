#include "notifications.hpp"

#include "systemnotification.h"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <settings.hpp>
#include <QApplication>
#include <QMediaPlayer>

QMediaPlayer *mediaPlayer = nullptr;

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
    if(!settings::settings().value("playSound", true).toBool()) return;

    try {
        if (mediaPlayer == nullptr) {
            mediaPlayer = new QMediaPlayer(MainWindow::inst());
        }

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
    } catch (...) {
        notifications::notifyNolog(QObject::tr("KShare: No sound driver"), "No sound driver found. Install libqt5multimedia5-plugins for notifcation sound support.", QSystemTrayIcon::Warning);
    }
}
