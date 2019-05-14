#ifndef CUSTOMUPLOADER_HPP
#define CUSTOMUPLOADER_HPP

#include "uploader.hpp"
#include <QApplication>
#include <QJsonObject>
#include <QMap>
#include <QUrl>
#include <QNetworkReply>

enum class HttpMethod { POST };

enum class RequestFormat { X_WWW_FORM_URLENCODED, JSON, MULTIPART_FORM_DATA, PLAIN };

class CustomUploader : public Uploader {
    Q_DECLARE_TR_FUNCTIONS(CustomUploader)

public:
    CustomUploader(QString absFilePath);
    QString name();
    QString description();
    void doUpload(QByteArray imgData, QString format, QString filename);

private:
    double limit = -1;
    QString desc;
    QString uName;
    RequestFormat rFormat = RequestFormat::JSON;
    HttpMethod method = HttpMethod::POST;
    QUrl target;
    QJsonValue body;
    QJsonObject headers;
    bool base64 = false;
    QString returnPathspec;
    QString urlPrepend, urlAppend;
    void parseResult(QNetworkReply *r, QJsonDocument result, QByteArray data, QString returnPathspec, QString name, QString filename);
};

#endif // CUSTOMUPLOADER_HPP
