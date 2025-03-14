#include "mainwindow.h"
#include "EventWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCalendarWidget>
#include <QScrollArea>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QDate>
#include <QLocale>
#include <QDebug>
#include <QCoreApplication>
#include <QDateEdit>
#include <QLabel>
#include <QMap>
#include <algorithm>
#include <QSet>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Устанавливаем русскую локаль (это влияет на форматирование дат)
    QLocale::setDefault(QLocale(QLocale::Russian, QLocale::Russia));

    this->resize(1000, 800);
    this->setWindowTitle("Приложение для запоминания дат");

    // Центральный виджет и основной горизонтальный layout
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Левая панель: календарь и кнопки управления
    QVBoxLayout *leftLayout = new QVBoxLayout();
    calendarWidget = new QCalendarWidget(this);
    calendarWidget->setGridVisible(true);
    leftLayout->addWidget(calendarWidget);

    // Кнопки управления событиями
    addButton = new QPushButton("Добавить событие", this);
    deleteButton = new QPushButton("Удалить событие", this);
    exportButton = new QPushButton("Экспорт", this);   // Пока без функционала
    importButton = new QPushButton("Импорт", this);     // Пока без функционала
    logoutButton = new QPushButton("Выход", this);
    CreateAddButton();
    CreateDelButton();
    CreateSearchLine();
    //CreateExportButton();
    //CreateImportButton();
    CreateLogoutButton();

    leftLayout->addWidget(addButton);
    leftLayout->addWidget(deleteButton);
    leftLayout->insertWidget(0, searchLineEdit);
    leftLayout->addWidget(exportButton);
    leftLayout->addWidget(importButton);
    leftLayout->addWidget(logoutButton);
    leftLayout->addStretch();

    // Правая панель: недельный вид без временных столбцов
    weekContainer = new QWidget(this);
    weekLayout = new QHBoxLayout(weekContainer);
    weekContainer->setLayout(weekLayout);

    weekScrollArea = new QScrollArea(this);
    weekScrollArea->setWidget(weekContainer);
    weekScrollArea->setWidgetResizable(true);

    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(weekScrollArea);
    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 3);

    setCentralWidget(centralWidget);

    // Настройка системного трей
    trayIcon = new QSystemTrayIcon(this);
    QIcon trayIconPNG(":/Resource/clock-five.png");
    trayIcon->setIcon(trayIconPNG);
    trayIcon->show();
    setWindowIcon(trayIconPNG);

    loadDates();
    updateWeekTable(calendarWidget->selectedDate());
    checkDate();

    connect(calendarWidget, &QCalendarWidget::selectionChanged, [=]() {
        updateWeekTable(calendarWidget->selectedDate());
    });

}



void MainWindow::CreateLogoutButton() {
    logoutButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"    /* Синий фон */
        "    color: white;"                  /* Белый текст */
        "    border: 2px solid #2980b9;"      /* Рамка */
        "    border-radius: 5px;"             /* Скругленные углы */
        "    padding: 10px;"                 /* Внутренние отступы */
        "    font-size: 16px;"               /* Размер шрифта */
        "}"
        "QPushButton:hover {"
        "    background-color: #CC0000;"      /* Изменяем цвет при наведении */
        "}"
        );
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
}



// void MainWindow::CreateExportButton() {
//     exportButton = new QPushButton("Экспорт в CSV");
//     connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportToCSV);
//     exportButton->setStyleSheet(
//         "QPushButton {"
//         "    background-color: #e74c3c;"  /* Красный фон */
//         "    color: white;"
//         "    border-radius: 10px;"
//         "    padding: 10px;"
//         "    font-size: 16px;"
//         "}"
//         "QPushButton:hover {"
//         "    background-color: #c0392b;"  /* Темный красный при наведении */
//         "}"
//         );
// }




// void MainWindow::CreateImportButton() {
//     importButton = new QPushButton("Загрузить из CSV");
//     connect(importButton, &QPushButton::clicked, this, &MainWindow::importFromCSV);
//     importButton->setStyleSheet(
//         "QPushButton {"
//         "    background-color: #2ecc71;"  /* Зеленый фон */
//         "    color: white;"
//         "    border-radius: 10px;"
//         "    padding: 10px;"
//         "    font-size: 16px;"
//         "}"
//         "QPushButton:hover {"
//         "    background-color: #27ae60;"  /* Темный зеленый при наведении */
//         "}"
//         );
// }


