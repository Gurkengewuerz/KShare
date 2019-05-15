#ifndef UPLOADER_HPP
#define UPLOADER_HPP

#include <QPixmap>
#include <QString>
#include <logs/screenshotfile.h>

class Uploader {
public:
    virtual void doUpload(QByteArray imgData, QString format, ScreenshotFile sf) = 0;
    virtual QString name() = 0;
    virtual QString description() = 0;
    virtual void showSettings() {
    }
    virtual bool validate() {
        return true;
    }
};

#endif // UPLOADER_HPP
