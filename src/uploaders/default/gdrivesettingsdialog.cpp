#include "gdrivesettingsdialog.hpp"
#include "ui_gdrivesettingsdialog.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QPushButton>
#include <QUrl>
#include <io/ioutils.hpp>
#include <settings.hpp>

GDriveSettingsDialog::GDriveSettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::GDriveSettingsDialog) {
    ui->setupUi(this);
    connect(this, &GDriveSettingsDialog::accepted, this, &GDriveSettingsDialog::deleteLater);
    ui->clientId->setText(settings::settings().value("google/cid").toString());
    ui->clientSecret->setText(settings::settings().value("google/csecret").toString());
    ui->folderId->setText(settings::settings().value("google/folder").toString().toUtf8());

    ui->isPublic->setChecked(settings::settings().value("google/public").toBool());
}

GDriveSettingsDialog::~GDriveSettingsDialog() {
    delete ui;
}

void GDriveSettingsDialog::on_addApp_clicked() {
    QDesktopServices::openUrl(
    QUrl(QString("https://accounts.google.com/o/oauth2/auth?client_id=%1&redirect_uri=urn:ietf:wg:oauth:2.0:oob&response_type=code&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fdrive").arg(ui->clientId->text())));
}

void GDriveSettingsDialog::on_isPublic_clicked(bool checked) {
    settings::settings().setValue("google/public", checked);
}

void GDriveSettingsDialog::on_folderId_textChanged(QString arg1) {
    settings::settings().setValue("google/folder", arg1);
}

bool GDriveSettingsDialog::checkAuthorization () {
    if (ui->folderId->text().isEmpty()) return false;
    settings::settings().setValue("google/folder", ui->folderId->text());

    if (settings::settings().contains("google/expire")     //
        && settings::settings().contains("google/refresh") //
        && settings::settings().contains("google/access")) {
        QDateTime expireTime = settings::settings().value("google/expire").toDateTime();
        if (QDateTime::currentDateTimeUtc() > expireTime) {
            // Token expired
            ui->status->setText(tr("Token expired"));
        } else {
            ui->buttonBox->setEnabled(false);
            ui->testButton->setEnabled(false);
            ioutils::getData(QUrl(QString("https://www.googleapis.com/drive/v3/files?q='%1'+in+parents&fields=files(md5Checksum,+originalFilename)").arg(ui->folderId->text())),
                              QList<QPair<QString, QString>>({ QPair<QString, QString>("Authorization", settings::settings().value("google/access").toString().prepend("Bearer ")) }),
                              [&](QByteArray a, QNetworkReply *r) {

                        QVariant statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                        if (!statusCode.isValid()) return;

                        ui->testButton->setEnabled(true);

                        int status = statusCode.toInt();
                        if (status != 200 && status != 400)
                        {
                            QString reason = r->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
                            qDebug() << reason;

                            ui->buttonBox->setEnabled(true);
                            ui->status->setText(reason);
                            return;
                        }

                          QJsonDocument response = QJsonDocument::fromJson(a);
                          if (!response.isObject()) {
                              ui->buttonBox->setEnabled(true);
                              ui->status->setText("Invalid JSON");
                              return;
                          }
                          QJsonObject res = response.object();

                          if (res.contains("error")) {
                              ui->buttonBox->setEnabled(true);

                              ui->status->setText(res.value("error_description").toString().toUtf8());
                              ui->status->setStyleSheet("* { color: red; }");

                              return;
                          }

                        ui->buttonBox->setEnabled(true);
                        ui->testButton->hide();

                        ui->status->setText(tr("It works!"));
                        ui->status->setStyleSheet("* { color: green; }");
            });
        }
    } else {
        // No Token set
        ui->status->setText(tr("Login with Google needed"));
    }
}

void GDriveSettingsDialog::on_testButton_clicked() {
    GDriveSettingsDialog::checkAuthorization();
}

void GDriveSettingsDialog::on_authorize_clicked() {
    if (ui->pin->text().isEmpty() || ui->folderId->text().isEmpty()) return;
    ui->buttonBox->setEnabled(false);
    QJsonObject object;
    object.insert("client_id", ui->clientId->text());
    object.insert("client_secret", ui->clientSecret->text());
    object.insert("grant_type", "authorization_code");
    object.insert("code", ui->pin->text());
    object.insert("redirect_uri", "urn:ietf:wg:oauth:2.0:oob");
    settings::settings().setValue("google/cid", ui->clientId->text());
    settings::settings().setValue("google/csecret", ui->clientSecret->text());
    settings::settings().setValue("google/folder", ui->folderId->text());
    settings::settings().setValue("google/public", ui->isPublic->isChecked());

    ioutils::postJson(QUrl("https://accounts.google.com/o/oauth2/token"),
                      QList<QPair<QString, QString>>({ QPair<QString, QString>("Content-Type", "applicaton/json") }),
                      QJsonDocument::fromVariant(object.toVariantMap()).toJson(),
                      [&](QJsonDocument response, QByteArray, QNetworkReply *r) {
                        QVariant statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                        if (!statusCode.isValid()) return;

                        int status = statusCode.toInt();
                        if (status != 200 && status != 400)
                        {
                            QString reason = r->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
                            qDebug() << reason;

                            ui->buttonBox->setEnabled(true);
                            return;
                        }


                          if (!response.isObject()) {
                              ui->buttonBox->setEnabled(true);
                              return;
                          }
                          QJsonObject res = response.object();

                          if (res.contains("error")) {
                              ui->buttonBox->setEnabled(true);

                              ui->status->setText(res.value("error_description").toString().toUtf8());
                              ui->status->setStyleSheet("* { color: red; }");

                              return;
                          }

                          if (res.value("success").toBool()) {
                              ui->buttonBox->setEnabled(true);
                              return;
                          }

                          settings::settings().setValue("google/expire",
                                                        QDateTime::currentDateTimeUtc().addSecs(res["expires_in"].toInt()));
                          settings::settings().setValue("google/refresh", res["refresh_token"].toString().toUtf8());
                          settings::settings().setValue("google/access", res["access_token"].toString().toUtf8());
                          ui->status->setText(tr("It works!"));
                          ui->status->setStyleSheet("* { color: green; }");

                          ui->authorize->setEnabled(false);
                          ui->addApp->setEnabled(false);
                          ui->clientSecret->setEnabled(false);
                          ui->clientId->setEnabled(false);

                          ui->buttonBox->setEnabled(true);

                          ui->testButton->hide();
                      });
}
