#include "mainwindow.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSqlError>
#include <QTimer>
#include <QDate>
#include <QSystemTrayIcon>


/*
 * onServerResponse используется при любом ответе

*/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    connectToServer();


    this->resize(800, 800);
    this->setWindowTitle("Приложение для запоминания дат");

    // Создаем centralWidget и layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout();  // Убираем родителя

    CreateTable();
    CreateAddButton();
    CreateDelButton();
    CreateSearchLine();

    CreateExportButton();


    CreateImportButton();


    gridLayout = new QGridLayout();
    // Добавляем виджеты в layout

    gridLayout->addWidget(addButton,0 ,0);
    gridLayout->addWidget(deleteButton,0,1);
    gridLayout->addWidget(searchLineEdit,1,0,1,2);
    gridLayout->addWidget(exportButton, 2, 0);
    gridLayout->addWidget(importButton, 2, 1);

    layout->addLayout(gridLayout);

    layout->addWidget(tableWidget);


    // Устанавливаем layout для centralWidget
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // Настройка системного трей
    trayIcon = new QSystemTrayIcon(this);
    QString iconPath = QCoreApplication::applicationDirPath() + "/../Resource/clock-five.png";
    QIcon trayIconPNG(iconPath);
    trayIcon->setIcon(trayIconPNG);
    trayIcon->show();

    QIcon icon("Resource/clock-five.png"); // Убедитесь, что путь к файлу правильный
    setWindowIcon(icon);


    loadDates();
    BDUpdata();
    checkDate();

}

void MainWindow::CreateExportButton()
{
    exportButton = new QPushButton("Экспорт в CSV");
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportToCSV);
    exportButton->setStyleSheet(
        "QPushButton {"
        "background-color: #e74c3c;"  // Красный цвет фона
        "color: white;"                // Белый цвет текста
        "border-radius: 10px;"         // Закругленные углы
        "padding: 10px;"               // Внутренний отступ
        "font-size: 16px;"             // Размер текста
        "}"
        "QPushButton:hover {"
        "background-color: #c0392b;"  // Темный красный при наведении
        "}"
        );

}


void MainWindow::CreateImportButton()
{
    importButton = new QPushButton("Загрузить из CSV");
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importFromCSV);
    importButton->setStyleSheet(
        "QPushButton {"
        "background-color: #2ecc71;"  // Зеленый цвет фона
        "color: white;"                // Белый цвет текста
        "border-radius: 10px;"         // Закругленные углы
        "padding: 10px;"               // Внутренний отступ
        "font-size: 16px;"             // Размер текста
        "}"
        "QPushButton:hover {"
        "background-color: #27ae60;"  // Темный зеленый при наведении
        "}"
        );
}

void MainWindow::CreateSearchLine()
{
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("Введите имя для поиска");
    searchLineEdit->setStyleSheet(
        "QLineEdit {"
        "border: 2px solid #3498db;"      // Синяя рамка
        "border-radius: 10px;"            // Закругленные углы
        "padding: 8px;"                   // Внутренний отступ
        "font-size: 16px;"                // Размер текста
        "color: #2c3e50;"                 // Цвет текста
        "background-color: #ecf0f1;"      // Светло-серый фон
        "}"
        "QLineEdit:focus {"
        "border: 2px solid #2980b9;"      // Цвет рамки при фокусе
        "background-color: #ffffff;"      // Белый фон при фокусе
        "}"
        );

    connect(searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::searchByName);
}

void MainWindow::CreateDelButton()
{
    deleteButton = new QPushButton("Удалить дату");
    deleteButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    deleteButton->setStyleSheet(
        "QPushButton {"
        "background-color: #e74c3c;"  // Красный цвет (Alizarin)
        "color: white;"
        "border-radius: 10px;"        // Закругленные углы
        "padding: 10px;"              // Внутренний отступ
        "font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "background-color: #c0392b;"  // Темно-красный при наведении
        "}"
        );




    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteDate);
}


