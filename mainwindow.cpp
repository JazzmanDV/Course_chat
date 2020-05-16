#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);  
    setupServer();
}

void MainWindow::setupServer() {
    mainTcpServer = new QTcpServer(this);   // Создаём переменную сервера
    if (!mainTcpServer->listen(QHostAddress::Any, 64000)) {   // Если сервер не может слушать, то выводим ошибку и закрываем приложение
        QMessageBox::critical(this, tr("Ошибка старта сервера"), tr("Невозможно запустить серверную часть: %1.").arg(mainTcpServer->errorString()));
        close();
        exit(1);
    }
    QList<QHostAddress> addressList = QNetworkInterface::allAddresses();    // Получаем список адресов своего ПК
    qDebug() << addressList;    // Отладка
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

MainWindow::~MainWindow()
{
    delete mainTcpServer;
    delete ui;
}

void MainWindow::on_newConnection() {
    QTcpSocket *tcpSocket = mainTcpServer->nextPendingConnection();
    Contact* contact;
    qDebug() << "Peer address: " << tcpSocket->peerAddress() << ":" << tcpSocket->peerPort() << endl << tcpSocket->peerName();

    bool isContactExists = false;
    for (int i = 0; i < contactList.size(); i++) {
        Contact* tempContact = contactList.at(i);
        qDebug() << tempContact->serverIP;
        if (tcpSocket->peerAddress() == tempContact->serverIP) {  // Если адрес подключения из сокета совпадает с одним из ранее добавленных контактов
            contact = tempContact;
            isContactExists = true;
            break;
        }
    }

    if (!isContactExists) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Новое подключение", "К вам пытается подключиться " + tcpSocket->peerAddress().toString() + ".\n Создать подключение?");
        if (reply == QMessageBox::StandardButton::Yes) {
            addContactToUI(tcpSocket->peerAddress().toString(), QString::number(tcpSocket->peerPort()));
            contact = new Contact();
            contact->serverIP = tcpSocket->peerAddress();
            contact->serverPort = tcpSocket->peerPort();
            contactList.push_back(contact);
        }
        else {
            tcpSocket->disconnectFromHost();
            delete tcpSocket;
            return;
        }
    }

    contact->tcpSocket = tcpSocket;
    connect(contact->tcpSocket, &QTcpSocket::readyRead, [this, contact]() {this->readMessage(contact);});
    connect(contact->tcpSocket, &QTcpSocket::disconnected, contact->tcpSocket, &QTcpSocket::deleteLater); // TODO Не то написано, см. на сайте правильную версию
    connect(contact->tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(on_tcpSocketError(QAbstractSocket::SocketError)));

    QString welcomeMessage = "Connected to " + mainTcpServer->serverAddress().toString() + ":" + QString::number(mainTcpServer->serverPort());  // TODO: на самом деле адрес и порт отличаются от переменных в классе
    sendMessage(contact, welcomeMessage);
}

void MainWindow::sendMessage(Contact* contact, QString message) {
    QByteArray data;
    QDataStream outStream(&data, QIODevice::ReadWrite);

    outStream << quint32(0);
    outStream << message;
    outStream.device()->seek(0);
    outStream << quint32(data.size() - sizeof(quint32));
    contact->tcpSocket->write(data);

    ui->chatArea->appendPlainText("От вас: " + message);

    ChatHistory chatHistory {true, message};
    contact->chatHistoryList.push_back(chatHistory);
}

