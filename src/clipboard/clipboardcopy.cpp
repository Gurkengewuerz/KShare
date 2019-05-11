#include "clipboardcopy.hpp"
#include <QClipboard>
#include <QMimeData>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QMimeDatabase>
#include <QPixmap>
#include <QTemporaryFile>
#include <QIODevice>
#include <QTextStream>
#include <QMediaPlayer>
#include <QUrl>
#include <logger.hpp>
#include <io/ioutils.hpp>
#include <notifications.hpp>
#include <uploaders/uploadersingleton.hpp>
#include "mainwindow.hpp"

void clipboardcopy::copyClipboard() {
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if(mimeData->hasImage()) {
        QPixmap map = qvariant_cast<QPixmap>(mimeData->imageData());
    } else if(mimeData->hasText()) {
        QFileInfo fileInfo(mimeData->text());
        if(fileInfo.exists() && fileInfo.isReadable() && fileInfo.isFile()) {
            QMimeDatabase db;
            QMimeType mimeType = db.mimeTypeForFile(fileInfo);
            QString type = mimeType.name();
            QFile file(fileInfo.absoluteFilePath());
            logger::info(type);
            UploaderSingleton::inst().upload(file);
        } else if (fileInfo.exists() && fileInfo.isReadable() && fileInfo.isDir()) {
            notifications::notify("KShare - Directory is not uploadable", fileInfo.absolutePath(), QSystemTrayIcon::Warning);
            playErrorSound();
        } else {
            QTemporaryFile tmpFile;
            tmpFile.setAutoRemove(true);
            if(tmpFile.open()) {
                QTextStream stream(&tmpFile);
                stream << mimeData->text();
                stream.flush();
                tmpFile.seek(0);
                UploaderSingleton::inst().upload(tmpFile);
            } else {
                logger::warn("Can not open tmp file");
                playErrorSound();
            }
        }
    } else {
        notifications::notify("Unsupported File Format", "Can not upload clipboard", QSystemTrayIcon::Warning);
        playErrorSound();
    }
}

void clipboardcopy::playErrorSound() {
    QMediaPlayer*mediaPlayer = new QMediaPlayer(MainWindow::inst());
    mediaPlayer->setMedia(QUrl("qrc:/errorsound.wav"));
    mediaPlayer->setVolume(50);
    mediaPlayer->play();

    if(mediaPlayer->error() != QMediaPlayer::NoError && mediaPlayer->error() != QMediaPlayer::ServiceMissingError)
        notifications::notify(QString::number(mediaPlayer->error()), mediaPlayer->errorString(), QSystemTrayIcon::Warning);
}