void MainWindow::CreateSearchLine()
{
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("Введите имя для поиска");
    searchLineEdit->setStyleSheet(
        "QLineEdit {"
        "border: 2px solid #3498db;"
        "border-radius: 10px;"
        "padding: 8px;"
        "font-size: 16px;"
        "color: #2c3e50;"
        "background-color: #ecf0f1;"
        "}"
        "QLineEdit:focus {"
        "border: 2px solid #2980b9;"
        "background-color: #ffffff;"
        "}"
        );

    // Добавляем строку поиска в верх левой панели


    connect(searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::searchByName);
}



void MainWindow::CreateDelButton() {
    deleteButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    deleteButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #e74c3c;"  /* Красный фон */
        "    color: white;"
        "    border-radius: 10px;"
        "    padding: 10px;"
        "    font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #c0392b;"  /* Темный красный при наведении */
        "}"
        );
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteDate);
}



void MainWindow::CreateAddButton() {
    addButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    addButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"  /* Синий фон */
        "    color: white;"
        "    border-radius: 10px;"
        "    padding: 10px;"
        "    font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"  /* Цвет при наведении */
        "}"
        );
    connect(addButton, &QPushButton::clicked, this, &MainWindow::showAddDateDialog);
}



void MainWindow::onLogoutClicked() {
    // Удаляем файл авторизации
    QFile file("authorization.txt");
    if (file.exists()) {
        file.remove();
    }

    // Перезапускаем приложение (завершаем и запускаем заново)
    qApp->quit();
    QProcess::startDetached(qApp->applicationFilePath(), QStringList());
}

MainWindow::~MainWindow()
{
    if (socket) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(3000);
        delete socket;
    }
}

void MainWindow::connectToServer()
{
    socket->connectToHost("127.0.0.1", 1244);
    if (!socket->waitForConnected(5000)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к серверу. Приложение будет закрыто.");
        QCoreApplication::quit();
    }
    // Можно использовать асинхронное чтение, если требуется.
}

void MainWindow::loadDates()
{
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        QString req = user_id + "|GET_DATES";
        socket->write(req.toUtf8());
        socket->flush();

        if (!socket->waitForReadyRead(3000)) {
            qDebug() << "Нет ответа от сервера";
            return;
        }
        QString data = QString::fromUtf8(socket->readAll());
        eventsData = data.split("\n", Qt::SkipEmptyParts);
    }
}

