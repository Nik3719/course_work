#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QCryptographicHash>



class Server : public QTcpServer {
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    void startServer();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    //void onClientConnected();


private:
    void onClientDisconnected();
    void onReadyRead();
    void handleAddDate(QTcpSocket *clientSocket, const QStringList &parts);
    void handleDeleteDate(QTcpSocket *clientSocket, const QStringList &parts);
    void handleCheckDate(QTcpSocket *clientSocket);
    void handleGetDates(QTcpSocket *clientSocket, const QStringList &parts);
    void handleSearchByName(QTcpSocket *clientSocket, const QString &name);
    void handleEditDate(QTcpSocket *clientSocket, const QStringList &parts);
    void handleImportCSV(QTcpSocket *clientSocket, const QStringList &parts);
    void handleLogin(QTcpSocket *clientSocket, const QStringList &parts);
    void handleRegister(QTcpSocket *clientSocket, const QStringList &parts);
    QString hashFunction(const QString &input);
    void printDatabaseContents();

    QSqlDatabase db;
    QList<QTcpSocket*> clients;
};

#endif // SERVER_H
