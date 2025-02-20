#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QCryptographicHash>
#include"network.h"

// Пример функции хэширования с использованием QCryptographicHash
QString hashFunction(const QString &password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    this->resize(300, 200);
    setWindowTitle("Авторизация");

    QLabel *usernameLabel = new QLabel("Имя пользователя:");
    QLabel *passwordLabel = new QLabel("Пароль:");

    usernameEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);

    loginButton = new QPushButton("Войти");
    registerButton = new QPushButton("Зарегистрироваться");

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(usernameEdit);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(passwordEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

LoginDialog::~LoginDialog() {
    // Если сокет еще открыт, закрываем его
    if (socket->isOpen())
        socket->close();
}

void LoginDialog::onLoginClicked() {
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    // Пример использования сокета для авторизации через сервер
    if(socket->state() != QAbstractSocket::ConnectedState) {
        // socket->connectToHost("127.0.0.1", 1244); // IP сервера и порт
        if(!socket->waitForConnected(3000)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось подключиться к серверу.");
            return;
        }
    }

    // Формируем запрос на авторизацию, например: "LOGIN <username> <password>"
    QByteArray request = QString("empty_user|LOGIN|%1|%2")
                             .arg(username, password)
                             .toUtf8();

    socket->write(request);
    socket->flush();

    // Ожидаем ответ сервера
    socket->waitForReadyRead();
    QByteArray serverResponse = socket->readAll();

    QStringList parts = QString::fromUtf8(serverResponse).split('|');

    if (parts[0] == "OK") {
        qDebug() << "serverResponse " <<serverResponse << " " << parts[0] << " " << parts[1];

        if (parts.size() >= 2) {  // Уб
            user_id = parts[1];

            // Записываем в файл authorization.txt
            QFile file("authorization.txt");
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << usernameEdit->text() << "|"
                    << passwordEdit->text() << "|"
                    << user_id << "\n";
                file.close();
            } else {
                qWarning() << "Не удалось открыть файл authorization.txt для записи";
            }

            // Авторизация успешна: закрываем сокет и диалог
            // socket->disconnectFromHost();
            accept();
            return;
        }
    }

    QMessageBox::warning(this, "Ошибка", "Неверное имя пользователя или пароль");
}







void LoginDialog::onRegisterClicked() {
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя пользователя и пароль для регистрации");
        return;
    }

    // Пример использования сокета для регистрации через сервер
    if(socket->state() != QAbstractSocket::ConnectedState) {
        socket->connectToHost("127.0.0.1", 1244); // IP сервера и порт
        if(!socket->waitForConnected(3000)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось подключиться к серверу.");
            return;
        }
    }

    QByteArray request = QString("empty_user|REG|%1|%2")
                             .arg(username, password)
                             .toUtf8();
    socket->write(request);
    socket->flush();

    if(socket->waitForReadyRead(3000)) {
        QByteArray serverResponse = socket->readAll();
        if(serverResponse.trimmed() == "OK") {
            QMessageBox::information(this, "Успех", "Регистрация успешна. Теперь вы можете войти.");
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Не удалось зарегистрироваться. Возможно, пользователь уже существует.");
}

// Если вы хотите оставить возможность локальной проверки (например, через базу данных),
// можно реализовать эти функции следующим образом (пример ниже не используется в данном сокет-методе):

bool LoginDialog::authenticate(const QString &username, const QString &password) {
    // Здесь можно добавить логику локальной аутентификации через БД,
    // например, используя QSqlQuery. Этот метод можно вызвать, если сервер недоступен.
    return false;
}

bool LoginDialog::registerUser(const QString &username, const QString &password) {
    // Здесь можно добавить логику локальной регистрации через БД.
    return false;
}
