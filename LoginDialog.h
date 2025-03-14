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


class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog(); // Деструктор для корректного закрытия сокета

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void CreateLoginButton();
    void CreateRegistreButton();
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
};

#endif // LOGINDIALOG_H