void MainWindow::CreateAddButton()
{
    // Создаем кнопки без родителя
    addButton = new QPushButton("Добавить дату");
    addButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    addButton->setStyleSheet(
        "QPushButton {"
        "background-color: #3498db;"
        "color: white;"
        "border-radius: 10px;"  // Закругленные углы
        "padding: 10px;"        // Внутренний отступ
        "font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"  // Цвет при наведении
        "}"
        );
    connect(addButton, &QPushButton::clicked, this, &MainWindow::showAddDateDialog);
}

void MainWindow::CreateTable()
{
    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels({"ID", "Дата", "Название", "Описание"});
    tableWidget->setColumnHidden(0, true);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    //tableWidget->setSortingEnabled(true);  // Включаем сортировку по столбцам

    connect(tableWidget, &QTableWidget::customContextMenuRequested,
            this, &MainWindow::onTableWidgetCustomContextMenuRequested);

    tableWidget->setStyleSheet(
        "QTableWidget {"
        "border: 1px solid #7f8c8d;"             /* Более тёмная рамка */
        "gridline-color: #95a5a6;"               /* Более чёткий цвет линий сетки */
        "font-size: 14px;"                       /* Размер текста */
        "color: #2c3e50;"                        /* Тёмный цвет текста */
        "background-color: #ffffff;"             /* Белый фон */
        "alternate-background-color: #eaf2f8;"   /* Светло-голубой фон для чередующихся строк */
        "selection-background-color: #5dade2;"   /* Голубой цвет для выделения строки */
        "selection-color: #ffffff;"              /* Белый цвет текста для выделенных строк */
        "}"
        "QHeaderView::section {"
        "background-color: #2c3e50;"             /* Тёмно-серый фон заголовков */
        "color: #ecf0f1;"                        /* Светлый цвет текста в заголовках */
        "padding: 8px;"                          /* Внутренний отступ в заголовках */
        "font-weight: bold;"                     /* Жирный шрифт для заголовков */
        "border: 1px solid #34495e;"             /* Более тёмная рамка вокруг заголовков */
        "}"
        );

}


MainWindow::~MainWindow() {
    if (socket) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(3000);  // Ждем до 3 секунд для корректного отключения
        delete socket;
    }
}


void MainWindow::initDatabase() {

}



void MainWindow::showAddDateDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить новую дату");

    QFormLayout form(&dialog);
    QDateEdit *dateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    dateEdit->setCalendarPopup(true);
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    QLineEdit *descriptionEdit = new QLineEdit(&dialog);
    QCheckBox *importantCheckBox = new QCheckBox("Отметить как важную", &dialog); // Флажок для важности

    form.addRow("Дата:", dateEdit);
    form.addRow("Название:", nameEdit);
    form.addRow("Описание:", descriptionEdit);
    form.addRow(importantCheckBox);

    QPushButton *submitButton = new QPushButton("Сохранить", &dialog);
    form.addWidget(submitButton);

    connect(submitButton, &QPushButton::clicked, [&]() {
        QString date = dateEdit->date().toString("dd.MM.yyyy");
        QString name = nameEdit->text();
        QString description = descriptionEdit->text();
        bool isImportant = importantCheckBox->isChecked();

        if (name.isEmpty() || description.isEmpty()) {
            QMessageBox::warning(&dialog, "Некорректные данные", "Пожалуйста, заполните все поля.");
            return;
        }

        // Формируем строку запроса
        QString request = QString("ADD_DATE|%1|%2|%3|%4")
                              .arg(date)
                              .arg(name)
                              .arg(description)
                              .arg(isImportant ? "1" : "0");

        socket->write(request.toUtf8());
        // Проверяем, если сокет подключен
        if (socket && socket->state() == QTcpSocket::ConnectedState) {
            if (!socket->waitForBytesWritten()) {
                QMessageBox::critical(&dialog, "Ошибка", "Не удалось отправить данные на сервер.");
                return;
            }
            if (socket->waitForReadyRead())
            {
                if (response == "OK")
                {
                    response.clear();
                    loadDates();
                    BDUpdata();
                    dialog.accept();
                } else {
                    qDebug() << "showAddDateDialog Errror";
                }

            }

        } else {
            QMessageBox::critical(&dialog, "Ошибка", "Не подключено к серверу.");
        }
    });
    dialog.exec();
}




