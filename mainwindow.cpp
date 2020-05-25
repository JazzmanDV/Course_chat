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
    }

    contact->tcpSocket = newConnection;
    int row = contactList.indexOf(contact);

    connect(contact->tcpSocket, &QTcpSocket::readyRead, [this, contact]() {this->on_readyRead(contact);});
    //connect(contact->tcpSocket, &QTcpSocket::disconnected, [this, contact, row]() {this->on_disconnected(contact, row);});
    //connect(contact->tcpSocket, &QTcpSocket::disconnected, contact->tcpSocket, &QTcpSocket::deleteLater);
    connect(contact->tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [this, contact, row]() {this->on_tcpSocketError(contact, row);});

    if (ui->connectionList->currentRow() == row) {  // Если текущий контакт выделен, то добавляем в окно переписки "Подключено"
        ui->chatArea->appendPlainText("Подключено\n");
    }
    ui->connectionList->item(row)->setText(contact->socketString + " (подключено)");

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
        contact->tcpSocket->disconnectFromHost();
        contact->tcpSocket->deleteLater();
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
        Contact* contact = contactList.at(currentIndex);

        QString newItemText = contact->socketString;
        if (contact->tcpSocket != nullptr) {   // Если память выделена по tcpSocket (чтобы не было ошибки)
            if (contact->tcpSocket->state() == QAbstractSocket::ConnectedState) {  // Если мы соединены по выбранному сокету, то подписываем (подключено) и убираем звёздочку, иначе просто убираем звёздочку
                newItemText.append(" (подключено)");
            }
        }
        currentItem->setText(newItemText);

        updateChatArea(contact->chatHistoryList);
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
            if (contact->tcpSocket->waitForConnected(3000)) {   // Если мы подключились в течение 3 секунд, то выводим сообщение об успехе, иначе - об ошибке
                ui->connectionList->item(currentIndex)->setText(contact->socketString + " (подключено)");
                connect(contact->tcpSocket, &QTcpSocket::readyRead, [this, contact]() {this->on_readyRead(contact);});
                //connect(contact->tcpSocket, &QTcpSocket::disconnected, [this, contact, currentIndex]() {this->on_disconnected(contact, currentIndex);});
                //connect(contact->tcpSocket, &QTcpSocket::disconnected, contact->tcpSocket, &QTcpSocket::deleteLater);
                connect(contact->tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [this, contact, currentIndex]() {this->on_tcpSocketError(contact, currentIndex);});

                ui->chatArea->appendPlainText("Подключено\n");

                QMessageBox msgBox;
                msgBox.setWindowTitle("Подключено");
                msgBox.setText("Подключение к " + contact->socketString + " выполнено.");
                msgBox.exec();
            }
            else {
                ui->chatArea->appendPlainText("Ошибка подключения\n");

                QString errorMessage = "Подключение к " + contact->socketString + " не выполнено.\n" + contact->tcpSocket->errorString();
                QMessageBox::critical(this, "Ошибка подключения", errorMessage);
                //contact->tcpSocket->disconnectFromHost();

                delete contact->tcpSocket;
                contact->tcpSocket = nullptr;
            }
        }
        else if (contact->tcpSocket->state() == QAbstractSocket::ConnectedState) {  // Иначе, если сокет подключен, то просто отправляем сообщение
            sendMessage(contact, message);
            ui->chatArea->appendPlainText("От вас:\n" + message + "\n");    // Добавляем сообщение в окно переписки
            ui->inputArea->clear(); // Очищаем окно ввода сообщений
        }
    }
}

void MainWindow::on_tcpSocketError(Contact* contact, int row) {
    qDebug() << "1 " << contact->tcpSocket->state();
    qDebug() << contact->tcpSocket->error();
    qDebug() << "2 " << contact->tcpSocket->errorString();

    if (contact->tcpSocket->error() == 1) { // Если хост отключился, то выводим сообщение об этом
        ui->connectionList->item(row)->setText(contact->socketString);
        if (row == ui->connectionList->currentRow()) {
            ui->chatArea->appendPlainText("Отключено\n");
        }
        QMessageBox msgBox;
        msgBox.setWindowTitle("Отключено");
        msgBox.setText("Отключено от " + contact->socketString);
        msgBox.exec();
    }
    contact->tcpSocket->deleteLater();
}

void MainWindow::sendMessage(Contact* contact, QString message) {
    QByteArray data;
    QDataStream outStream(&data, QIODevice::ReadWrite);

    outStream << quint32(0) << int(0);
    outStream << message;    // Заносим резерв под размер + тип сообщения 0 (т.е. отправка обычного сообщения) + сообщение
    outStream.device()->seek(0);    // Переносим указатель на место резерва
    outStream << quint32(data.size() - sizeof(int) - sizeof(quint32)); // Заносим вместо резерва сам размер сообщения без учета резерва (размера) и типа сообщения
    contact->tcpSocket->write(data);    // Отправляем собеседнику сообщение

    ChatHistory chatHistory {true, message};
    contact->chatHistoryList.push_back(chatHistory);
}

