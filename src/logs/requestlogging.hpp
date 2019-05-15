#ifndef REQUESTLOGGING_HPP
#define REQUESTLOGGING_HPP

#include <QByteArray>
#include <QNetworkReply>
#include <QString>
#include <settings.hpp>
#include <logs/screenshotfile.h>

namespace requestlogging {
    struct RequestContext {
        QByteArray response;
        QNetworkReply *reply;
        ScreenshotFile screenshotFile;
        QString result;
    };

    class LoggedRequest {
        friend QList<LoggedRequest> getRequests();

    public:
        QString getUrl() {
            return url;
        }
        ScreenshotFile getScreenshotFile() {
            return screenshotFile;
        }
        QString getType() {
            return type;
        }
        QString getResult() {
            return result;
        }
        QString getTime() {
            return time;
        }
        int getResponseCode() {
            return responseCode;
        }
        QByteArray getResponse() {
            return QFile(settings::dir().absoluteFilePath("responses/" + time)).readAll();
        }

    private:
        QString url;
        ScreenshotFile screenshotFile;
        QString result;
        QString type;
        QString time;
        int responseCode;
    };

    QList<LoggedRequest> getRequests();
    void addEntry(RequestContext context);

    namespace indicator {
        void show(int count);
    } // namespace indicator
} // namespace requestlogging

#endif // REQUESTLOGGING_HPP
