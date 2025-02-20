#include "server.h"
#include <QDebug>
#include <QCoreApplication>
#include <QTcpServer>
#include <QSqlDatabase>
#include <QSqlError>

Server::Server(QObject *parent) : QTcpServer(parent) {
    // Подключаемся к базе данных
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("mem_date");
    db.setUserName("postgres");
    db.setPassword("nik");


    if (!db.open()) {
        qDebug() << "Ошибка подключения к БД:" << db.lastError().text();
    } else {
        qDebug() << "Подключение к БД успешно!";
    }

    // Выбираем порт, на котором будет слушать сервер (например, 1234)
    quint16 port = 1244;

    // Пытаемся запустить прослушивание порта
    if (!listen(QHostAddress::Any, port)) {
        qDebug() << "Порт" << port << "уже используется другим процессом. Завершаем приложение.";
        exit(1);
        return; // На всякий случай выходим из конструктора
    } else {
        qDebug() << "Сервер слушает порт" << port;
    }
}

Server::~Server() {
    db.close();
}

void Server::startServer() {
    if (!this->listen(QHostAddress::Any, 1244)) {
        qDebug() << "Ошибка запуска сервера:" << this->errorString();
    } else {
        qDebug() << "Сервер запущен на порту 1234";
    }
}

void Server::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *clientSocket = new QTcpSocket();
    clientSocket->setSocketDescriptor(socketDescriptor);

    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);

    clients.append(clientSocket);
    qDebug() << "Новое подключение: " << socketDescriptor;
}



void Server::onReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QByteArray data = clientSocket->readAll();
    qDebug() << "Получено сообщение:" << data;

    QStringList parts = QString::fromUtf8(data).split("|");
    if (parts.isEmpty()) return;

    QString user_id = parts[0];

    QString command = parts[1];
    qDebug()<< user_id << " " << command;

    if (command == "GET_DATES") {
        handleGetDates(clientSocket, parts);
    }
    else if (command == "ADD_DATE" && parts.size() == 6) {
        handleAddDate(clientSocket, parts);
    }
    else if (command == "DELETE_DATE" && parts.size() > 1) {
        handleDeleteDate(clientSocket, parts);
    }
    else if (command == "CHECK_DATE") {
        handleCheckDate(clientSocket);
    }
    else if (command == "EDIT_DATE" && parts.size() == 7) {
        handleEditDate(clientSocket, parts);
    }
    else if (command == "IMPORT_CSV") {
        handleImportCSV(clientSocket, parts);
    }
    else if(command == "LOGIN") {
        handleLogin(clientSocket, parts);
    }
    else if (command == "REG"){
        handleRegister(clientSocket, parts);

    }

}

void Server::handleRegister(QTcpSocket *clientSocket, const QStringList &parts) {
    // Проверяем, что получено достаточно частей: ожидаем минимум 4 части
    if (parts.size() < 4) {
        clientSocket->write("REG_ERROR|Недостаточно данных");
        clientSocket->flush();
        return;
    }

    QString username = parts[2].trimmed();
    QString password = parts[3].trimmed();

    // Проверяем, существует ли уже пользователь с таким именем
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE username = :username");
    query.bindValue(":username", username);
    if (!query.exec()) {
        qDebug() << "Ошибка проверки пользователя:" << query.lastError().text();
        clientSocket->write("REG_ERROR|Ошибка базы данных");
        clientSocket->flush();
        return;
    }
    if (!query.next()) {
        qDebug() << "Ошибка: нет данных в результате запроса";
        return;
    }

    query.next();



    if (query.value(0).toInt() > 0) {
        clientSocket->write("REG_ERROR|Пользователь уже существует");
        clientSocket->flush();
        return;
    }

    // Регистрируем нового пользователя
    query.prepare("INSERT INTO users (username, password) VALUES (?, ?)");
    query.addBindValue(username);
    // Если у вас есть функция хэширования, например:
    // QString hashFunction(const QString &password)
    query.addBindValue(hashFunction(password));

    if (query.exec()) {
        clientSocket->write("OK");
        clientSocket->flush();
    } else {
        qDebug() << "Ошибка регистрации:" << query.lastError().text();
        clientSocket->write("REG_ERROR|Ошибка регистрации");
        clientSocket->flush();
    }
}


QString Server::hashFunction(const QString &input) {
    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}