void MainWindow::readMessage(Contact* contact) {
    while (contact->tcpSocket->bytesAvailable() < qint64(sizeof(quint32))) {
        if (!contact->tcpSocket->waitForReadyRead()) {
            return;
        }
    }

    QDataStream inStream(contact->tcpSocket);
    quint32 messageSize = 0;
    inStream >> messageSize;

    while (contact->tcpSocket->bytesAvailable() < qint64(messageSize)) {
        if (!contact->tcpSocket->waitForReadyRead()) {
            return;
        }
    }

    QByteArray buffer;
    buffer.resize(messageSize);
    inStream.readRawData(buffer.data(), messageSize);
    int senderIndex = contactList.indexOf(contact);
    if (senderIndex == ui->connectionList->currentRow()) {  // Если нужное окно с чатом открыто, то добавляем сообщение в чат, иначе включаем индикацию нового сообщения
        ui->chatArea->appendPlainText("От собеседника: " + buffer);
    }
    else {
        ui->connectionList->item(senderIndex, 0)->setText(contact->tcpSocket->peerAddress().toString() + QString::number(contact->tcpSocket->peerPort()) + " (!)");
    }

    ChatHistory chatHistory {false, buffer};
    contact->chatHistoryList.push_back(chatHistory);
}

void MainWindow::on_tcpSocketError(QAbstractSocket::SocketError socketError) {
    QMessageBox::critical(this, tr("Подключение к серверу"), tr("Невозможно подключиться к серверу.\n Код ошибки: %1") .arg(socketError));
}

// Остановился здесь
void MainWindow::on_addNewConnectionBtn_clicked() {
    CreateConnectionWindow* createConnectionWindow = new CreateConnectionWindow(this);
    QObject::connect(createConnectionWindow, &CreateConnectionWindow::addNewConnection, this, &MainWindow::on_addNewConnection);
    createConnectionWindow->open();
}

void MainWindow::on_addNewConnection(QString ip, quint16 port, Contact* tcpConnection) {
    if (tcpConnection == nullptr) {
        tcpConnection = new Contact();
        QTcpSocket* tcpSocket = new QTcpSocket(this);        
        tcpConnection->tcpSocket = tcpSocket;
        contactList.push_back(tcpConnection);
        addContactToUI(ip, QString::number(port));
    }

    connect(tcpConnection->tcpSocket, &QTcpSocket::readyRead, [this, tcpConnection]() {this->readMessage(tcpConnection);});
    connect(tcpConnection->tcpSocket, &QTcpSocket::disconnected, tcpConnection->tcpSocket, &QTcpSocket::deleteLater); // TODO Не то написано, см. на сайте правильную версию
    connect(tcpConnection->tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(on_tcpSocketError(QAbstractSocket::SocketError)));

    if (tcpConnection->tcpSocket->state() != QAbstractSocket::ConnectedState) {
        tcpConnection->tcpSocket->connectToHost(ip, port); // TODO: Можно попробовать вместо displayTcpSocketError try-catch
    }
}

void MainWindow::addContactToUI(QString ip, QString port) {
    ui->connectionList->insertRow(ui->connectionList->rowCount());
    ui->connectionList->setItem(ui->connectionList->rowCount()-1, 0, new QTableWidgetItem(ip+":" + port));
}

void MainWindow::on_connectionList_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    ui->chatArea->clear();
    int currentIndex = ui->connectionList->currentIndex().row();
    QList<ChatHistory> chatHistoryList = contactList.at(currentIndex)->chatHistoryList;
    QTcpSocket* currentTcpSocket = contactList.at(currentIndex)->tcpSocket;
    ui->connectionIPLabel->setText("IP собеседника: " + currentTcpSocket->peerAddress().toString() + ":" + QString::number(currentTcpSocket->peerPort()));
    QString currentMessage;
    for (auto& chatHistory : chatHistoryList) {
        if (chatHistory.isYourMessage) {
            currentMessage.append("От вас: ");
        }
        else {
            currentMessage.append("От собеседника: ");
        }
        currentMessage.append(chatHistory.message);
    }
    ui->chatArea->appendPlainText(currentMessage);
}

void MainWindow::on_sendBtn_clicked()
{
    QString message = ui->inputArea->toPlainText();
    if (message != "") {
        int currentIndex = ui->connectionList->currentIndex().row();
        if (currentIndex >= 0 && currentIndex < contactList.size()) {
            sendMessage(contactList.at(currentIndex), message);
        }
    }
}
