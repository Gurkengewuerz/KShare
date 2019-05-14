#ifndef NOTIFICATIONS_HPP
#define NOTIFICATIONS_HPP

#include <QString>
#include <QSystemTrayIcon>

namespace notifications {
    enum class Sound { ERROR = 0, SUCCESS = 1, CAPTURE = 2 };
    void notify(QString title, QString body, QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
    void notifyNolog(QString title, QString body, QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
    void playSound(notifications::Sound soundType);
} // namespace notifications

#endif // NOTIFICATIONS_HPP