void Server::handleLogin(QTcpSocket *clientSocket, const QStringList &parts)
{
    if (parts.size() < 3) {
        clientSocket->write("ERROR Недостаточно параметров для авторизации");
        clientSocket->flush();
        return;
    }

    QString login = parts[2];
    QString password = parts[3];

    qDebug() << "login " << login << " " << password;

    // Для безопасности рекомендуется хэшировать пароль
    QString hashedPassword = hashFunction(password);

    QSqlQuery query;
    // Обновляем запрос, чтобы вернуть также идентификатор пользователя
    query.prepare("SELECT id, password FROM users WHERE login = :username");
    query.bindValue(":username", login);

    if (!query.exec()) {
        qDebug() << "Ошибка запроса:" << query.lastError().text();
        clientSocket->write("ERROR Ошибка сервера");
        clientSocket->flush();
        return;
    }

    if (query.next()) {
        QString storedPassword = query.value("password").toString();
        qDebug() << "storedPassword " << storedPassword;
        if (storedPassword == hashedPassword) {
            int userId = query.value("id").toInt();
            // Отправляем клиенту сообщение с идентификатором пользователя, например, "OK 123"
            clientSocket->write(("OK|" + QString::number(userId)).toUtf8());
        } else {
            clientSocket->write("ERROR|Неверный пароль");
        }
    } else {
        clientSocket->write("ERROR|Пользователь не найден");
    }

    clientSocket->flush();
}

void Server::handleImportCSV(QTcpSocket *clientSocket, const QStringList &parts) {
    if (parts.size() < 3) {
        clientSocket->write("IMPORT_ERROR|Недостаточно данных");
        clientSocket->flush();
        return;
    }

    QString user_id = parts[0].trimmed();  // ID пользователя
    QString csvData = parts.mid(2).join("|");  // Соединяем все части после "IMPORT_CSV"

    qDebug() << "User ID:" << user_id;
    qDebug() << "CSV Data:" << csvData;

    QStringList lines = csvData.split("\n", Qt::SkipEmptyParts);
    int insertedCount = 0;
    QSqlQuery query;

    for (const QString &line : lines) {
        QStringList fields = line.split(",", Qt::SkipEmptyParts);

        if (fields.size() < 3) {
            qDebug() << "Ошибка: строка содержит менее 3 полей:" << line;
            continue;
        }

        qDebug() << "Обработка строки:" << fields;

        query.prepare("INSERT INTO dates (user_id, date, name, description) VALUES (?, ?, ?, ?)");
        query.addBindValue(user_id);
        query.addBindValue(fields[0].trimmed());  // Дата
        query.addBindValue(fields[1].trimmed());  // Название
        query.addBindValue(fields[2].trimmed());  // Описание

        if (query.exec()) {
            insertedCount++;
        } else {
            qDebug() << "Ошибка добавления в БД:" << query.lastError().text();
        }
    }

    clientSocket->write(QString("IMPORT_SUCCESS|%1").arg(insertedCount).toUtf8());
    clientSocket->flush();

    qDebug() << "Импорт завершён. Вставлено строк:" << insertedCount;
}







void Server::handleEditDate(QTcpSocket *clientSocket, const QStringList &parts) {
    QString id = parts[2];
    QString newDate = parts[3];
    QString newName = parts[4];
    QString newDescription = parts[5];
    QString isImportant = parts[6];  // Значение "0" или "1"

    QSqlQuery query;
    query.prepare("UPDATE dates SET date = :date, name = :name, description = :description, is_important = :important WHERE id = :id");
    query.bindValue(":date", newDate);
    query.bindValue(":name", newName);
    query.bindValue(":description", newDescription);
    query.bindValue(":important", (isImportant == "1") ? QVariant(true) : QVariant(false));  // Правильное приведение типа
    query.bindValue(":id", id);

    if (query.exec()) {
        clientSocket->write("EDIT_SUCCESS");
    } else {
        clientSocket->write("EDIT_ERROR|" + query.lastError().text().toUtf8());
    }
    qDebug()<< query.lastError().text();
    clientSocket->flush();
    printDatabaseContents();
}



void Server::handleGetDates(QTcpSocket *clientSocket, const QStringList &parts) {
    QString user_id = parts[0];  // Получаем user_id из запроса
    QSqlQuery query;

    // Изменяем запрос, чтобы учитывать user_id
    query.prepare("SELECT id, date, name, description, is_important FROM dates WHERE user_id = :user_id ORDER BY id");
    query.bindValue(":user_id", user_id);  // Привязываем значение user_id из запроса

    if (query.exec()) {
        QByteArray response;

        // Если результат пустой
        if (!query.next()) {
            clientSocket->write("NO_DATA");  // Отправляем, если данных нет
            clientSocket->flush();
            return;
        }

        // Формируем данные в виде строки
        do {
            response += query.value(0).toString().toUtf8() + "|" +
                        query.value(1).toString().toUtf8() + "|" +
                        query.value(2).toString().toUtf8() + "|" +
                        query.value(3).toString().toUtf8() + "|" +
                        query.value(4).toString().toUtf8() + "\n";
        } while (query.next());

        clientSocket->write(response);
    } else {
        clientSocket->write("DB_ERROR");  // Ошибка при запросе
    }

    clientSocket->flush();
}


