#include "server.h"
#include <QDebug>
#include<iostream>

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

    QString command = parts[0];

    if (command == "GET_DATES") {
        handleGetDates(clientSocket);
    }
    else if (command == "ADD_DATE" && parts.size() == 5) {
        handleAddDate(clientSocket, parts);
    }
    else if (command == "DELETE_DATE" && parts.size() > 1) {
        handleDeleteDate(clientSocket, parts);
    }
    else if (command == "CHECK_DATE") {
        handleCheckDate(clientSocket);
    }
    else if (command == "EDIT_DATE" && parts.size() == 6) {
        handleEditDate(clientSocket, parts);
    }
    else if (command == "IMPORT_CSV") {
        handleImportCSV(clientSocket, parts);
    }

}

void Server::handleImportCSV(QTcpSocket *clientSocket, const QStringList &parts) {
    // Проверяем, что получено как минимум две части: команда и CSV-содержимое
    if (parts.size() < 2) {
        clientSocket->write("IMPORT_ERROR|Недостаточно данных");
        clientSocket->flush();
        return;
    }

    // Получаем CSV-содержимое (предполагается, что оно передаётся во второй части)
    QString csvData = parts[1];
    // Если CSV-файл содержит несколько строк, они разделяются символом новой строки
    QStringList lines = csvData.split("\n", Qt::SkipEmptyParts);

    int insertedCount = 0;
    QSqlQuery query;

    // Обрабатываем каждую строку CSV
    for (const QString &line : lines) {
        // Разбиваем строку на поля по запятой
        QStringList fields = line.split(",", Qt::SkipEmptyParts);
        // Если строка не содержит минимум 3 поля, пропускаем её
        if (fields.size() < 3) {
            continue;
        }

        qDebug() << "Обработка строки:" << fields;

        query.prepare("INSERT INTO dates (date, name, description) VALUES (?, ?, ?)");
        // Обратите внимание на обрезку пробелов, если необходимо
        query.addBindValue(fields[0].trimmed());
        query.addBindValue(fields[1].trimmed());
        query.addBindValue(fields[2].trimmed());

        if (query.exec()) {
            insertedCount++;
        } else {
            qDebug() << "Ошибка добавления в БД:" << query.lastError().text();
            // Можно решить: прервать импорт или продолжить обработку остальных строк
        }
    }

    // Отправляем клиенту сообщение об успехе импорта и количестве вставленных строк
    QString response = QString("IMPORT_SUCCESS|%1").arg(insertedCount);
    clientSocket->write(response.toUtf8());
    clientSocket->flush();

    qDebug() << "Импорт CSV завершён. Вставлено строк:" << insertedCount;
    printDatabaseContents();
}





void Server::handleEditDate(QTcpSocket *clientSocket, const QStringList &parts) {
    QString id = parts[1];
    QString newDate = parts[2];
    QString newName = parts[3];
    QString newDescription = parts[4];
    QString isImportant = parts[5];  // Значение "0" или "1"

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
    clientSocket->flush();
    printDatabaseContents();
}



void Server::handleGetDates(QTcpSocket *clientSocket) {
    QSqlQuery query;
    query.prepare("SELECT id, date, name, description, is_important FROM dates ORDER BY id");
    if (query.exec()) {
        QByteArray response;
        while (query.next()) {
            response += query.value(0).toString().toUtf8() + "," +
                        query.value(1).toString().toUtf8() + "," +
                        query.value(2).toString().toUtf8() + "," +
                        query.value(3).toString().toUtf8() + "," +
                        query.value(4).toString().toUtf8() + "\n";
        }
        clientSocket->write(response);
    } else {
        clientSocket->write("DB_ERROR");
    }
}

void Server::handleAddDate(QTcpSocket *clientSocket, const QStringList &parts) {
    QString date = parts[1];
    QString name = parts[2];
    QString description = parts[3];
    bool isImportant = (bool)parts[4].toInt();

    QSqlQuery query;
    query.prepare("INSERT INTO dates (date, name, description, is_important) VALUES (:date, :name, :description, :is_important)");
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

    for (int i = 1; i < parts.size(); ++i) {
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



// === ОБРАБОТКА ПРОВЕРКИ ДАТ ===
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

