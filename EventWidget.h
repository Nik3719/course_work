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
#include <QComboBox>
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class EventWidget : public QFrame {
    Q_OBJECT
public:

    explicit EventWidget(const QString &id, const QString &eventName, const QString &eventDescription,
                         const QDate &eventDate, const QColor &eventColor, QWidget *parent = nullptr);
    QString getEventName() const { return m_eventName; }

    QString id() const;
    bool isSelected() const;

signals:
    void editRequested(const QString& id, const QDate& date,
                       const QString& name, const QString& description,
                       bool isImportant, const QString& color);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QString m_id;
    bool m_selected;
    QString m_fullDescription;
    QDate date;
    QString m_eventName;
    QColor m_color;
    QLabel *nameLabel;
    QLabel *descLabel;
};

#endif // EVENTWIDGET_H
