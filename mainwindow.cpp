#include "mainwindow.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSqlError>
#include <QTimer>
#include <QDate>
#include <QSystemTrayIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    this->resize(800, 800);
    this->setWindowTitle("Приложение для запоминания дат");

    // Создаем centralWidget и layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout();  // Убираем родителя

    // Создаем tableWidget без родителя
    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels({"ID", "Дата", "Название", "Описание"});
    tableWidget->setColumnHidden(0, true);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidget->setSortingEnabled(true);  // Включаем сортировку по столбцам

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



    // // Подключаем сигналы и слоты
    connect(addButton, &QPushButton::clicked, this, &MainWindow::showAddDateDialog);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteDate);

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


    QPushButton *exportButton = new QPushButton("Экспорт в CSV");
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

    QPushButton *importButton = new QPushButton("Загрузить из CSV");
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


    QGridLayout *gridLayout = new QGridLayout();
    // Добавляем виджеты в layout

    gridLayout->addWidget(addButton,0 ,0);
    gridLayout->addWidget(deleteButton,0,1);
    gridLayout->addWidget(searchLineEdit,1,0,1,2);
    gridLayout->addWidget(exportButton, 2, 0);
    gridLayout->addWidget(importButton, 2, 1);
    // layout->addWidget(addButton);
    // layout->addWidget(deleteButton);
    // layout->addWidget(searchLineEdit);

    layout->addLayout(gridLayout);

    layout->addWidget(tableWidget);


    // Устанавливаем layout для centralWidget
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // Инициализация базы данных и загрузка данных
    initDatabase();
    loadDates();


    // Настройка системного трей
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon("/home/nik/Desktop/untitled3/Resource/clock-five.png")); // Путь к иконке
    trayIcon->show();

    checkDate();

}


MainWindow::~MainWindow() {
    db.close();
}

void MainWindow::initDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("dates.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка базы данных", db.lastError().text());
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS dates ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "date TEXT, "
               "name TEXT, "
               "description TEXT, "
               "is_important INTEGER DEFAULT 0)"); // 0 - не важно, 1 - важно
}


void MainWindow::loadDates() {
    tableWidget->setRowCount(0);

    // Теперь выбираем id, date, name, description, is_important
    QSqlQuery query("SELECT id, date, name, description, is_important FROM dates");

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Устанавливаем значения в правильные столбцы
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value(0).toString())); // ID (скрытый)
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString())); // Дата
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value(2).toString())); // Название
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value(3).toString())); // Описание

        // Если дата важная, выделяем её цветом (опционально)
        if (query.value(4).toInt() == 1) {
            for (int col = 0; col < 4; ++col) {
                tableWidget->item(row, col)->setBackground(QColor(176, 196, 222));
            }
        }

        row++;
    }
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
        bool isImportant = importantCheckBox->isChecked(); // Получаем значение флажка

        if (!name.isEmpty() && !description.isEmpty()) {
            QSqlQuery query;
            query.prepare("INSERT INTO dates (date, name, description, is_important) VALUES (:date, :name, :description, :is_important)");
            query.bindValue(":date", date);
            query.bindValue(":name", name);
            query.bindValue(":description", description);
            query.bindValue(":is_important", isImportant ? 1 : 0); // Вставляем важность

            if (!query.exec()) {
                QMessageBox::critical(this, "Ошибка добавления", query.lastError().text());
            } else {
                loadDates();
                dialog.accept();
            }
        } else {
            QMessageBox::warning(&dialog, "Некорректные данные", "Пожалуйста, заполните все поля.");
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

    int row = tableWidget->row(selectedItems.first());
    QString id = tableWidget->item(row, 0)->text();  // Получаем ID скрытого столбца

    QSqlQuery query;
    query.prepare("DELETE FROM dates WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка удаления", query.lastError().text());
    } else {
        tableWidget->removeRow(row);
        QMessageBox::information(this, "Удаление даты", "Дата успешно удалена.");
    }
}


