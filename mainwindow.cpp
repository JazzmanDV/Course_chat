#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QException>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);  
    setupServer();  // Настраиваем сервер

    QDir dir(QDir::currentPath());
    if (!dir.exists("Database")) {  // Если папки не существует, то выходим из конструктора
        return;
    }

    dir.cd("Database"); // Перемещаемся в созданную папку

    QFileInfoList infoList = dir.entryInfoList(QDir::Files);    // Получаем список файлов в папке
    for (auto& fileInfo : infoList) {
        QFile file(fileInfo.filePath());
        if (file.open(QIODevice::ReadOnly)) {
            QDataStream stream(&file);
            QString temp_pseudo_hash;
            stream >> temp_pseudo_hash;
            if (temp_pseudo_hash != PSEUDO_HASH) {  // Если нашли ошибку в заголовке файла, то значит кто-то загрузил свой файл в директорию. Удаляем его и переходим к следующему
                QMessageBox::critical(this, "Ошибка чтения файла", "Невозможно прочитать один из файлов.\nВозможно, вы изменили его содержимое или содержимое Database.");
                file.close();
                file.remove();
                continue;
            }

            Contact* contact = new Contact();
            stream >> contact->serverIP >> contact->serverPort >> contact->socketString;
            int chatHistoryListSize;
            stream >> chatHistoryListSize;
            for (int i = 0; i < chatHistoryListSize; i++) {
                ChatHistory chatHistory;
                stream >> chatHistory.isYourMessage >> chatHistory.message;
                contact->chatHistoryList.push_back(chatHistory);
            }
            contactList.push_back(contact);
            addContactToUI(contact->socketString);
            file.close();
        }
        else {
            qWarning("Could not open file");
        }
    }
}

void MainWindow::setupServer() {
    mainTcpServer = new QTcpServer(this);   // Создаём переменную сервера
    if (!mainTcpServer->listen(QHostAddress::Any, 64000)) {   // Если сервер не может слушать, то выводим ошибку и закрываем приложение
        QMessageBox::critical(this, tr("Ошибка старта сервера"), tr("Невозможно запустить серверную часть: %1.").arg(mainTcpServer->errorString()));
        close();
        exit(1);
    }

    QList<QHostAddress> addressList = QNetworkInterface::allAddresses();    // Получаем список адресов своего ПК
    for (auto& address : addressList) {
         if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost) {   // Если нашли IPv4 локальный адрес или внешний, то сохраняем и выходим из цикла
             mainServerIP = address;
             break;
         }
    }

    if (addressList.isEmpty()) {        // Если список адресов пуст, то назначаем адрес 127.0.0.1
        mainServerIP = QHostAddress(QHostAddress::LocalHost);
    }

    mainServerPort = mainTcpServer->serverPort();
    ui->yourIPLabel->setText("Ваш IP: " + mainServerIP.toString() + ":" + QString::number(mainServerPort));
    connect(mainTcpServer, &QTcpServer::newConnection, this, &MainWindow::on_newConnection);    // Соединяем нашу серверную часть с сигналом о том, что поступило новое соединение
}

MainWindow::~MainWindow() {
    delete mainTcpServer;
    delete ui;

    QDir dir("Database");
    if (dir.exists()) { // Если папка существует, то удаляем её рекурсивно
        dir.removeRecursively();
        while (dir.exists()) {

        }
    }
    QDir dir2(QDir::currentPath()); // Создаём такую же папку
    if (!dir2.mkdir("Database")) {   // Если не удалось создать папку
        QMessageBox::critical(this, "Ошибка создания директории Database", "Невозможно создать директорию Database.\nОсвободите место на диске.");
        close();
        exit(1);
    }

    for (auto& contact : contactList) {
        QFile file("Database/" + contact->serverIP.toString() + " " + QString::number(contact->serverPort) + ".dat");
        if (file.open(QIODevice::WriteOnly)) {
            QDataStream stream(&file);
            stream << PSEUDO_HASH;
            stream << contact->serverIP << contact->serverPort << contact->socketString;
            stream << contact->chatHistoryList.size();
            for (auto& chatHistory : contact->chatHistoryList) {
                stream << chatHistory.isYourMessage << chatHistory.message;
            }
            file.close();
        }
        else {
            QMessageBox::critical(this, "Ошибка открытия файла", "Невозможно открыть один из файлов.\nВозможно, не хватает места на диске.");
        }
    }
}

