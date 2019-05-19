#include "requestlogging.hpp"
#include <QDateTime>
#include <mainwindow.hpp>
#include <io/ioutils.hpp>
#include <logs/screenshotfile.h>
#include <utils.hpp>

#include "mainwindow.hpp"

// $type $url $status $time
// $type = GET POST PATCH DELETE etc
// $url = request target
// $status = response code
// $time = time of request, file name for response: $SETTINGS_DIR/responses/$time

QDir responses(settings::dir().absoluteFilePath("responses"));
QString requestPath = settings::dir().absoluteFilePath("history");


void requestlogging::addEntry(RequestContext context) {
    if (!responses.exists()) responses.mkpath(".");
    QString timeNow = QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd HH-mm-ss-zzz");
    QFile responseFile(responses.absoluteFilePath(timeNow));
    QFile requestFile(requestPath);

    if (!responseFile.open(QIODevice::WriteOnly)) {
        qCritical().noquote() << "Could not save response! " + responseFile.errorString();
        return;
    }

    if (!requestFile.open(QIODevice::Append)) {
        qCritical().noquote() << "Could not append request! " + responseFile.errorString();
        return;
    }

    for (auto header : context.reply->rawHeaderList())
        responseFile.write(header + ": " + context.reply->rawHeader(header) + "\n");
    responseFile.write("\n\n" + context.response);
    responseFile.close();

    ScreenshotFile sf = context.screenshotFile;

    QTextStream(&requestFile) << ioutils::methodString(context.reply->operation()) << " "   // $type
                              << context.reply->url().toString().replace(" ", "%20") << " " // $url
                              << context.result.replace(" ", "%20") << " " // $result
                              << sf.getSubfolder().replace(" ", "_") << " " // $subfolder
                              << sf.getFilename().replace(" ", "_") << " " // $filename
                              << context.reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() << " " // $status
                              << timeNow.replace(" ", "_") << endl
                              << flush; // $time
    requestFile.close();

    MainWindow::inst()->addResponse(
            context.reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
            sf,
            context.result,
            context.reply->url().toString(),
            timeNow.replace("_", " "));
}

using requestlogging::LoggedRequest;

QList<LoggedRequest> requestlogging::getRequests() {
    QList<LoggedRequest> ret;

    QFile requestFile(requestPath);
    if (!requestFile.exists() || !requestFile.open(QIODevice::ReadOnly)) return ret;
    QByteArray line;

    requestFile.seek(requestFile.size());
    long int pos = requestFile.pos();
    int count = 0;

    while(pos) {
        requestFile.seek(--pos);
        QString s = requestFile.read(1);
        if(s == '\n') {
            if(count++ == 8) break;
        }
    }

    while ((line = requestFile.readLine()).size() != 0) {
        LoggedRequest r;
        QTextStream stream(&line);
        ScreenshotFile sf;
        stream >> r.type;
        stream >> r.url;
        stream >> r.result;
        stream >> sf.subfolder;
        stream >> sf.filename;
        stream >> r.responseCode;
        stream >> r.time;
        r.time = r.time.replace("_", " ");
        sf.subfolder = sf.subfolder.replace("_", " ");
        sf.filename = sf.filename.replace("_", " ");
        r.screenshotFile = sf;
        ret.append(r);
    }

    requestFile.close();

    return ret;
}

void requestlogging::indicator::show(int count) {
    MainWindow::inst()->setTrayIcon(utils::getTrayIcon(count));
}
