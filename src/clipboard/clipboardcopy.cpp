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
#include <QUrl>
#include <logger.hpp>
#include <io/ioutils.hpp>
#include <notifications.hpp>
#include <uploaders/uploadersingleton.hpp>

void clipboardcopy::copyClipboard() {
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if(mimeData->hasImage()) {
        QPixmap map = qvariant_cast<QPixmap>(mimeData->imageData());
    } else if(mimeData->hasText()) {
#ifdef Q_OS_WIN
        QUrl fileUrl(mimeData->text());
        QFileInfo fileInfo(fileUrl.toLocalFile());
#else
        QFileInfo fileInfo(mimeData->text());
#endif
        if(fileInfo.exists() && fileInfo.isReadable() && fileInfo.isFile()) {
            QMimeDatabase db;
            QMimeType mimeType = db.mimeTypeForFile(fileInfo);
            QString type = mimeType.name();
            QFile file(fileInfo.absoluteFilePath());
            UploaderSingleton::inst().upload(file);
        } else if (fileInfo.exists() && fileInfo.isReadable() && fileInfo.isDir()) {
            notifications::notify("KShare - Directory is not uploadable", fileInfo.absolutePath(), QSystemTrayIcon::Warning);
            notifications::playSound(notifications::Sound::ERROR);
        } else {
            QTemporaryFile tmpFile;
            tmpFile.setAutoRemove(true);
            if(tmpFile.open()) {
                QTextStream stream(&tmpFile);
                stream.setCodec("UTF-8");
                stream << mimeData->text();
                stream.flush();
                tmpFile.seek(0);
                UploaderSingleton::inst().upload(tmpFile);
            } else {
                logger::warn("Can not open tmp file");
                notifications::playSound(notifications::Sound::ERROR);
            }
        }
    } else {
        notifications::notify("Unsupported File Format", "Can not upload clipboard", QSystemTrayIcon::Warning);
        notifications::playSound(notifications::Sound::ERROR);
    }
}