void MainWindow::on_newConnection() {
    QTcpSocket *newConnection = mainTcpServer->nextPendingConnection();

    Contact* contact = nullptr;
    bool isContactExists = false;
    for (int i = 0; i < contactList.size(); i++) {  // Поиск, существует ли уже запись для такого адреса
        Contact* tempContact = contactList.at(i);
        QString tempPeerAddress = "::ffff:" + tempContact->serverIP.toString();
        if (newConnection->peerAddress().toString() == tempPeerAddress) {  // Если адрес подключения из сокета совпадает с одним из ранее добавленных контактов
            contact = tempContact;
            isContactExists = true;
            break;
        }
    }

    if (!isContactExists) { // Если контакт с таким адресом еще не создан, то создаем
        contact = new Contact();
        contact->serverIP = QHostAddress(newConnection->peerAddress().toString().replace(0, 7, ""));
        contact->serverPort = 64000;
        contact->socketString = contact->serverIP.toString() + ":" + QString::number(contact->serverPort);
        contactList.push_back(contact);

        addContactToUI(contact->socketString);
        ui->connectionList->item(ui->connectionList->count() - 1)->setText(contact->socketString + " (подключено)");
    }

    contact->tcpSocket = newConnection;
    int row = ui->connectionList->count() - 1;
    connect(contact->tcpSocket, &QTcpSocket::readyRead, [this, contact]() {this->readMessage(contact);});
    connect(contact->tcpSocket, &QTcpSocket::disconnected, [this, contact, row]() {this->on_disconnected(contact, row);});
    connect(contact->tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [this, contact]() {this->on_tcpSocketError(contact);});

    if (ui->connectionList->currentRow() == row) {  // Если текущий контакт выделен, то добавляем в окно переписки "Подключено"
        ui->chatArea->appendPlainText("\nПодключено\n");
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Подключено");
    msgBox.setText("Подключение к " + contact->socketString + " выполнено.");
    msgBox.exec();
}

void MainWindow::on_addContactBtn_clicked() {
    CreateConnectionWindow* createConnectionWindow = new CreateConnectionWindow(this);
    QObject::connect(createConnectionWindow, &CreateConnectionWindow::addContact, this, &MainWindow::on_addContact);    // Привязываем вызов функции on_addContact к сигналу addContact
    createConnectionWindow->open();
}

void MainWindow::on_addContact(QString ip, quint16 port) {
    Contact* contact = new Contact();
    contact->serverIP = QHostAddress(ip);
    contact->serverPort = port;
    contact->socketString = contact->serverIP.toString() + ":" + QString::number(contact->serverPort);
    contactList.push_back(contact);

    addContactToUI(contact->socketString);
}

void MainWindow::addContactToUI(QString socketString) {
    ui->connectionList->insertItem(ui->connectionList->count(), socketString);
}

void MainWindow::on_delContactBtn_clicked() {
    int currentIndex = ui->connectionList->currentIndex().row();
    Contact* contact = contactList.at(currentIndex);

    if (currentIndex != -1) {   // Если выбран какой-то из контактов, то освобождаем память и убираем запись из connectioList
        delete contact->tcpSocket;
        delete contact;
        contactList.removeAt(currentIndex);

        delete ui->connectionList->takeItem(currentIndex);
        ui->chatArea->clear();
    }
}

void MainWindow::on_delAllContactsBtn_clicked() {
    for (auto& contact : contactList) {
        delete contact->tcpSocket;
        delete contact;
        contactList.removeOne(contact);
    }

    ui->connectionList->clear();
    ui->chatArea->clear();
}

void MainWindow::on_connectionList_currentItemChanged(QListWidgetItem *currentItem) {
    int currentIndex = ui->connectionList->currentRow();
    if (currentIndex != -1) {   // Если выбран какой-то из контактов
        ui->chatArea->clear();

        Contact* contact = contactList.at(currentIndex);

        QString newItemText = contact->socketString;
        if (contact->tcpSocket != nullptr) {   // Если память выделена по tcpSocket (чтобы не было ошибки)
            if (contact->tcpSocket->state() == QAbstractSocket::ConnectedState) {  // Если мы соединены по выбранному сокету, то подписываем (connected) и убираем звёздочку, иначе просто убираем звёздочку
                newItemText.append(" (подключено)");
            }
        }
        currentItem->setText(newItemText);

        QList<ChatHistory>& chatHistoryList = contact->chatHistoryList;
        int counter = 1;
        for (auto& chatHistory : chatHistoryList) { // Заполняем окно сообщений (chatArea) историей сообщений
            QString currentMessage;
            if (chatHistory.isYourMessage) {
                currentMessage.append("От вас:\n");
            }
            else {
                currentMessage.append("От собеседника:\n");
            }
            currentMessage.append(chatHistory.message);
            // TODO: проверить, нужен ли этот if
            //if (counter == chatHistoryList.count()) {   // Если это последнее сообщение, то добавляем перенос
            //    currentMessage.append("\n");
            //}
            ui->chatArea->appendPlainText(currentMessage);
            counter++;
        }
    }
}

void MainWindow::on_sendMessageBtn_clicked() {
    QString message = ui->inputArea->toPlainText(); // Сохраняем введенное сообщение в переменную
    int currentIndex = ui->connectionList->currentRow();

    if (message != "" && currentIndex != -1) {    // Если отправляемое сообщение не пустое и был выделен какой-либо контакт
        Contact* contact = contactList.at(currentIndex);
        if (contact->tcpSocket == nullptr) {    // Если это первая попытка подключения, то нужно выделить память под сокет
            contact->tcpSocket = new QTcpSocket(this);
        }
        if (contact->tcpSocket->state() != QAbstractSocket::ConnectedState && contact->tcpSocket->state() != QAbstractSocket::UnconnectedState) {   // Если сокет не подключен, но и не отключен, то значит, что мы недавно отключались и память была очищена
            contact->tcpSocket = new QTcpSocket(this);
        }
        if (contact->tcpSocket->state() != QAbstractSocket::ConnectedState) {   // Если сокет не подключен, то подключаемся (без отправки сообщения). Иначе, если сокет подключен, то просто отправляем сообщение
            QHostAddress ip = contact->serverIP;
            quint16 port = contact->serverPort;
            contact->tcpSocket->connectToHost(ip, port);

            // TODO: проверить, влияет ли то, что при ошибке подключения у нас не удаляется tcpSocket и несколько раз идут эти connect`ы
            connect(contact->tcpSocket, &QTcpSocket::readyRead, [this, contact]() {this->readMessage(contact);});
            connect(contact->tcpSocket, &QTcpSocket::disconnected, [this, contact, currentIndex]() {this->on_disconnected(contact, currentIndex);});
            connect(contact->tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [this, contact]() {this->on_tcpSocketError(contact);});

            if (contact->tcpSocket->waitForConnected(3000)) {   // Если мы подключились в течение 3 секунд, то выводим сообщение об успехе, иначе - об ошибке
                ui->connectionList->item(currentIndex)->setText(contact->socketString + " (подключено)");
                ui->chatArea->appendPlainText("\nПодключено\n");

                QMessageBox msgBox;
                msgBox.setWindowTitle("Подключено");
                msgBox.setText("Подключение к " + contact->socketString + " выполнено.");
                msgBox.exec();
            }
            else {
                ui->chatArea->appendPlainText("Ошибка подключения");

                QString errorMessage = "Подключение к " + contact->socketString + " не выполнено.\n" + contact->tcpSocket->errorString();
                QMessageBox::critical(this, "Ошибка подключения", errorMessage);
                //contact->tcpSocket->disconnectFromHost();

                delete contact->tcpSocket;
                contact->tcpSocket = nullptr;
            }
        }
        else if (contact->tcpSocket->state() == QAbstractSocket::ConnectedState) {  // Иначе, если сокет подключен, то просто отправляем сообщение
            sendMessage(contact, message);
            ui->chatArea->appendPlainText("От вас:\n" + message);    // Добавляем сообщение в окно переписки
            ui->inputArea->clear(); // Очищаем окно ввода сообщений
        }
    }
}

void MainWindow::on_tcpSocketError(Contact* contact) {
    // TODO: возможно, стоит выводить сообщение об ошибке здесь, а не в on_sendMessageBtn_clicked
    qDebug() << "1 " << contact->tcpSocket->state();
    qDebug() << "2 " << contact->tcpSocket->errorString();
    //QMessageBox::critical(this, tr("Подключение к серверу"), tr("Невозможно подключиться к серверу.\nКод ошибки: %1").arg(socketError));
}

void MainWindow::sendMessage(Contact* contact, QString message) {
    QByteArray data;
    QDataStream outStream(&data, QIODevice::ReadWrite);

    outStream << quint32(0) << false << message;    // Заносим резерв под размер + флаг запроса истории (false) + сообщение
    outStream.device()->seek(0);    // Переносим указатель на место резерва
    outStream << quint32(data.size() - sizeof(bool) - sizeof(quint32)); // Заносим вместо резерва сам размер сообщения без учета резерва (размера) и флага
    contact->tcpSocket->write(data);    // Отправляем собеседнику сообщение

    ChatHistory chatHistory {true, message};
    contact->chatHistoryList.push_back(chatHistory);
}

void MainWindow::readMessage(Contact* contact) {
    while (contact->tcpSocket->bytesAvailable() < qint64(sizeof(quint32))) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать размер сообщения
        if (!contact->tcpSocket->waitForReadyRead()) {  // Если не готовы для чтения, то выходим
            return;
        }
    }

    QDataStream inStream(contact->tcpSocket);
    quint32 messageSize = 0;
    inStream >> messageSize;    // Читаем размер сообщения

    while (contact->tcpSocket->bytesAvailable() < qint64(sizeof(bool))) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать флаг типа сообщения
        if (!contact->tcpSocket->waitForReadyRead()) {
            return;
        }
    }

    bool isHistoryRequest;
    inStream >> isHistoryRequest;    // Читаем флаг

    if (isHistoryRequest) {

    }
    else {  // Если обычное сообщение, то читаем его
        while (contact->tcpSocket->bytesAvailable() < qint64(messageSize)) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать сообщение целиком
            if (!contact->tcpSocket->waitForReadyRead()) {
                return;
            }
        }

        QString message;
        inStream >> message;    // Читаем сообщение

        int senderIndex = contactList.indexOf(contact);
        if (senderIndex == ui->connectionList->currentRow()) {  // Если нужное окно с чатом открыто, то добавляем сообщение в чат, иначе включаем индикацию нового сообщения
            ui->chatArea->appendPlainText("От собеседника:\n" + message);
        }
        else {
            QString newContactTitle = "* " + contact->socketString + " (connected)";
            ui->connectionList->item(senderIndex)->setText(newContactTitle);
        }

        ChatHistory chatHistory {false, message};
        contact->chatHistoryList.push_back(chatHistory);
    }
}

void MainWindow::on_disconnectBtn_clicked() {
    int currentRow = ui->connectionList->currentRow();
    if (currentRow != -1) { // Если выбран какой-либо контакт
        Contact* contact = contactList.at(currentRow);
        if (contact->tcpSocket->state() == QAbstractSocket::ConnectedState) {    // Если выбранный сокет находится в подключенном состоянии, то отключаем
            contact->tcpSocket->disconnectFromHost();
        }
    }
}

void MainWindow::on_disconnected(Contact* contact, int row) {
    qDebug() << contact->tcpSocket->errorString();
    delete contact->tcpSocket;
    contact->tcpSocket = nullptr;

    int currentRow = ui->connectionList->currentRow();
    if (currentRow != -1) {
        if (currentRow == row) {
            ui->chatArea->appendPlainText("\nОтключено\n");
        }
    }
    ui->connectionList->item(currentRow)->setText(contact->socketString);
}
