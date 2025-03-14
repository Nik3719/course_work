#include<QApplication>
#include "mainwindow.h"
#include"LoginDialog.h"
#include"network.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    socket = new QTcpSocket();
    socket->connectToHost("127.0.0.1", 1244); // IP сервера и порт
    socket->waitForConnected(3000);

    QFile file("authorization.txt");
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readLine();  // Читаем первую строку

        if (!content.isEmpty()) {
            QStringList parts = content.split('|');
            if (parts.size() == 3) {
                // Извлекаем user_id из файла
                user_id = parts[2];
                qDebug() << user_id;

                // Если файл содержит user_id, то сразу запускаем основное окно
                MainWindow w;
                w.show();
                return app.exec();
            }
        }
    }

    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        // Если авторизация успешна, запускаем основное окно
        MainWindow w;
        w.show();
        return app.exec();
    }
    socket->disconnect();
    return 0;

}
