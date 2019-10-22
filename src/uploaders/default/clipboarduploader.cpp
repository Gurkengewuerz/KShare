#include "clipboarduploader.hpp"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <formats.hpp>
#include <notifications.hpp>
#include <QString>
#include <logs/screenshotfile.h>

void ClipboardUploader::doUpload(QByteArray imgData, QString format, ScreenshotFile sf) {
    auto f = formats::recordingFormatFromName(format);
    if (f != formats::Recording::None) {
        auto data = new QMimeData();
        data->setData(formats::recordingFormatMIME(f), imgData);
        QApplication::clipboard()->setMimeData(data);
    } else
        QApplication::clipboard()->setImage(QImage::fromData(imgData, format.toLocal8Bit().constData()));
    notifications::notify("KShare", tr("Copied to clipboard!"));
}
