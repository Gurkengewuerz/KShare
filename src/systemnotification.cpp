#include "systemnotification.h"
#include <QApplication>
#include <QUrl>

#ifndef Q_OS_WIN
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#else
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
SystemNotification::SystemNotification(QObject *parent) : QObject(parent) {
    m_interface = new QDBusInterface(QStringLiteral("org.freedesktop.Notifications"),
                                     QStringLiteral("/org/freedesktop/Notifications"),
                                     QStringLiteral("org.freedesktop.Notifications"),
                                     QDBusConnection::sessionBus(),
                                     this);
}
#else
SystemNotification::SystemNotification(QObject *parent) : QObject(parent) {
    m_interface = nullptr;
}
#endif

void SystemNotification::sendMessage(const QString &text, const QString &savePath) {
    sendMessage(text, tr("KShare Info"), QSystemTrayIcon::Information, savePath);
}

void SystemNotification::sendMessage(
        const QString &text,
        const QString &title,
        const QSystemTrayIcon::MessageIcon icon,
        const QString &savePath,
        const int timeout)
{

#ifndef Q_OS_WIN
    QList<QVariant> args;
    QVariantMap hintsMap;
    if (!savePath.isEmpty()) {
        QUrl fullPath = QUrl::fromLocalFile(savePath);
        // allows the notification to be dragged and dropped
        hintsMap[QStringLiteral("x-kde-urls")] = QStringList({fullPath.toString()});
    }

    QString iconString = "";
    switch (icon) {
        case QSystemTrayIcon::Warning:
            iconString = "dialog-warning";
            break;
        case QSystemTrayIcon::Critical:
            iconString = "dialog-error";
            break;
        default:
            iconString = "dialog-information";
            break;
    }

    args << (qAppName())                 //appname
         << static_cast<unsigned int>(0) //id
         << iconString                   //icon
         << title                        //summary
         << text                         //body
         << QStringList()                //actions
         << hintsMap                     //hints
         << timeout;                     //timeout
    m_interface->callWithArgumentList(QDBus::AutoDetect, QStringLiteral("Notify"), args);
#else
    if (!MainWindow::inst()) return;
    MainWindow::inst()->tray->showMessage(text, title, icon, timeout);
#endif
}
