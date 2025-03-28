#include "LoginDialog.h"
#include"network.h"


// QString hashFunction(const QString &password) {
//     QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
//     return hash.toHex();
// }

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    this->resize(300, 200);
    setWindowTitle("Авторизация");

    QLabel *usernameLabel = new QLabel("Имя пользователя:");
    QLabel *passwordLabel = new QLabel("Пароль:");

    unlockTimer = new QTimer(this);
    connect(unlockTimer, &QTimer::timeout, this, &LoginDialog::unlockButtons);

    usernameEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);

    CreateLoginButton();
    CreateRegistreButton();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(usernameEdit);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(passwordEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);

    mainLayout->addLayout(buttonLayout);

    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &LoginDialog::updateCountdown);

    // Создаем лейбл для отображения времени
    timerLabel = new QLabel(this);
    timerLabel->setAlignment(Qt::AlignCenter);
    timerLabel->setStyleSheet("color: red;");
    timerLabel->hide();  // Скрываем по умолчанию

    // Добавляем в макет
    mainLayout->addWidget(timerLabel);

    setLayout(mainLayout);
}

void LoginDialog::CreateRegistreButton(){
    registerButton = new QPushButton("Зарегистрироваться");

    registerButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #2196F3;"      // синий фон
        "   border: none;"
        "   color: white;"
        "   padding: 12px 24px;"
        "   font-size: 16px;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1976D2;"      // изменяем фон при наведении
        "}"
        );


    connect(registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
}



void LoginDialog::CreateLoginButton(){
    loginButton = new QPushButton("Войти");

    loginButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"      // зелёный фон
        "   border: none;"                   // без рамки
        "   color: white;"                   // белый текст
        "   padding: 12px 24px;"             // отступы
        "   font-size: 16px;"                // размер шрифта
        "   border-radius: 8px;"             // скруглённые углы
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"      // изменяем фон при наведении
        "}"
        );

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
}

LoginDialog::~LoginDialog() {
    if (socket->isOpen())
        socket->close();
}

void LoginDialog::onLoginClicked() {

    if (!loginButton->isEnabled()) {
        QMessageBox::warning(this, "Блокировка",
                             QString("Повторите через %1 сек.").arg(remainingTime));
        return;
    }

    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if(socket->state() != QAbstractSocket::ConnectedState) {
        if(!socket->waitForConnected(3000)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось подключиться к серверу.");
            return;
        }
    }

    QByteArray request = QString("empty_user|LOGIN|%1|%2")
                             .arg(username, password)
                             .toUtf8();

    socket->write(request);
    socket->flush();
    socket->waitForReadyRead();
    QByteArray serverResponse = socket->readAll();

    QStringList parts = QString::fromUtf8(serverResponse).split('|');

    if (parts[0] == "OK") {
        failedAttempts = 0;
        qDebug() << "serverResponse " <<serverResponse << " " << parts[0] << " " << parts[1];

        if (parts.size() >= 2) {
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

            accept();
            return;
        }
    }
    else
    {
        failedAttempts++; // Увеличиваем счетчик ошибок
        if (failedAttempts >= 3) {

        loginButton->setEnabled(false);
        registerButton->setEnabled(false);

        // Настраиваем таймеры
        remainingTime = 30;
        timerLabel->show();
        updateCountdownDisplay();

        unlockTimer->start(30000);    // Основной таймер блокировки
        countdownTimer->start(1000); // Таймер обновления отображения
        QMessageBox::warning(this, "Блокировка",
                             "Слишком много попыток. Повторите через 30 сек.");
        } else
        {
            QMessageBox::warning(this, "Ошибка", "Неверное имя пользователя или пароль");
        }


    }


    //QMessageBox::warning(this, "Ошибка", "Неверное имя пользователя или пароль");
}


void LoginDialog::onRegisterClicked() {
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    // Проверка на пустое имя пользователя или пароль
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя пользователя и пароль для регистрации");
        return;
    }

    // Проверка длины имени пользователя и пароля
    if (username.length() < 8) {
        QMessageBox::warning(this, "Ошибка", "Имя пользователя должно быть не менее 8 символов.");
        return;
    }
    if (password.length() < 8) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 8 символов.");
        return;
    }

    // Проверка подключения к серверу
    if(socket->state() != QAbstractSocket::ConnectedState) {
        socket->connectToHost("127.0.0.1", 1244); // IP сервера и порт
        if(!socket->waitForConnected(3000)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось подключиться к серверу.");
            return;
        }
    }

    // Формирование запроса для отправки на сервер
    QByteArray request = QString("empty_user|REG|%1|%2")
                             .arg(username, password)
                             .toUtf8();
    socket->write(request);
    socket->flush();

    // Ожидание ответа от сервера
    if(socket->waitForReadyRead(3000)) {
        QByteArray serverResponse = socket->readAll();
        if(serverResponse.trimmed() == "OK") {
            QMessageBox::information(this, "Успех", "Регистрация успешна. Теперь вы можете войти.");
            return;
        }
    }

    // Обработка ошибки регистрации
    QMessageBox::warning(this, "Ошибка", "Не удалось зарегистрироваться. Возможно, пользователь уже существует.");
}

void LoginDialog::unlockButtons() {
    loginButton->setEnabled(true);
    registerButton->setEnabled(true);
    failedAttempts = 0; // Сбрасываем счетчик
    unlockTimer->stop();
    timerLabel->hide();
    countdownTimer->stop();
}

// Новый слот для обновления отсчета
void LoginDialog::updateCountdown() {
    remainingTime--;
    updateCountdownDisplay();

    if(remainingTime <= 0) {
        countdownTimer->stop();
        unlockButtons();
    }
}

// Обновление отображения времени
void LoginDialog::updateCountdownDisplay() {
    timerLabel->setText(QString("До разблокировки: %1 сек.").arg(remainingTime));
}

