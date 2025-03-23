#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include<QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QCryptographicHash>
#include<QTimer>


class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void unlockButtons();
    void updateCountdown();
    void updateCountdownDisplay();

private:
    void CreateLoginButton();
    void CreateRegistreButton();
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
    int failedAttempts = 0;
    QTimer *unlockTimer;
    QLabel *timerLabel;      // Для отображения таймера
    QTimer *countdownTimer;  // Таймер обновления отсчета
    int remainingTime;       // Оставшееся время
};

#endif // LOGINDIALOG_H
