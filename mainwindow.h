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
#include <QFileDialog>
#include <QTextStream>
#include <QTcpSocket>
#include<QThread>
#include<QCoreApplication>
#include<QIcon>
#include <QApplication>
#include <QWidget>
#include <QSharedMemory>


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadDates();
    void showAddDateDialog();
    void deleteDate();
    void checkDate();
    void exportToCSV();
    void importFromCSV();


private:
    void initDatabase();
    void createContextMenu(int row);
    void onTableWidgetCustomContextMenuRequested(const QPoint &pos);
    void searchByName();
    void connectToServer();
    void onServerResponse();
    void sendRequest(const QString &request);
    void BDUpdata();
    void CreateTable();
    void CreateAddButton();
    void CreateDelButton();
    void CreateSearchLine();
    void CreateExportButton();
    void CreateImportButton();




    QLineEdit *searchLineEdit;  // Поле для ввода текста поиска
    QSqlDatabase db;
    QTableWidget *tableWidget;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *exportButton;
    QPushButton *importButton;

    QGridLayout *gridLayout;

    QSystemTrayIcon *trayIcon;     // Иконка в системном трее для уведомлений

    QTcpSocket *socket;
    QByteArray response;
};


#endif // MAINWINDOW_H
