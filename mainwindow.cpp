#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QException>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) , ui(new Ui::MainWindow) {
    ui->setupUi(this);  

    setupServer();  // Настраиваем сервер
    loadFromFile(); // Выгружаем из файла все данные в contactList
}

MainWindow::~MainWindow() {
    delete mainTcpServer;
    delete ui;

    saveToFile();
}

void MainWindow::setupServer() {
    mainTcpServer = new QTcpServer(this);   // Создаём переменную сервера
    if (!mainTcpServer->listen(QHostAddress::Any, 64000)) {   // Если сервер не может слушать, то выводим ошибку и закрываем приложение
        QMessageBox::critical(this, tr("Ошибка старта сервера"), tr("Невозможно запустить серверную часть: %1.").arg(mainTcpServer->errorString()));
        close();
        exit(1);
    }

    QHostAddress mainServerIP;

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

    quint16 mainServerPort = mainTcpServer->serverPort();
    ui->yourIPLabel->setText("Ваш IP: " + mainServerIP.toString() + ":" + QString::number(mainServerPort));
    connect(mainTcpServer, &QTcpServer::newConnection, this, &MainWindow::on_newConnection);    // Соединяем нашу серверную часть с сигналом о том, что поступило новое соединение
}

void MainWindow::loadFromFile() {
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
                QString errorMessage = "Ошибка чтения файла " + fileInfo.filePath() + ".\nВозможно, вы изменили его содержимое или содержимое Database.";
                QMessageBox::critical(this, "Ошибка чтения файла", errorMessage);
                file.close();
                file.remove();
                continue;
            }

            QHostAddress serverIP;
            quint16 serverPort;
            QString socketString;
            stream >> serverIP >> serverPort >> socketString;

            Contact* contact = new Contact(serverIP, serverPort, socketString);

            int chatHistoryListSize;
            stream >> chatHistoryListSize;
            for (int i = 0; i < chatHistoryListSize; i++) {
                ChatHistory chatHistory;
                stream >> chatHistory.isYourMessage >> chatHistory.message;
                contact->addChatHistory(chatHistory);
            }

            contactList.push_back(contact);

            addContactToUI(socketString);
            file.close();
        }
        else {
            QString errorMessage = "Невозможно открыть файл " + fileInfo.filePath() + ".\nВозможно, файл был повреждён.";
            QMessageBox::critical(this, "Ошибка открытия файла", errorMessage);
        }
    }
}

void MainWindow::saveToFile() {
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
        QString serverIP = contact->getServerIP().toString();
        QString serverPort = QString::number(contact->getServerPort());
        QString fileName = "Database/" + serverIP + " " + serverPort + ".dat";
        QFile file(fileName);

        if (file.open(QIODevice::WriteOnly)) {
            QString socketString = contact->getSocketString();
            QList<ChatHistory> chatHistoryList = contact->getChatHistoryList();

            QDataStream stream(&file);
            stream << PSEUDO_HASH;
            stream << contact->getServerIP() << contact->getServerPort() << contact->getSocketString();
            stream << chatHistoryList.size();
            for (auto& chatHistory : chatHistoryList) {
                stream << chatHistory.isYourMessage << chatHistory.message;
            }
            file.close();
        }
        else {
            QString errorMessage = "Невозможно открыть файл " + fileName + ".\nВозможно, не хватает места на диске.";
            QMessageBox::critical(this, "Ошибка открытия файла", errorMessage);
        }
    }
}