void MainWindow::on_readyRead(Contact* contact) {
    while (contact->tcpSocket->bytesAvailable() < qint64(sizeof(quint32))) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать размер сообщения
        if (!contact->tcpSocket->waitForReadyRead()) {  // Если не готовы для чтения, то выходим
            return;
        }
    }

    QDataStream inStream(contact->tcpSocket);
    quint32 messageSize = 0;
    inStream >> messageSize;    // Читаем размер сообщения

    while (contact->tcpSocket->bytesAvailable() < qint64(sizeof(int))) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать тип сообщения
        if (!contact->tcpSocket->waitForReadyRead()) {
            return;
        }
    }

    int messageType;
    inStream >> messageType;    // Читаем тип сообщения

    if (messageType == 0) {  // Если обычное сообщение, то читаем его
        readMessage(contact, messageSize, inStream);
    }
    else if (messageType == 1) {    // Если нам пришел запрос истории, то отправляем историю
        sendHistory(contact);
    }
    else if (messageType == 2) {    // Если нам пришла история по нашему запросу истории, то читаем историю
        readHistory(contact, messageSize, inStream);
    }
}

void MainWindow::readMessage(Contact* contact, quint32 messageSize, QDataStream& inStream) {
    while (contact->tcpSocket->bytesAvailable() < qint64(messageSize)) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать сообщение целиком
        if (!contact->tcpSocket->waitForReadyRead()) {
            return;
        }
    }

    QString message;
    inStream >> message;    // Читаем сообщение

    int senderIndex = contactList.indexOf(contact);
    if (senderIndex == ui->connectionList->currentRow()) {  // Если нужное окно с чатом открыто, то добавляем сообщение в чат, иначе включаем индикацию нового сообщения
        ui->chatArea->appendPlainText("От собеседника:\n" + message + "\n");
    }
    else {
        QString newContactTitle = "* " + contact->socketString + " (подключено)";
        ui->connectionList->item(senderIndex)->setText(newContactTitle);
    }

    ChatHistory chatHistory {false, message};
    contact->chatHistoryList.push_back(chatHistory);
}

void MainWindow::readHistory(Contact *contact, quint32 messageSize, QDataStream& inStream) {
    while (contact->tcpSocket->bytesAvailable() < qint64(messageSize)) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать сообщение целиком
        if (!contact->tcpSocket->waitForReadyRead()) {
            return;
        }
    }

    contact->chatHistoryList.clear();   // Очищаем всю сохраненную историю, чтобы перезаписать полученной историей

    int chatHistorySize = 0;
    inStream >> chatHistorySize;
    for (int i = 0; i < chatHistorySize; i++) {
        ChatHistory chatHistory;
        inStream >> chatHistory.isYourMessage >> chatHistory.message;
        chatHistory.isYourMessage = !chatHistory.isYourMessage; // инвертируем, т.к. для нас этот флаг должен быть обратным
        contact->chatHistoryList.push_back(chatHistory);
    }

    int row = contactList.indexOf(contact);
    int currentRow = ui->connectionList->currentRow();
    if (row == currentRow) {    // Если выделен контакт, который принимал эту историю сообщений, то надо обновить chatArea (вывести все сообщения из истории)
        updateChatArea(contact->chatHistoryList);
    }
}

void MainWindow::updateChatArea(QList<ChatHistory>& chatHistoryList) {
    ui->chatArea->clear();

    for (auto& chatHistory : chatHistoryList) { // Заполняем окно сообщений (chatArea) историей сообщений
        QString currentMessage;
        if (chatHistory.isYourMessage) {
            currentMessage.append("От вас:\n");
        }
        else {
            currentMessage.append("От собеседника:\n");
        }
        currentMessage.append(chatHistory.message + "\n");
        ui->chatArea->appendPlainText(currentMessage);
    }
}

void MainWindow::on_disconnectBtn_clicked() {
    int currentRow = ui->connectionList->currentRow();
    if (currentRow != -1) { // Если выбран какой-либо контакт
        Contact* contact = contactList.at(currentRow);
        if (contact->tcpSocket->state() == QAbstractSocket::ConnectedState) {    // Если выбранный сокет находится в подключенном состоянии, то отключаем
            contact->tcpSocket->disconnectFromHost();
            ui->connectionList->item(currentRow)->setText(contact->socketString);
            ui->chatArea->appendPlainText("Отключено\n");
            QMessageBox msgBox;
            msgBox.setWindowTitle("Отключено");
            msgBox.setText("Отключено от " + contact->socketString);
            msgBox.exec();
            delete contact->tcpSocket;
            contact->tcpSocket = nullptr;
            //contact->tcpSocket->deleteLater();
        }
    }
}

