#include "EventWidget.h"

EventWidget::EventWidget(const QString &id, const QString &eventName, const QString &eventDescription,
                         const QDate &eventDate, QWidget *parent)
    : QFrame(parent), m_id(id), m_selected(false), m_fullDescription(eventDescription), m_date(eventDate),
    m_eventName(eventName) {
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setLineWidth(2);
    setMidLineWidth(1);
    setStyleSheet("background-color: #c8e6c9; border-radius: 10px; padding: 5px;");

    QVBoxLayout *layout = new QVBoxLayout(this);

    nameLabel = new QLabel(this);
    QString truncatedName = eventName;
    if (truncatedName.length() > 10)
        truncatedName = truncatedName.left(10) + "...";
    nameLabel->setText(truncatedName);
    nameLabel->setToolTip(eventName);
    QFont boldFont = nameLabel->font();
    boldFont.setBold(true);
    nameLabel->setFont(boldFont);

    descLabel = new QLabel(this);
    QString truncatedDesc = eventDescription;
    if (truncatedDesc.length() > 10)
        truncatedDesc = truncatedDesc.left(10) + "...";
    descLabel->setText(truncatedDesc);
    descLabel->setToolTip(eventDescription);

    layout->addWidget(nameLabel);
    layout->addWidget(descLabel);
}

QString EventWidget::id() const {
    return m_id;
}

bool EventWidget::isSelected() const {
    return m_selected;
}

void EventWidget::mousePressEvent(QMouseEvent *event) {
    m_selected = !m_selected;

    QPalette pal = palette();
    pal.setColor(QPalette::Window, m_selected ? QColor("#5b9bd5") : QColor("#c8e6c9"));
    setPalette(pal);
    setAutoFillBackground(true);

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

        form.addRow("Дата:", dateEdit);
        form.addRow("Название:", nameEdit);
        form.addRow("Описание:", descriptionEdit);
        form.addRow(importantCheckBox);

        QPushButton *submitButton = new QPushButton("Сохранить", &dialog);
        form.addWidget(submitButton);

        connect(submitButton, &QPushButton::clicked, [&]() {
            QDate newDate = dateEdit->date();
            QString newName = nameEdit->text();
            QString newDescription = descriptionEdit->toPlainText();
            bool isImportant = importantCheckBox->isChecked();

            if (newName.isEmpty() || newDescription.isEmpty()) {
                QMessageBox::warning(&dialog, "Некорректные данные", "Пожалуйста, заполните все поля.");
                return;
            }
            emit editRequested(m_id, newDate, newName, newDescription, isImportant);
            dialog.accept();
        });
        dialog.setStyleSheet(
            "QDialog { background-color: #f5f5f5; border-radius: 10px; padding: 10px; }"
            "QLineEdit, QTextEdit, QDateEdit { background-color: white; border: 1px solid #ccc; border-radius: 5px; padding: 3px; }"
            "QPushButton { background-color: #3498db; color: white; border-radius: 5px; padding: 5px; }" // Синий фон для всех кнопок
            "QPushButton:hover { background-color: #2980b9; }"  // Темнее при наведении
            "QPushButton#saveButton { background-color: #4CAF50; }"  // Зелёный только для "Сохранить"
            "QPushButton#saveButton:hover { background-color: #45a049; }" // Темнее при наведении
            );



        dialog.exec();
    }
}