void MainWindow::on_newConnection() {
    QTcpSocket *newConnection = mainTcpServer->nextPendingConnection(); // Сохраняем новое подключение в переменную

    Contact* contact = nullptr;
    bool isContactExists = false;
    for (int i = 0; i < contactList.size(); i++) {  // Поиск, существует ли уже запись для такого адреса
        Contact* tempContact = contactList.at(i);
        QString tempPeerAddress = "::ffff:" + tempContact->getServerIP().toString();
        if (newConnection->peerAddress().toString() == tempPeerAddress) {  // Если адрес подключения из сокета совпадает с одним из ранее добавленных контактов
            contact = tempContact;
            isContactExists = true;
            break;
        }
    }

    if (!isContactExists) { // Если контакт с таким адресом еще не создан, то создаем
        QHostAddress serverIP = QHostAddress(newConnection->peerAddress().toString().replace(0, 7, ""));
        quint16 serverPort = 64000;
        QString socketString = serverIP.toString() + ":" + QString::number(serverPort);
        contact = new Contact(serverIP, serverPort, socketString);

        contactList.push_back(contact);

        addContactToUI(socketString);
    }

    contact->setTcpSocket(newConnection);
    int row = contactList.indexOf(contact);

    connect(contact->getTcpSocket(), &QTcpSocket::readyRead, [this, contact]() {this->on_readyRead(contact);});
    //connect(contact->tcpSocket, &QTcpSocket::disconnected, [this, contact, row]() {this->on_disconnected(contact, row);});
    //connect(contact->tcpSocket, &QTcpSocket::disconnected, contact->tcpSocket, &QTcpSocket::deleteLater);
    connect(contact->getTcpSocket(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [this, contact, row]() {this->on_tcpSocketError(contact, row);});

    if (ui->connectionList->currentRow() == row) {  // Если текущий контакт выделен, то добавляем в окно переписки "Подключено"
        ui->chatArea->appendPlainText("Подключено\n");
    }
    ui->connectionList->item(row)->setText(contact->getSocketString() + " (подключено)");

    QMessageBox msgBox;
    msgBox.setWindowTitle("Подключено");
    msgBox.setText("Подключение к " + contact->getSocketString() + " выполнено.");
    msgBox.exec();
}

void MainWindow::addContactToUI(QString socketString) {
    ui->connectionList->insertItem(ui->connectionList->count(), socketString);
}

void MainWindow::on_addContactBtn_clicked() {
    CreateConnectionWindow* createConnectionWindow = new CreateConnectionWindow(this);
    connect(createConnectionWindow, &CreateConnectionWindow::addContact, this, &MainWindow::on_addContact);    // Привязываем вызов функции on_addContact к сигналу addContact
    createConnectionWindow->open();
}

void MainWindow::on_addContact(QHostAddress serverIP, quint16 serverPort) {
    QString socketString = serverIP.toString() + ":" + QString::number(serverPort);

    for (auto& contact : contactList) {
        if (contact->getSocketString() == socketString) {
            QMessageBox::critical(this, "Ошибка добавления контакта.", "Такой контакт уже существует.");
            return;
        }
    }

    Contact* contact = new Contact(serverIP, serverPort, socketString);
    contactList.push_back(contact);

    addContactToUI(socketString);
}

void MainWindow::on_delContactBtn_clicked() {
    int currentIndex = ui->connectionList->currentIndex().row();
    Contact* contact = contactList.at(currentIndex);

    if (currentIndex != -1) {   // Если выбран какой-то из контактов, то отключаемся от хоста и освобождаем память
        //contact->disconnectFromHost();  // TODO: возможно не нужно
        delete contact;
        contactList.removeAt(currentIndex);

        delete ui->connectionList->takeItem(currentIndex);
        ui->chatArea->clear();
    }
}

void MainWindow::on_delAllContactsBtn_clicked() {
    for (auto& contact : contactList) {
        //contact->disconnectFromHost();  // TODO: возможно не нужно
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

        QString newItemText = contact->getSocketString();
        if (contact->isConnected()) {
            newItemText.append(" (подключено)");
        }
        currentItem->setText(newItemText);

        updateChatArea(contact->getChatHistoryList());
    }
}

void MainWindow::updateChatArea(QList<ChatHistory> chatHistoryList) {
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

void MainWindow::on_sendMessageBtn_clicked() {
    QString message = ui->inputArea->toPlainText(); // Сохраняем введенное сообщение в переменную
    int currentIndex = ui->connectionList->currentRow();

    if (message != "" && currentIndex != -1) {    // Если отправляемое сообщение не пустое и был выделен какой-либо контакт
        Contact* contact = contactList.at(currentIndex);
        int status = contact->sendMessage(message);

        if (status == 0) {
            connect(contact->getTcpSocket(), &QTcpSocket::readyRead, [this, contact]() {this->on_readyRead(contact);});
            //connect(contact->tcpSocket, &QTcpSocket::disconnected, [this, contact, row]() {this->on_disconnected(contact, row);});
            //connect(contact->tcpSocket, &QTcpSocket::disconnected, contact->tcpSocket, &QTcpSocket::deleteLater);
            connect(contact->getTcpSocket(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [this, contact, currentIndex]() {this->on_tcpSocketError(contact, currentIndex);});

            ui->connectionList->item(currentIndex)->setText(contact->getSocketString() + " (подключено)");
            ui->chatArea->appendPlainText("Подключено\n");

            QMessageBox msgBox;
            msgBox.setWindowTitle("Подключено");
            msgBox.setText("Подключение к " + contact->getSocketString() + " выполнено.\n Нажмите на кнопку еще раз");
            msgBox.exec();
        }
        else if (status == 1) {
            ui->chatArea->appendPlainText("Ошибка подключения\n");
            QString errorMessage = "Подключение к " + contact->getSocketString() + " не выполнено.\n";
            QMessageBox::critical(this, "Ошибка подключения", errorMessage);
        }
        else if (status == 2) {
            ui->chatArea->appendPlainText("От вас:\n" + message + "\n");    // Добавляем сообщение в окно переписки
            ui->inputArea->clear(); // Очищаем окно ввода сообщений
        }
    }
}

void MainWindow::on_historyRequestBtn_clicked() {
    int currentIndex = ui->connectionList->currentRow();

    if (currentIndex != -1) {
        Contact* contact = contactList.at(currentIndex);
        int status = contact->sendHistoryRequest();

        if (status == 0) {
            connect(contact->getTcpSocket(), &QTcpSocket::readyRead, [this, contact]() {this->on_readyRead(contact);});
            //connect(contact->tcpSocket, &QTcpSocket::disconnected, [this, contact, row]() {this->on_disconnected(contact, row);});
            //connect(contact->tcpSocket, &QTcpSocket::disconnected, contact->tcpSocket, &QTcpSocket::deleteLater);
            connect(contact->getTcpSocket(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [this, contact, currentIndex]() {this->on_tcpSocketError(contact, currentIndex);});

            ui->connectionList->item(currentIndex)->setText(contact->getSocketString() + " (подключено)");
            ui->chatArea->appendPlainText("Подключено\n");

            QMessageBox msgBox;
            msgBox.setWindowTitle("Подключено");
            msgBox.setText("Подключение к " + contact->getSocketString() + " выполнено.\n Нажмите на кнопку еще раз");
            msgBox.exec();
        }
        else if (status == 1) {
            ui->chatArea->appendPlainText("Ошибка подключения\n");
            QString errorMessage = "Подключение к " + contact->getSocketString() + " не выполнено.\n";
            QMessageBox::critical(this, "Ошибка подключения", errorMessage);
        }
    }
}

void MainWindow::on_tcpSocketError(Contact* contact, int row) {
    qDebug() << "1 " << contact->getTcpSocket()->state();
    qDebug() << "2 " << contact->getTcpSocket()->error();
    qDebug() << "3 " << contact->getTcpSocket()->errorString();

    if (contact->getTcpSocket()->error() == QAbstractSocket::SocketError::RemoteHostClosedError) { // Если хост отключился, то выводим сообщение об этом
        ui->connectionList->item(row)->setText(contact->getSocketString());
        if (row == ui->connectionList->currentRow()) {
            ui->chatArea->appendPlainText("Отключено\n");
        }
        QMessageBox msgBox;
        msgBox.setWindowTitle("Отключено");
        msgBox.setText("Отключено от " + contact->getSocketString());
        msgBox.exec();
    }
    contact->getTcpSocket()->deleteLater();
}

void MainWindow::on_readyRead(Contact* contact) {
    int dataType = contact->readData();

    if (dataType == 0) {
        int senderIndex = contactList.indexOf(contact);
        if (senderIndex == ui->connectionList->currentRow()) {  // Если нужное окно с чатом открыто, то добавляем сообщение в чат, иначе включаем индикацию нового сообщения
            QString message = contact->getChatHistoryList().last().message;
            ui->chatArea->appendPlainText("От собеседника:\n" + message + "\n");
        }
        else {
            QString newContactTitle = "* " + contact->getSocketString() + " (подключено)";
            ui->connectionList->item(senderIndex)->setText(newContactTitle);
        }
    }
    else if (dataType == 2) {
        int row = contactList.indexOf(contact);
        int currentRow = ui->connectionList->currentRow();
        if (row == currentRow) {    // Если выделен контакт, который принимал эту историю сообщений, то надо обновить chatArea (вывести все сообщения из истории)
            updateChatArea(contact->getChatHistoryList());
        }
    }
}

void MainWindow::on_disconnectBtn_clicked() {
    int currentRow = ui->connectionList->currentRow();
    if (currentRow != -1) { // Если выбран какой-либо контакт
        Contact* contact = contactList.at(currentRow);
        if (contact->isConnected()) {    // Если выбранный сокет находится в подключенном состоянии, то отключаем
            contact->disconnectFromHost();
            contact->deleteTcpSocket();

            ui->connectionList->item(currentRow)->setText(contact->getSocketString());
            ui->chatArea->appendPlainText("Отключено\n");
            QMessageBox msgBox;
            msgBox.setWindowTitle("Отключено");
            msgBox.setText("Отключено от " + contact->getSocketString());
            msgBox.exec();
        }
    }
}
