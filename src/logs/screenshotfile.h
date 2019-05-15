#ifndef SCREENSHOTFILE_H
#define SCREENSHOTFILE_H

#include <QByteArray>
#include <QNetworkReply>
#include <QString>
#include <settings.hpp>
#include <QFile>


class ScreenshotFile {

    public:
        QString getSubfolder() {
            return subfolder;
        }
        QString getFilename() {
            return filename;
        }

        QString subfolder;
        QString filename;
};


#endif // SCREENSHOTFILE_H
