#ifndef GDRIVESETTINGSDIALOG_HPP
#define GDRIVESETTINGSDIALOG_HPP

#include <QDialog>

namespace Ui {
    class GDriveSettingsDialog;
}

class GDriveSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit GDriveSettingsDialog(QWidget *parent = 0);
    ~GDriveSettingsDialog();

private slots:
    void on_addApp_clicked();
    void on_authorize_clicked();
    void on_testButton_clicked();
    bool checkAuthorization();
    void on_isPublic_clicked(bool);
    void on_folderId_textChanged(QString);

private:
    Ui::GDriveSettingsDialog *ui;
};

#endif // GRDIVESETTINGSDIALOG_HPP
