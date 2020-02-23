#include "gdriveuploader.hpp"

#include <QBuffer>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QString>
#include <formats.hpp>
#include <io/ioutils.hpp>
#include <notifications.hpp>
#include <settings.hpp>
#include <utils.hpp>
#include <logs/screenshotfile.h>
#include <uploaders/default/gdrivesettingsdialog.hpp>

struct SegfaultWorkaround { // I'm a scrub for doing this
    SegfaultWorkaround(QByteArray a, GDriveUploader *u, QString m, bool ip, ScreenshotFile f) : byteArray(), dis(u), mime(m), isPublic(ip), sf(f) {
        a.swap(byteArray);
        QJsonObject object;
        object.insert("client_id", settings::settings().value("google/cid").toString());
        object.insert("client_secret", settings::settings().value("google/csecret").toString());
        object.insert("grant_type", "refresh_token");
        object.insert("refresh_token", settings::settings().value("google/refresh").toString());

        ioutils::postJson(
        QUrl("https://accounts.google.com/o/oauth2/token"),
        QList<QPair<QString, QString>>({ QPair<QString, QString>("Content-Type", "applicaton/json") }),
        QJsonDocument::fromVariant(object.toVariantMap()).toJson(), [&](QJsonDocument response, QByteArray, QNetworkReply *r) {
            qDebug() << response;
            if (r->error() != QNetworkReply::NoError || !response.isObject()) {
                notifications::notify(QObject::tr("KShare Google Uploader"),
                                      QString(QObject::tr("Failed upload! Error in refresh token")));
                notifications::playSound(notifications::Sound::ERROR);
                return;
            }

            QJsonObject res = response.object();
            QString token = res["access_token"].toString();
            settings::settings().setValue("google/expire", QDateTime::currentDateTimeUtc().addSecs(res["expires_in"].toInt()));
            settings::settings().setValue("google/refresh", res["refresh_token"].toString());
            settings::settings().setValue("google/access", token);

            dis->handleSend(token.prepend("Bearer "), isPublic, mime, byteArray, sf);

            QScopedPointer<SegfaultWorkaround>(this);
        });
    }

private:
    QByteArray byteArray;
    GDriveUploader *dis;
    QString mime;
    bool isPublic;
    ScreenshotFile sf;
}; // I feel terrible for making this. I am sorry, reader

void GDriveUploader::doUpload(QByteArray byteArray, QString format, ScreenshotFile sf) {
    if (byteArray.size() > 1e+7) {
        notifications::notify(tr("KShare imgur Uploader"), tr("Failed upload! Image too big"));
        return;
    }
    QString mime;
    if (formats::normalFormatFromName(format) != formats::Normal::None)
        mime = formats::normalFormatMIME(formats::normalFormatFromName(format));
    else
        mime = formats::recordingFormatMIME(formats::recordingFormatFromName(format));

    if (settings::settings().contains("google/expire")     //
        && settings::settings().contains("google/refresh") //
        && settings::settings().contains("google/access")) {
        QDateTime expireTime = settings::settings().value("google/expire").toDateTime();
        bool isPublic = settings::settings().value("google/public", false).toBool();
        if (QDateTime::currentDateTimeUtc() > expireTime) {
            qDebug() << "Need to refresh Google Access Token";
            new SegfaultWorkaround(byteArray, this, mime, isPublic, sf);
        } else handleSend("Bearer " + settings::settings().value("google/access", "").toString().toUtf8(), isPublic, mime, byteArray, sf);
    }
}

void GDriveUploader::showSettings() {
    (new GDriveSettingsDialog())->show();
}

void GDriveUploader::setPermission(QString fileID, QString auth, QString role, QString type, bool allowFileDiscovery) {
    QJsonObject object;
    object.insert("role", role);
    object.insert("type", type);
    object.insert("allowFileDiscovery", QString(allowFileDiscovery));
    QJsonDocument doc(object);

    ioutils::postData(QUrl(QString("https://www.googleapis.com/drive/v3/files/%1/permissions").arg(fileID)),
                      QList<QPair<QString, QString>>({ QPair<QString, QString>("Content-Type", "application/json"), QPair<QString, QString>("Authorization", auth) }),
                      QString(doc.toJson(QJsonDocument::Compact)).toUtf8(),
                      [&](QByteArray , QNetworkReply *) {

    });
}

void GDriveUploader::handleSend(QString auth, bool isPublic, QString mime, QByteArray byteArray, ScreenshotFile sf) {
    QHttpMultiPart *multipart = new QHttpMultiPart(QHttpMultiPart::RelatedType);

    QHttpPart metaPart;
    metaPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    metaPart.setHeader(
                QNetworkRequest::ContentDispositionHeader,
                QVariant("form-data; name=\"metadata\"")
                );
    QString data("{name: '"+sf.getFilename()+"', parents: ['"+settings::settings().value("google/folder").toString().toUtf8()+"']}");
    metaPart.setBody(data.toUtf8());
    multipart->append(metaPart);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(mime.toUtf8()));
    imagePart.setHeader(
                QNetworkRequest::ContentDispositionHeader,
                QVariant("form-data; name=\"file\"; filename=\""+sf.getFilename()+"\"")
                );

    imagePart.setBody(byteArray);
    multipart->append(imagePart);

    ioutils::postMultipartData(QUrl("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart&fields=id,webViewLink,webContentLink"),
                      QList<QPair<QString, QString>>() << QPair<QString, QString>("Authorization", auth),
                      multipart, [byteArray, this, mime, sf, auth, isPublic](QByteArray res, QNetworkReply *r) {
                          qDebug() << QString(res).toUtf8();
                          if (r->error() == QNetworkReply::ContentAccessDenied) {
                              (new GDriveSettingsDialog())->show();
                              return;
                          }

                          QString result = QString(res).toUtf8();
                          if (!result.isEmpty()) {
                              QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
                              if(!doc.isObject()) {
                                  notifications::notify(tr("KShare Google Uploader "),
                                                        QString(tr("Failed upload! Invalid JSON")));
                                  notifications::playSound(notifications::Sound::ERROR);
                                  return;
                              }

                              if(isPublic) {
                                  setPermission(doc.object()["id"].toString(), auth);
                              }
                              result = doc.object()["webViewLink"].toString();
                              ioutils::addLogEntry(r, res, result, sf);

                              notifications::notify(tr("KShare Google Uploader"), tr("Uploaded to Google!"));
                              notifications::playSound(notifications::Sound::SUCCESS);
                              utils::toClipboard(result);
                          } else {

                              ioutils::addLogEntry(r, res, result, sf);
                              notifications::notify(tr("KShare Google Uploader "),
                                                    QString(tr("Failed upload! Google said: HTTP %1: %2"))
                                                    .arg(r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
                                                    .arg(r->errorString()));
                              notifications::playSound(notifications::Sound::ERROR);
                          }
                      });
}
