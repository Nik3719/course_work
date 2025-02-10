#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDateEdit>
#include <QHeaderView>
#include <QTimer>              // Добавляем для QTimer
#include <QSystemTrayIcon>     // Добавляем для QSystemTrayIcon
#include<QCheckBox>
#include <QMenu>
#include <QContextMenuEvent>  // Для использования QContextMenuEvent

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadDates();
    void showAddDateDialog();
    void deleteDate();
    void checkDate();         // Слот для проверки даты (вызывается таймером)

private:
    void initDatabase();
    void createContextMenu(int row);
    void onTableWidgetCustomContextMenuRequested(const QPoint &pos);
    // void searchByName();

    QSqlDatabase db;
    QTableWidget *tableWidget;
    QPushButton *addButton;
    QPushButton *deleteButton;
    // QLineEdit *searchLineEdit;  // Поле для ввода текста поиска
    // QPushButton *searchButton;  // Кнопка для запуска поиска

    QTimer *dateTimer;            // Таймер для периодической проверки даты
    QSystemTrayIcon *trayIcon;     // Иконка в системном трее для уведомлений
};

#endif // MAINWINDOW_H
