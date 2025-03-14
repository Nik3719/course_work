#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDateEdit>
#include <QDate>
#include <QFont>
#include <QTextEdit>

class EventWidget : public QFrame {
    Q_OBJECT
public:
    explicit EventWidget(const QString &id, const QString &eventName, const QString &eventDescription,
                         const QDate &eventDate, QWidget *parent = nullptr);

    QString id() const;
    bool isSelected() const;

signals:
    void editRequested(const QString &id, const QDate &newDate,
                       const QString &newName, const QString &newDescription, bool isImportant);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QString m_id;
    bool m_selected;
    QString m_fullDescription;
    QDate m_date;
    QString m_eventName;
    QLabel *nameLabel;
    QLabel *descLabel;
};

#endif // EVENTWIDGET_H