void MainWindow::deleteDate() {
    QList<QTableWidgetItem*> selectedItems = tableWidget->selectedItems();

    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Удаление даты", "Выберите дату для удаления.");
        return;
    }

    QSet<QString> idsToDelete;
    QMap<QString, int> idToRowMap;  // Карта ID -> строка

    // Заполняем ID и соответствующие строки
    for (QTableWidgetItem *item : selectedItems) {
        int row = item->row();
        QString id = tableWidget->item(row, 0)->text();
        idsToDelete.insert(id);
        idToRowMap[id] = row;  // Связываем ID с номером строки
    }

    // Формируем запрос для сервера
    QString request = "DELETE_DATE";
    for (const QString &id : idsToDelete) {
        request += "|" + id;
    }

    socket->write(request.toUtf8());
    if (socket->waitForReadyRead()) {
        qDebug() << "Ответ от сервера:" << response;
        response.clear();

        // Удаляем строки, начиная с самых больших индексов (чтобы избежать смещения)
        QList<int> rows;
        for (const QString &id : idsToDelete) {
            rows.append(idToRowMap[id]);
        }
        std::sort(rows.rbegin(), rows.rend());  // Упорядочиваем от большего к меньшему

        for (int row : rows) {
            tableWidget->removeRow(row);
        }

        QMessageBox::information(this, "Удаление даты", "Выбранные даты успешно удалены.");

    } else {
        QMessageBox::critical(this, "Ошибка", "Нет ответа от сервера.");
    }
}





void MainWindow::checkDate() {

    socket->write("GET_DATES");
    socket->waitForReadyRead();
    QStringList rows = QString(response).split("\n", Qt::SkipEmptyParts);
    response.clear();
    int row = 0;
    QDate currentDate = QDate::currentDate();
    for (const QString &line : rows) {
        QStringList columns = line.split(",");
        QDate savedDate = QDate::fromString(columns[1].trimmed(), "yyyy-MM-dd");
        // Если дата совпадает, показываем уведомление
        if (savedDate.month() == currentDate.month() && savedDate.day() == currentDate.day()) {

            QString message = QString("Сегодня важная дата: %1").arg(columns[2].trimmed());
            trayIcon->showMessage("Напоминание", message, QSystemTrayIcon::Information, 1000); // Уведомление на 5 секунд

            QString command = QString("notify-send 'Напоминание' '%1'").arg(message);
            system(command.toUtf8().constData());
        }
        row++;
    }

}



