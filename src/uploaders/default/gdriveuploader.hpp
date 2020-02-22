#ifndef GDRIVEUPLOADER_HPP
#define GRIVEUPLOADER_HPP

#include "../uploader.hpp"
#include <QApplication>
#include <logs/screenshotfile.h>

class GDriveUploader : public Uploader {
    Q_DECLARE_TR_FUNCTIONS(GDriveUploader)
    friend struct SegfaultWorkaround;

public:
    QString name() override {
        return "Google Drive";
    }
    QString description() override {
        return "drive.google.com uploader";
    }
    void doUpload(QByteArray byteArray, QString, ScreenshotFile sf) override;
    void showSettings() override;
    void setPermission(QString, QString, QString = "reader", QString = "anyone", bool = false);

private:
    void handleSend(QString auth, bool isPublic, QString mime, QByteArray byteArray, ScreenshotFile sf);
};

#endif // GDRIVEUPLOADER_HPP
