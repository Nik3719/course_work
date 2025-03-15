#include "EventWidget.h"

EventWidget::EventWidget(const QString &id, const QString &eventName, const QString &eventDescription,
                         const QDate &eventDate, const QColor &eventColor, QWidget *parent)
    : QFrame(parent), m_id(id), m_selected(false), m_fullDescription(eventDescription), m_date(eventDate),
    m_eventName(eventName), m_color(eventColor)
{
    // Устанавливаем стиль фона с использованием QColor
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setLineWidth(2);
    setMidLineWidth(1);
    setStyleSheet(QString("background-color: %1; border-radius: 10px; padding: 5px;")
                      .arg(eventColor.name()));  // Преобразуем QColor в строку через метод name()

    QVBoxLayout *layout = new QVBoxLayout(this);

    nameLabel = new QLabel(this);
    QString truncatedName = eventName;
    if (truncatedName.length() > 10)
        truncatedName = truncatedName.left(10) + "...";
    nameLabel->setText(truncatedName);
    nameLabel->setToolTip(eventName);  // Устанавливаем тултип для полного имени
    QFont boldFont = nameLabel->font();
    boldFont.setBold(true);
    nameLabel->setFont(boldFont);

    descLabel = new QLabel(this);
    QString truncatedDesc = eventDescription;
    if (truncatedDesc.length() > 10)
        truncatedDesc = truncatedDesc.left(10) + "...";
    descLabel->setText(truncatedDesc);
    descLabel->setToolTip(eventDescription);  // Устанавливаем тултип для полного описания

    layout->addWidget(nameLabel);
    layout->addWidget(descLabel);
    setLayout(layout);
}


QString EventWidget::id() const {
    return m_id;
}

bool EventWidget::isSelected() const {
    return m_selected;
}




void EventWidget::mousePressEvent(QMouseEvent *event) {
    m_selected = !m_selected;

    // Если выделяем, меняем только рамку и добавляем эффект
    if (m_selected) {
        // Меняем только стиль рамки (без изменения фона)
        setStyleSheet(QString("background-color: %1; border-radius: 10px; padding: 5px; border: 3px solid #000000;")
                          .arg(m_color.name()));  // Оставляем изначальный цвет фона
        // Добавляем тень при выделении
        if (!graphicsEffect()) {
            QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
            shadowEffect->setBlurRadius(15);
            shadowEffect->setOffset(0, 0);
            shadowEffect->setColor(QColor(0, 0, 0, 160));  // Черная тень
            setGraphicsEffect(shadowEffect);
        }
    } else {
        // При снятии выделения возвращаем изначальное оформление
        setStyleSheet(QString("background-color: %1; border-radius: 10px; padding: 5px;")
                          .arg(m_color.name()));  // Оставляем исходный цвет фона
        // Убираем тень
        setGraphicsEffect(nullptr);
    }

    QFrame::mousePressEvent(event);
}







void EventWidget::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    QAction *openAction = menu.addAction("Открыть");
    QAction *editAction = menu.addAction("Редактировать");
    QAction *selectedAction = menu.exec(event->globalPos());

    if (selectedAction == openAction) {
        QDialog dialog(this);
        dialog.setWindowTitle("Просмотр события");
        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        QLabel *nameLbl = new QLabel(m_eventName, &dialog);
        QLabel *descLbl = new QLabel(m_fullDescription, &dialog);
        QLabel *dateLbl = new QLabel(m_date.toString("yyyy-MM-dd"), &dialog);
        descLbl->setWordWrap(true);
        layout->addWidget(new QLabel("Название:", &dialog));
        layout->addWidget(nameLbl);
        layout->addWidget(new QLabel("Описание:", &dialog));
        layout->addWidget(descLbl);
        layout->addWidget(new QLabel("Дата:", &dialog));
        layout->addWidget(dateLbl);
        dialog.setMinimumSize(400, 300);
        dialog.exec();
    }
    else if (selectedAction == editAction) {
        QDialog dialog(this);
        dialog.setWindowTitle("Редактировать событие");
        QFormLayout form(&dialog);

        QDateEdit *dateEdit = new QDateEdit(m_date, &dialog);
        dateEdit->setCalendarPopup(true);
        QLineEdit *nameEdit = new QLineEdit(m_eventName, &dialog);
        QTextEdit *descriptionEdit = new QTextEdit(m_fullDescription, &dialog);
        descriptionEdit->setMinimumHeight(80);
        QCheckBox *importantCheckBox = new QCheckBox("Отметить как важное", &dialog);

        // Добавляем элемент для выбора цвета
        QComboBox *colorComboBox = new QComboBox(&dialog);

        colorComboBox->addItem("Серый", "#808080");  // Red
        colorComboBox->addItem("Оранжевый", "#FF7F00");  // Orange
        colorComboBox->addItem("Желтый", "#FFFF00");  // Yellow
        colorComboBox->addItem("Зеленый", "#00FF00");  // Green
        colorComboBox->addItem("Голубой", "#0000FF");  // Blue
        colorComboBox->addItem("Индиго", "#4B0082");  // Indigo
        colorComboBox->addItem("Фиолетовый", "#8B00FF");  // Violet

        // Если m_color совпадает с одним из вариантов, устанавливаем его
        int index = colorComboBox->findData(m_color);
        if (index != -1)
            colorComboBox->setCurrentIndex(index);

        form.addRow("Дата:", dateEdit);
        form.addRow("Название:", nameEdit);
        form.addRow("Описание:", descriptionEdit);
        form.addRow(importantCheckBox);
        form.addRow("Цвет:", colorComboBox);

        QPushButton *submitButton = new QPushButton("Сохранить", &dialog);
        form.addWidget(submitButton);

        connect(submitButton, &QPushButton::clicked, [&]() {
            QDate newDate = dateEdit->date();
            QString newName = nameEdit->text();
            QString newDescription = descriptionEdit->toPlainText();
            bool isImportant = importantCheckBox->isChecked();
            // Получаем выбранный цвет из QComboBox
            QString newColor = colorComboBox->currentData().toString();

            if (newName.isEmpty() || newDescription.isEmpty()) {
                QMessageBox::warning(&dialog, "Некорректные данные", "Пожалуйста, заполните все поля.");
                return;
            }

            emit editRequested(m_id, newDate, newName, newDescription, isImportant, newColor);

            dialog.accept();
        });
        dialog.setStyleSheet(
            "QDialog { background-color: #f5f5f5; border-radius: 10px; padding: 10px; }"
            "QLineEdit, QTextEdit, QDateEdit { background-color: white; border: 1px solid #ccc; border-radius: 5px; padding: 3px; }"
            "QPushButton { background-color: #3498db; color: white; border-radius: 5px; padding: 5px; }"
            "QPushButton:hover { background-color: #2980b9; }"
            "QPushButton#saveButton { background-color: #4CAF50; }"
            "QPushButton#saveButton:hover { background-color: #45a049; }"
            );
        dialog.exec();
    }
}