void MainWindow::on_disconnected(Contact* contact, int row) {
    delete contact->tcpSocket;
    contact->tcpSocket = nullptr;
}

void MainWindow::on_historyRequestBtn_clicked() {
    int currentIndex = ui->connectionList->currentRow();
    Contact* contact = contactList.at(currentIndex);

    if (currentIndex != -1) {
        if (contact->tcpSocket == nullptr) {    // Если это первая попытка подключения, то нужно выделить память под сокет
            contact->tcpSocket = new QTcpSocket();
        }
        if (contact->tcpSocket->state() != QAbstractSocket::ConnectedState && contact->tcpSocket->state() != QAbstractSocket::UnconnectedState) {   // Если сокет не подключен, но и не отключен, то значит, что мы недавно отключались и память была очищена
            contact->tcpSocket = new QTcpSocket();
        }
        if (contact->tcpSocket->state() != QAbstractSocket::ConnectedState) {   // Если сокет не подключен, то подключаемся (без отправки запроса). Иначе, если сокет подключен, то просто отправляем запрос
            QHostAddress ip = contact->serverIP;
            quint16 port = contact->serverPort;

            contact->tcpSocket->connectToHost(ip, port);
            if (contact->tcpSocket->waitForConnected(3000)) {   // Если мы подключились в течение 3 секунд, то выводим сообщение об успехе, иначе - об ошибке
                ui->connectionList->item(currentIndex)->setText(contact->socketString + " (подключено)");
                connect(contact->tcpSocket, &QTcpSocket::readyRead, [this, contact]() {this->on_readyRead(contact);});
                //connect(contact->tcpSocket, &QTcpSocket::disconnected, [this, contact, currentIndex]() {this->on_disconnected(contact, currentIndex);});
                //connect(contact->tcpSocket, &QTcpSocket::disconnected, contact->tcpSocket, &QTcpSocket::deleteLater);
                connect(contact->tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [this, contact, currentIndex]() {this->on_tcpSocketError(contact, currentIndex);});

                ui->chatArea->appendPlainText("Подключено\n");

                QMessageBox msgBox;
                msgBox.setWindowTitle("Подключено");
                msgBox.setText("Подключение к " + contact->socketString + " выполнено.");
                msgBox.exec();
            }
            else {
                ui->chatArea->appendPlainText("Ошибка подключения\n");

                QString errorMessage = "Подключение к " + contact->socketString + " не выполнено.\n" + contact->tcpSocket->errorString();
                QMessageBox::critical(this, "Ошибка подключения", errorMessage);
                //contact->tcpSocket->disconnectFromHost();

                delete contact->tcpSocket;
                contact->tcpSocket = nullptr;
            }
        }
        else if (contact->tcpSocket->state() == QAbstractSocket::ConnectedState) {  // Иначе, если сокет подключен, то просто отправляем сообщение
            sendHistoryRequest(contact);
        }
    }
}

void MainWindow::sendHistoryRequest(Contact* contact) {
    QByteArray data;
    QDataStream outStream(&data, QIODevice::ReadWrite);

    outStream << quint32(0) << int(1);    // Заносим резерв под размер и тип сообщения 1 (т.е. отправка запроса истории)
    outStream.device()->seek(0);    // Переносим указатель на место резерва
    outStream << quint32(data.size() - sizeof(int) - sizeof(quint32)); // Заносим вместо резерва сам размер сообщения без учета резерва (размера) и типа сообщения
    contact->tcpSocket->write(data);    // Отправляем собеседнику запрос
}

void MainWindow::sendHistory(Contact* contact) {
    QByteArray data;
    QDataStream outStream(&data, QIODevice::ReadWrite);

    outStream << quint32(0) << int(2);    // Заносим резерв под размер и тип сообщения 2 (т.е. отправка истории)
    outStream << contact->chatHistoryList.size();
    for (auto& chatHistory : contact->chatHistoryList) {
        outStream << chatHistory.isYourMessage << chatHistory.message;
    }
    outStream.device()->seek(0);    // Переносим указатель на место резерва
    outStream << quint32(data.size() - sizeof(int) - sizeof(quint32)); // Заносим вместо резерва сам размер сообщения без учета резерва (размера) и типа сообщения
    contact->tcpSocket->write(data);    // Отправляем собеседнику историю сообщений
}
