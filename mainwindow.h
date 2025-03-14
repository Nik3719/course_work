#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCalendarWidget>
#include <QTableWidget>
#include <QTcpSocket>
#include <QSystemTrayIcon>
#include <QPushButton>
#include <QScrollArea>
#include<QFile>
#include<QProcess>
#include "EventWidget.h"
#include"network.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // UI элементы
    QCalendarWidget *calendarWidget;
    QTableWidget *weekTable;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *exportButton;  // Пока только кнопка
    QPushButton *importButton;  // Пока только кнопка
    QPushButton *logoutButton;

    // Сетевые элементы
    QByteArray response;
    QSystemTrayIcon *trayIcon;

    QScrollArea *weekScrollArea;
    QWidget *weekContainer;
    QHBoxLayout *weekLayout;
    QStringList eventsData;
    QLineEdit *searchLineEdit;

    // Функции работы с сервером и событиями
    void connectToServer();
    void loadDates();
    void checkDate();
    void updateWeekTable(const QDate &selectedDate);
    void showAddDateDialog();
    void deleteDate();
    void onServerResponse();
    void handleEditRequest(const QString &id, const QDate &newDate,
                           const QString &newName, const QString &newDescription,
                           bool isImportant, const QString &newColor);
    void CreateAddButton();
    void CreateDelButton();
    void CreateSearchLine();
    // void CreateExportButton();
    // void CreateImportButton();
    void onLogoutClicked();
    void CreateLogoutButton();
    void searchByName();

};

#endif // MAINWINDOW_H