void MainWindow::createContextMenu(int row) {
    QMenu contextMenu(this);

    QAction *editAction = contextMenu.addAction("Редактировать");

    connect(editAction, &QAction::triggered, [this, row]() {
        // Получаем данные для редактирования
        QString id = tableWidget->item(row, 0)->text();
        QString date = tableWidget->item(row, 1)->text();
        QString name = tableWidget->item(row, 2)->text();
        QString description = tableWidget->item(row, 3)->text();

        // Открываем диалог редактирования
        QDialog dialog(this);
        dialog.setWindowTitle("Редактировать дату");

        QFormLayout form(&dialog);
        QDateEdit *dateEdit = new QDateEdit(QDate::fromString(date, "yyyy-MM-dd"), &dialog);
        dateEdit->setCalendarPopup(true);
        QLineEdit *nameEdit = new QLineEdit(name, &dialog);
        QLineEdit *descriptionEdit = new QLineEdit(description, &dialog);
        QCheckBox *importantCheckBox = new QCheckBox("Отметить как важную", &dialog);

        form.addRow("Дата:", dateEdit);
        form.addRow("Название:", nameEdit);
        form.addRow("Описание:", descriptionEdit);
        form.addRow(importantCheckBox);

        QPushButton *submitButton = new QPushButton("Сохранить", &dialog);
        form.addWidget(submitButton);

        connect(submitButton, &QPushButton::clicked, [&]() {
            QString newDate = dateEdit->date().toString("yyyy-MM-dd");
            QString newName = nameEdit->text();
            QString newDescription = descriptionEdit->text();
            bool isImportant = importantCheckBox->isChecked();

            if (!newName.isEmpty() && !newDescription.isEmpty()) {
                if (socket && socket->state() == QAbstractSocket::ConnectedState) {
                    QString request = QString("EDIT_DATE|%1|%2|%3|%4|%5")
                                          .arg(id)
                                          .arg(newDate)
                                          .arg(newName)
                                          .arg(newDescription)
                                          .arg(isImportant ? "1" : "0");

                    socket->write(request.toUtf8());
                    socket->flush();  // Гарантируем отправку данных
                    qDebug() << "Отправлен запрос на редактирование: " << request;
                    if (socket->waitForReadyRead())
                    {
                        if (response=="EDIT_SUCCESS")
                        {
                            response.clear();
                            loadDates();
                            BDUpdata();
                        }
                    }
                } else {
                    QMessageBox::critical(this, "Ошибка", "Нет подключения к серверу!");
                }
                dialog.accept();
            } else {
                QMessageBox::warning(&dialog, "Некорректные данные", "Пожалуйста, заполните все поля.");
            }
        });

        dialog.exec();
    });

    contextMenu.exec(QCursor::pos());
}


void MainWindow::onTableWidgetCustomContextMenuRequested(const QPoint &pos) {
    QModelIndex index = tableWidget->indexAt(pos);
    if (index.isValid()) {
        createContextMenu(index.row());
    }
}


void MainWindow::searchByName() {
    QString searchText = searchLineEdit->text().trimmed();

    if (searchText.isEmpty()) {
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
            tableWidget->setRowHidden(row, false);  // Показываем все строки
        }
        return;
    }

    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QString name = tableWidget->item(row, 2)->text();  // Колонка с именем
        bool match = name.contains(searchText, Qt::CaseInsensitive);
        tableWidget->setRowHidden(row, !match);  // Скрываем строки, которые не совпадают
    }
}





void MainWindow::exportToCSV() {
    loadDates();
    BDUpdata();
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как CSV", "", "CSV Files (*.csv)");

    if (fileName.isEmpty())
        return;  // Если файл не выбран, выходим

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи.");
        return;
    }

    QTextStream stream(&file);

    // Запись заголовков
    QStringList headers;
    for (int col = 0; col < tableWidget->columnCount(); ++col) {
        if (!tableWidget->isColumnHidden(col)) {  // Пропускаем скрытые столбцы
            headers << tableWidget->horizontalHeaderItem(col)->text();
        }
    }
    stream << headers.join(",") << "\n";

    // Запись данных строк
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < tableWidget->columnCount(); ++col) {
            if (!tableWidget->isColumnHidden(col)) {  // Пропускаем скрытые столбцы
                QTableWidgetItem *item = tableWidget->item(row, col);
                rowData << (item ? item->text() : "");  // Если ячейка пуста, записываем пустую строку
            }
        }
        stream << rowData.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Экспорт завершен", "Таблица успешно экспортирована в CSV.");
}