void MainWindow::updateWeekTable(const QDate &selectedDate)
{
    // Очищаем предыдущий вид дней недели
    QLayoutItem *child;
    while ((child = weekLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    // Используем локаль для русских названий
    QLocale locale(QLocale::Russian, QLocale::Russia);

    // Вычисляем понедельник выбранной недели (неделя начинается с понедельника)
    int dayOfWeek = selectedDate.dayOfWeek(); // 1 = понедельник
    QDate monday = selectedDate.addDays(-dayOfWeek + 1); // Исправление вычисления понедельника

    // Для каждого дня недели создаем колонку
    for (int i = 0; i < 7; i++) {
        QDate day = monday.addDays(i);
        QWidget *dayWidget = new QWidget(this);
        QVBoxLayout *dayLayout = new QVBoxLayout(dayWidget);

        // Заголовок с датой на русском (например: пн 13 июл)
        QString dayStr = locale.toString(day, "ddd dd MMM");
        QLabel *dayLabel = new QLabel(dayStr, this);
        dayLabel->setAlignment(Qt::AlignCenter);
        dayLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        dayLayout->addWidget(dayLabel);

        // Добавляем события для данного дня
        for (const QString &line : eventsData) {
            QStringList fields = line.split("|");
            if (fields.size() < 6)
                continue;

            QDate evDate = QDate::fromString(fields[1], "yyyy-MM-dd");
            if (evDate == day) {
                QString eventId = fields[0];
                QString evName = fields[2];
                QString evDesc = fields[3];
                bool isImportant = (fields.size() >= 5 && fields[4] == "1");

                // Цвет события получаем из базы данных (предполагаем, что это строка HEX или обычный цвет)
                QString colorStr = fields[5];
                QColor eventColor;
                if (colorStr.isEmpty()) {
                    // Если цвет не задан, используем дефолтный (например, зеленый)
                    eventColor = QColor(0, 255, 0);
                } else {
                    eventColor = QColor(colorStr);  // QColor может правильно распознавать HEX или rgb
                }

                // Создаем виджет для события с полученным цветом
                // При создании EventWidget передавайте цвет из данных
                EventWidget* eventWidget = new EventWidget(
                    eventId,
                    evName,
                    evDesc,
                    evDate,
                    eventColor,  // <-- здесь передаем цвет
                    this
                    );
                connect(eventWidget, &EventWidget::editRequested,
                        this, &MainWindow::handleEditRequest);
                dayLayout->addWidget(eventWidget);
            }
        }

        dayLayout->addStretch();  // Добавляем растяжение для выравнивания
        weekLayout->addWidget(dayWidget);  // Добавляем день в общий layout
    }
}



void MainWindow::showAddDateDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить событие");

    QFormLayout form(&dialog);
    QDate selectedDate = calendarWidget->selectedDate();
    QDateEdit *dateEdit = new QDateEdit(selectedDate, &dialog);
    dateEdit->setCalendarPopup(true);
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    QTextEdit *descriptionEdit = new QTextEdit(&dialog);
    descriptionEdit->setMinimumHeight(80);  // Многострочное поле
    QCheckBox *importantCheckBox = new QCheckBox("Отметить как важное", &dialog);

    // Добавляем выбор цвета через QComboBox
    QComboBox *colorComboBox = new QComboBox(&dialog);
    colorComboBox->addItem("Красный", "#FF0000");  // Red
    colorComboBox->addItem("Оранжевый", "#FF7F00");  // Orange
    colorComboBox->addItem("Желтый", "#FFFF00");  // Yellow
    colorComboBox->addItem("Зеленый", "#00FF00");  // Green
    colorComboBox->addItem("Голубой", "#0000FF");  // Blue
    colorComboBox->addItem("Индиго", "#4B0082");  // Indigo
    colorComboBox->addItem("Фиолетовый", "#8B00FF");  // Violet


    form.addRow("Дата:", dateEdit);
    form.addRow("Название:", nameEdit);
    form.addRow("Описание:", descriptionEdit);
    form.addRow(importantCheckBox);
    form.addRow("Цвет:", colorComboBox);

    QPushButton *submitButton = new QPushButton("Сохранить", &dialog);
    form.addWidget(submitButton);

    connect(submitButton, &QPushButton::clicked, [&]() {
        QString dateStr = dateEdit->date().toString("yyyy-MM-dd");
        QString name = nameEdit->text();
        QString description = descriptionEdit->toPlainText();
        bool isImportant = importantCheckBox->isChecked();
        // Получаем выбранный цвет
        QString selectedColor = colorComboBox->currentData().toString();

        if (name.isEmpty() || description.isEmpty()) {
            QMessageBox::warning(&dialog, "Некорректные данные", "Пожалуйста, заполните все поля.");
            return;
        }

        // Формируем запрос с дополнительным параметром цвета
        QString req = QString("ADD_DATE|%1|%2|%3|%4|%5")
                          .arg(dateStr)
                          .arg(name)
                          .arg(description)
                          .arg(isImportant ? "1" : "0")
                          .arg(selectedColor);
        req = user_id + "|" + req;

        socket->write(req.toUtf8());
        if (socket && socket->state() == QTcpSocket::ConnectedState) {
            if (!socket->waitForBytesWritten()) {
                QMessageBox::critical(&dialog, "Ошибка", "Не удалось отправить данные на сервер.");
                return;
            }
            if (socket->waitForReadyRead()) {
                if (response.trimmed() == "DB_ERROR") {
                    QMessageBox::critical(&dialog, "Ошибка", "Не добавлено на сервер. Возможно, имя не уникально");
                    response.clear();
                } else {
                    eventsData.clear();
                    loadDates();
                    updateWeekTable(calendarWidget->selectedDate());
                    QMessageBox::information(&dialog, "Успешно", "Добавлено на сервер");
                    response.clear();
                }
                dialog.accept();
            }
        } else {
            QMessageBox::critical(&dialog, "Ошибка", "Нет подключения к серверу.");
        }
    });

    dialog.exec();
}


void MainWindow::deleteDate()
{
    QList<EventWidget*> widgetsToDelete;

    for (int i = 0; i < weekLayout->count(); i++) {
        QLayoutItem *item = weekLayout->itemAt(i);
        QWidget *dayWidget = item ? item->widget() : nullptr;
        if (!dayWidget)
            continue;
        QVBoxLayout *dayLayout = qobject_cast<QVBoxLayout*>(dayWidget->layout());
        if (!dayLayout)
            continue;
        // Пропускаем первый элемент (метка с датой)
        for (int j = 1; j < dayLayout->count(); j++) {
            QLayoutItem *childItem = dayLayout->itemAt(j);
            if (!childItem || !childItem->widget())
                continue;
            EventWidget *eventWidget = qobject_cast<EventWidget*>(childItem->widget());
            if (eventWidget && eventWidget->isSelected()) {
                widgetsToDelete.append(eventWidget);
            }
        }
    }

    if (widgetsToDelete.isEmpty()) {
        QMessageBox::warning(this, "Удаление события", "Выберите событие для удаления.");
        return;
    }

    QString request = "DELETE_DATE";
    for (EventWidget *w : widgetsToDelete) {
        request += "|" + w->id();
    }
    request = user_id + "|" + request;

    socket->write(request.toUtf8());
    socket->flush();

    if (socket->waitForReadyRead(3000)) {
        QByteArray reply = socket->readAll();
        qDebug() << "Ответ от сервера:" << reply;

        for (EventWidget *w : widgetsToDelete) {
            w->hide();
            w->deleteLater();
        }
        QMessageBox::information(this, "Удаление события", "Выбранные события успешно удалены.");

        loadDates();
        updateWeekTable(calendarWidget->selectedDate());
    } else {
        QMessageBox::critical(this, "Ошибка", "Нет ответа от сервера.");
    }
}

void MainWindow::checkDate()
{
    // Здесь можно реализовать уведомления по важным датам.
}

void MainWindow::handleEditRequest(const QString &id, const QDate &newDate,
                                   const QString &newName, const QString &newDescription, bool isImportant, const QString &newColor)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::critical(this, "Ошибка", "Нет подключения к серверу!");
        return;
    }

    QString dateStr = newDate.toString("yyyy-MM-dd");

    // Формируем строку запроса с цветом
    QString request = QString("EDIT_DATE|%1|%2|%3|%4|%5|%6")
                          .arg(id)
                          .arg(dateStr)
                          .arg(newName)
                          .arg(newDescription)
                          .arg(isImportant ? "1" : "0")
                          .arg(newColor); // Добавляем новый цвет

    request = user_id + "|" + request; // Добавляем user_id в начало запроса

    // Отправляем запрос на сервер
    socket->write(request.toUtf8());
    socket->flush();

    if (socket->waitForReadyRead(3000)) {
        QString resp = socket->readAll().trimmed();
        qDebug() << "Ответ от сервера:" << resp;

        if (resp == "EDIT_SUCCESS") {
            loadDates();
            updateWeekTable(calendarWidget->selectedDate());
        }
        else if (resp.startsWith("EDIT_ERROR")) {
            QMessageBox::critical(this, "Ошибка",
                                  "Не добавлено на сервер.\n Возможно имя не уникально");
        }
        else {
            qDebug() << "Неожиданный ответ сервера: " << resp;
        }
    }
    else {
        QMessageBox::critical(this, "Ошибка", "Нет ответа от сервера.");
    }
}

