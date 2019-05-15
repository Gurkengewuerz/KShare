#ifndef CLIPBOARDUPLOADER_HPP
#define CLIPBOARDUPLOADER_HPP

#include <QApplication>
#include <QPixmap>
#include <uploaders/uploader.hpp>
#include <logs/screenshotfile.h>

class ClipboardUploader : public Uploader {
    Q_DECLARE_TR_FUNCTIONS(ClipboardUploader)
public:
    QString name() {
        return "clipboard";
    }
    QString description() {
        return "Copies the image to clipboard";
    }

    void doUpload(QByteArray imgData, QString format, ScreenshotFile sf);
};

#endif // CLIPBOARDUPLOADER_HPP