void MainWindow::importFromCSV() {
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть CSV файл", "", "CSV Files (*.csv);;All Files (*)");

    if (fileName.isEmpty()) {
        return; // Если файл не выбран, выходим
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
        return;
    }

    QTextStream stream(&file);
    QStringList csvData;

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList fields = line.split(",");
        if (fields.size() < 3) continue;  // Пропускаем некорректные строки
        if (fields[0] == "Дата" && fields[1] == "Название" && fields[2] == "Описание") continue;  // Пропускаем заголовок
        csvData.append(line);
    }

    file.close();

    if (csvData.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Файл не содержит данных для импорта.");
        return;
    }

    // Отправляем данные на сервер
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        QString request = "IMPORT_CSV|" + csvData.join("\n");
        socket->write(request.toUtf8());
        socket->flush();

        socket->waitForReadyRead();

        if (response.startsWith("IMPORT_SUCCESS")) {
            QMessageBox::information(this, "Импорт завершен", "Таблица успешно импортирована в базу данных.");

            response.clear();
            loadDates();  // Запрашиваем свежие данные
            BDUpdata();

        } else if (response.startsWith("IMPORT_ERROR")) {
            QMessageBox::warning(this, "Ошибка импорта", "Не удалост импортировать в таблицу");
        }
        response.clear();
        qDebug() << "Отправлен запрос на импорт CSV";
    } else {
        QMessageBox::warning(this, "Ошибка", "Нет подключения к серверу.");
    }

}



void MainWindow::connectToServer() {
    socket = new QTcpSocket(this);

    qDebug() << "Попытка подключения к серверу...";
    socket->connectToHost("127.0.0.1", 1244); // IP сервера и порт

    // Ожидание подключения (тайм-аут 5 секунд)
    if (!socket->waitForConnected(5000)) {
        qDebug() << "Ошибка: Не удалось подключиться к серверу!";
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к серверу. Приложение будет закрыто.");
        QCoreApplication::quit(); // Закрытие приложения, если соединение не установлено
        exit(1);
    }

    qDebug() << "Успешно подключено к серверу!";

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onServerResponse);
    connect(socket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError error) {
        qDebug() << "Ошибка подключения:" << error;
    });
}




void MainWindow::loadDates() {
    // Если сокет не создан или не подключен — ждем подключения
    socket->waitForConnected();
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        // Подключаем слот к сигналу connected (с флагом UniqueConnection, чтобы не было дублирования)
        qDebug() << "Сокет еще не подключен, ждем подключения...";
        return;
    }


    socket->write("GET_DATES");
    qDebug() << "Функция loadDates выполнена";
}


// Теперь обработка ответа сервера происходит в onServerResponse
void MainWindow::onServerResponse() {

    response = socket->readAll();
    qDebug() << "this func onServerResponse " << response;

    if (response == "DB_ERROR") {
        qDebug() << "Ошибка базы данных на сервере!";
        return;
    }
}


void MainWindow::BDUpdata()
{
    if (!socket->waitForReadyRead(3000)) {
        qDebug() << "Нет ответа от сервера в течение 3 секунд.";
        return;
    }

    QStringList rows = QString(response).split("\n", Qt::SkipEmptyParts);
    response.clear();

    tableWidget->setRowCount(0);  // Очищаем таблицу перед загрузкой новых данных

    QVector<bool> importantDates(rows.size(), false);  // Массив для отслеживания важных дат
    int row = 0;

    for (const QString &line : rows) {
        QStringList columns = line.split(",");
        if (columns.size() < 5) continue;  // Пропускаем некорректные строки

        // Отмечаем важные даты
        bool isImportant = (columns[4] == "true");
        importantDates[row] = isImportant;

        tableWidget->insertRow(row);
        tableWidget->setItem(row, 0, new QTableWidgetItem(columns[0])); // ID
        tableWidget->setItem(row, 1, new QTableWidgetItem(columns[1])); // Дата
        tableWidget->setItem(row, 2, new QTableWidgetItem(columns[2])); // Название
        tableWidget->setItem(row, 3, new QTableWidgetItem(columns[3])); // Описание

        row++;
    }

    // Применяем форматирование для важности дат
    for (int i = 0; i < importantDates.size(); ++i) {
        if (importantDates[i]) {
            for (int col = 0; col < 4; ++col) {
                tableWidget->item(i, col)->setBackground(QColor(176, 196, 222));
            }
        }
    }

    qDebug() << "this func BDUpdata";
}