void MainWindow::checkDate() {
    QDate currentDate = QDate::currentDate();

    // Проходим по всем датам в базе данных
    QSqlQuery query("SELECT date, name, is_important FROM dates WHERE is_important = 1");
    while (query.next()) {
        QDate savedDate = QDate::fromString(query.value(0).toString(), "dd.MM.yyyy");

        // Если дата совпадает, показываем уведомление
        if (savedDate.month() == currentDate.month() && savedDate.day() == currentDate.day()) {
            QString message = QString("Сегодня важная дата: %1").arg(query.value(1).toString());
            trayIcon->showMessage("Напоминание", message, QSystemTrayIcon::Information, 5000); // Уведомление на 5 секунд
        }
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
        QDateEdit *dateEdit = new QDateEdit(QDate::fromString(date, "dd.MM.yyyy"), &dialog);
        dateEdit->setCalendarPopup(true);
        QLineEdit *nameEdit = new QLineEdit(name, &dialog);
        QLineEdit *descriptionEdit = new QLineEdit(description, &dialog);
        QCheckBox *importantCheckBox = new QCheckBox("Отметить как важную", &dialog); // Флажок для важности

        form.addRow("Дата:", dateEdit);
        form.addRow("Название:", nameEdit);
        form.addRow("Описание:", descriptionEdit);
        form.addRow(importantCheckBox);

        QPushButton *submitButton = new QPushButton("Сохранить", &dialog);
        form.addWidget(submitButton);

        connect(submitButton, &QPushButton::clicked, [&]() {
            QString newDate = dateEdit->date().toString("dd.MM.yyyy");
            QString newName = nameEdit->text();
            QString newDescription = descriptionEdit->text();
            bool isImportant = importantCheckBox->isChecked();

            if (!newName.isEmpty() && !newDescription.isEmpty()) {
                QSqlQuery query;
                query.prepare("UPDATE dates SET date = :date, name = :name, description = :description, is_important = :is_important WHERE id = :id");
                query.bindValue(":date", newDate);
                query.bindValue(":name", newName);
                query.bindValue(":description", newDescription);
                query.bindValue(":is_important", isImportant ? 1 : 0);
                query.bindValue(":id", id);

                if (!query.exec()) {
                    QMessageBox::critical(this, "Ошибка редактирования", query.lastError().text());
                } else {
                    loadDates();
                    dialog.accept();
                }
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
    QString searchText = searchLineEdit->text().trimmed();  // Получаем введенный текст

    if (searchText.isEmpty()) {
        loadDates();  // Если строка поиска пуста, загружаем все данные
        return;
    }

    // Очищаем таблицу перед добавлением отфильтрованных данных
    tableWidget->setRowCount(0);

    // Запрос для поиска по имени
    QSqlQuery query;
    query.prepare("SELECT id, date, name, description FROM dates WHERE name LIKE :name");
    query.bindValue(":name", "%" + searchText + "%");  // Поиск по имени с подстановкой

    if (query.exec()) {
        int row = 0;
        while (query.next()) {
            tableWidget->insertRow(row);
            tableWidget->setItem(row, 0, new QTableWidgetItem(query.value(0).toString())); // ID
            tableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString())); // Дата
            tableWidget->setItem(row, 2, new QTableWidgetItem(query.value(2).toString())); // Название
            tableWidget->setItem(row, 3, new QTableWidgetItem(query.value(3).toString())); // Описание
            row++;
        }
    } else {
        QMessageBox::critical(this, "Ошибка поиска", query.lastError().text());
    }
}





void MainWindow::exportToCSV() {
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
    // Открываем диалог для выбора файла CSV
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть CSV файл", "", "CSV Files (*.csv);;All Files (*)");

    if (fileName.isEmpty()) {
        return; // Если файл не выбран, выходим
    }

    // Открываем файл для чтения
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
        return;
    }

    QTextStream stream(&file);
    QString line;

    // Чтение данных из файла и добавление в БД
    while (!stream.atEnd()) {

        line = stream.readLine();
        QStringList fields = line.split(","); // Разделяем строку на части по запятой (для CSV)
        if (fields[0] == "Дата" && fields[1] == "Название" && fields[2] == "Описание") continue;

        if (fields.size() >= 3) {
            // Добавляем данные в базу данных
            QSqlQuery query;
            query.prepare("INSERT INTO dates (date, name, description) VALUES (?, ?, ?)");
            query.addBindValue(fields[0]);  // Дата
            query.addBindValue(fields[1]);  // Название
            query.addBindValue(fields[2]);  // Описание

            if (!query.exec()) {
                QMessageBox::warning(this, "Ошибка", "Не удалось добавить данные в базу.");
            }
        }

    }

    file.close();
    loadDates();  // Перезагружаем данные в таблице
}