void Server::handleAddDate(QTcpSocket *clientSocket, const QStringList &parts) {
    QString user_id = parts[0];
    QString date = parts[2];
    QString name = parts[3];
    QString description = parts[4];
    bool isImportant = (bool)parts[5].toInt();

    QSqlQuery query;
    query.prepare("INSERT INTO dates (user_id, date, name, description, is_important) VALUES (:user_id, :date, :name, :description, :is_important)");
     query.bindValue(":user_id", user_id);
    query.bindValue(":date", date);
    query.bindValue(":name", name);
    query.bindValue(":description", description);
    query.bindValue(":is_important", isImportant);


    if (query.exec()) {
        qDebug() << "Database no Error:" ;
        clientSocket->write("OK");
        clientSocket->flush();  // Принудительная отправка данных
        clientSocket->waitForBytesWritten(); // Ждем, пока данные отправятся

    } else {
        qDebug() << "Database Error:" << query.lastError().text();

        clientSocket->write("DB_ERROR");
        clientSocket->flush();  // Принудительная отправка данных
        clientSocket->waitForBytesWritten(); // Ждем, пока данные отправятся
    }
    printDatabaseContents();
}

void Server::handleDeleteDate(QTcpSocket *clientSocket, const QStringList &parts) {
    QSqlQuery query;
    bool success = true;


    for (int i = 2; i < parts.size(); ++i) {
        query.prepare("DELETE FROM dates WHERE id = :id");
        query.bindValue(":id", parts[i]);

        if (!query.exec()) {
            qDebug() << "Ошибка при удалении ID" << parts[i] << ":" << query.lastError().text();
            success = false;
            break;
        }
    }

    if (success) {
        clientSocket->write("OK");
    } else {
        clientSocket->write("DB_ERROR");
    }
}


void Server::handleSearchByName(QTcpSocket *clientSocket, const QString &name) {
    QSqlQuery query;
    query.prepare("SELECT id, date, name, description FROM dates WHERE name LIKE :name");
    query.bindValue(":name", "%" + name + "%");  // Подставляем имя с подстановочными символами %

    if (!query.exec()) {
        qDebug() << "Ошибка выполнения запроса: " << query.lastError().text();
        clientSocket->write("DB_ERROR");
        return;
    }

    QStringList result;
    while (query.next()) {
        QString row = QString("%1,%2,%3,%4")
                          .arg(query.value(0).toString())  // id
                          .arg(query.value(1).toString())  // date
                          .arg(query.value(2).toString())  // name
                          .arg(query.value(3).toString()); // description
        result.append(row);
    }

    QString response = result.join("\n") + "\n";  // Объединяем строки и добавляем перевод строки
    clientSocket->write(response.toUtf8());
    clientSocket->flush();
    qDebug() << "Отправлен ответ: " << response;
}

void Server::handleCheckDate(QTcpSocket *clientSocket) {
    QDate currentDate = QDate::currentDate();
    QSqlQuery query("SELECT date, name FROM dates WHERE is_important = 1");

    QByteArray response;
    while (query.next()) {
        QDate savedDate = QDate::fromString(query.value(0).toString(), "dd.MM.yyyy");
        if (savedDate.month() == currentDate.month() && savedDate.day() == currentDate.day()) {
            response += "IMPORTANT_DATE|" + query.value(1).toString().toUtf8() + "\n";
        }
    }

    if (!response.isEmpty()) {
        clientSocket->write(response);
    } else {
        clientSocket->write("NO_IMPORTANT_DATES");
    }
}



void Server::onClientDisconnected() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        clients.removeAll(clientSocket);
        clientSocket->deleteLater();
        qDebug() << "Клиент отключился";
    }
}

void Server::printDatabaseContents() {
    QSqlQuery query;
    query.prepare("SELECT * FROM dates");

    if (query.exec()) {
        while (query.next()) {
            // Извлекаем все необходимые данные, включая ID
            QString id = query.value("id").toString();  // Получаем ID
            QString date = query.value("date").toString();
            QString name = query.value("name").toString();
            QString description = query.value("description").toString();
            bool isImportant = query.value("is_important").toBool();

            // Выводим информацию
            qDebug() << "ID:" << id
                     << ", Date:" << date
                     << ", Name:" << name
                     << ", Description:" << description
                     << ", Important:" << isImportant;
        }
    } else {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
    }
}