void MainWindow::searchByName()
{
    QString searchText = searchLineEdit->text().trimmed().toLower();

    // Обходим все дни недели
    for (int i = 0; i < weekLayout->count(); ++i) {
        QLayoutItem *dayItem = weekLayout->itemAt(i);
        if (!dayItem || !dayItem->widget()) continue;

        QWidget *dayWidget = dayItem->widget();
        QVBoxLayout *dayLayout = qobject_cast<QVBoxLayout*>(dayWidget->layout());
        if (!dayLayout) continue;

        bool hasVisibleEvents = false;

        // Обходим все события в дне (начиная с индекса 1, пропуская заголовок)
        for (int j = 1; j < dayLayout->count(); ++j) {
            QLayoutItem *eventItem = dayLayout->itemAt(j);
            if (!eventItem || !eventItem->widget()) continue;

            EventWidget *eventWidget = qobject_cast<EventWidget*>(eventItem->widget());
            if (!eventWidget) continue;

            QString eventName = eventWidget->getEventName().toLower();
            bool match = eventName.contains(searchText);

            // Скрываем или показываем событие
            eventWidget->setVisible(match || searchText.isEmpty());
            if (match) hasVisibleEvents = true;
        }

        // Скрываем день, если нет событий и есть поисковый запрос
        dayWidget->setVisible(hasVisibleEvents || searchText.isEmpty());
    }
}


