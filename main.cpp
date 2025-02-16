
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Создаем объект QSharedMemory с уникальным ключом
    QSharedMemory sharedMemory;
    sharedMemory.setKey("UniqueApplicationKey");

    // Если не удалось создать блок памяти, значит экземпляр уже запущен
    if (!sharedMemory.create(1)) {
        QMessageBox::warning(nullptr, "Предупреждение", "Приложение уже запущено.");
        return 0;
    }

    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
