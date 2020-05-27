#include "Contact.h"
#include <QDataStream>

Contact::Contact(QHostAddress serverIP, quint16 serverPort, QString socketString) {
    tcpSocket = nullptr;

    this->serverIP = serverIP;
    this->serverPort = serverPort;
    this->socketString = socketString;
}

Contact::~Contact() {
//    if (tcpSocket != nullptr) {
//        delete tcpSocket;
//    }
    if (tcpSocket != nullptr) {
        tcpSocket->disconnectFromHost();
    }
}

void Contact::setTcpSocket(QTcpSocket *socket) {
    this->tcpSocket = socket;
}

QTcpSocket *Contact::getTcpSocket() {
    return tcpSocket;
}

QHostAddress Contact::getServerIP() const {
    return serverIP;
}

void Contact::setServerIP(const QHostAddress &serverIP) {
    this->serverIP = serverIP;
}

quint16 Contact::getServerPort() const {
    return serverPort;
}

void Contact::setServerPort(const quint16 &serverPort) {
    this->serverPort = serverPort;
}

QString Contact::getSocketString() const {
    return socketString;
}

void Contact::setSocketString(const QString &socketString) {
    this->socketString = socketString;
}

QList<ChatHistory> Contact::getChatHistoryList() const {
    return chatHistoryList;
}

void Contact::setChatHistoryList(const QList<ChatHistory> &chatHistoryList) {
    this->chatHistoryList = chatHistoryList;
}

void Contact::addChatHistory(ChatHistory chatHistory) {
    this->chatHistoryList.push_back(chatHistory);
}

void Contact::disconnectFromHost() {
    tcpSocket->disconnectFromHost();
}

void Contact::deleteTcpSocket() {
    if (tcpSocket != nullptr) {
        delete tcpSocket;
        tcpSocket = nullptr;
    }
}

int Contact::sendMessage(QString message) {
    if (tcpSocket == nullptr) {    // Если это первая попытка подключения, то нужно выделить память под сокет
        tcpSocket = new QTcpSocket();
    }
    if (tcpSocket->state() != QAbstractSocket::ConnectedState && tcpSocket->state() != QAbstractSocket::UnconnectedState) {   // Если сокет не подключен, но и не отключен, то значит, что мы недавно отключались и память была очищена
        tcpSocket = new QTcpSocket();
    }
    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {   // Если сокет не подключен, то подключаемся (без отправки сообщения). Иначе, если сокет подключен, то просто отправляем сообщение
        QHostAddress ip = serverIP;
        quint16 port = serverPort;

        tcpSocket->connectToHost(ip, port);
        if (tcpSocket->waitForConnected(3000)) {   // Если мы подключились в течение 3 секунд, то выводим сообщение об успехе, иначе - об ошибке
            return 0;
        }
        else {
            //contact->tcpSocket->disconnectFromHost();
            delete tcpSocket;
            tcpSocket = nullptr;
            return 1;
        }
    }
    else if (isConnected()) {
        QByteArray data;
        QDataStream outStream(&data, QIODevice::ReadWrite);

        outStream << quint32(0) << int(0);
        outStream << message;    // Заносим резерв под размер + тип сообщения 0 (т.е. отправка обычного сообщения) + сообщение
        outStream.device()->seek(0);    // Переносим указатель на место резерва
        outStream << quint32(data.size() - sizeof(int) - sizeof(quint32)); // Заносим вместо резерва сам размер сообщения без учета резерва (размера) и типа сообщения
        tcpSocket->write(data);    // Отправляем собеседнику сообщение

        ChatHistory chatHistory {true, message};
        chatHistoryList.push_back(chatHistory);

        return 2;
    }
}

int Contact::sendHistoryRequest() {
    if (tcpSocket == nullptr) {    // Если это первая попытка подключения, то нужно выделить память под сокет
        tcpSocket = new QTcpSocket();
    }
    if (tcpSocket->state() != QAbstractSocket::ConnectedState && tcpSocket->state() != QAbstractSocket::UnconnectedState) {   // Если сокет не подключен, но и не отключен, то значит, что мы недавно отключались и память была очищена
        tcpSocket = new QTcpSocket();
    }
    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {   // Если сокет не подключен, то подключаемся (без отправки сообщения). Иначе, если сокет подключен, то просто отправляем сообщение
        QHostAddress ip = serverIP;
        quint16 port = serverPort;

        tcpSocket->connectToHost(ip, port);
        if (tcpSocket->waitForConnected(3000)) {   // Если мы подключились в течение 3 секунд, то выводим сообщение об успехе, иначе - об ошибке
            return 0;
        }
        else {
            //contact->tcpSocket->disconnectFromHost();
            delete tcpSocket;
            tcpSocket = nullptr;
            return 1;
        }
    }
    else if (isConnected())  {
        QByteArray data;
        QDataStream outStream(&data, QIODevice::ReadWrite);

        outStream << quint32(0) << int(1);    // Заносим резерв под размер и тип сообщения 1 (т.е. отправка запроса истории)
        outStream.device()->seek(0);    // Переносим указатель на место резерва
        outStream << quint32(data.size() - sizeof(int) - sizeof(quint32)); // Заносим вместо резерва сам размер сообщения без учета резерва (размера) и типа сообщения
        tcpSocket->write(data);    // Отправляем собеседнику запрос
    }
}

void Contact::sendHistory() {
    if (isConnected())  {
        QByteArray data;
        QDataStream outStream(&data, QIODevice::ReadWrite);

        outStream << quint32(0) << int(2);    // Заносим резерв под размер и тип сообщения 2 (т.е. отправка истории)
        outStream << chatHistoryList.size();
        for (auto& chatHistory : chatHistoryList) {
            outStream << chatHistory.isYourMessage << chatHistory.message;
        }
        outStream.device()->seek(0);    // Переносим указатель на место резерва
        outStream << quint32(data.size() - sizeof(int) - sizeof(quint32)); // Заносим вместо резерва сам размер сообщения без учета резерва (размера) и типа сообщения
        tcpSocket->write(data);    // Отправляем собеседнику историю сообщений
    }
}

int Contact::readData() {
    QDataStream inStream(tcpSocket);

    quint32 messageSize = readMessageSize(inStream);

    int dataType = readDataType(inStream);

    if (dataType == 0) {  // Если обычное сообщение, то читаем его
        readMessage(messageSize, inStream);
        return 0;
    }
    else if (dataType == 1) {    // Если нам пришел запрос истории, то отправляем историю
        sendHistory();
        return 1;
    }
    else if (dataType == 2) {    // Если нам пришла история по нашему запросу истории, то читаем историю
        readHistory(messageSize, inStream);
        return 2;
    }
    else {
        qDebug() << "Прочитан несуществующий тип сообщения";
        return -1;
    }
}

quint32 Contact::readMessageSize(QDataStream& inStream) {
    while (tcpSocket->bytesAvailable() < qint64(sizeof(quint32))) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать размер сообщения
        if (!tcpSocket->waitForReadyRead()) {  // Если не готовы для чтения, то выходим
            return -1;
        }
    }

    quint32 messageSize = 0;
    inStream >> messageSize;    // Читаем размер сообщения

    return messageSize;
}

int Contact::readDataType(QDataStream& inStream) {
    while (tcpSocket->bytesAvailable() < qint64(sizeof(int))) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать тип сообщения
        if (!tcpSocket->waitForReadyRead()) {
            return -1;
        }
    }

    int dataType;
    inStream >> dataType;    // Читаем тип сообщения

    return dataType;
}

void Contact::readMessage(quint32 messageSize, QDataStream &inStream) {
    while (tcpSocket->bytesAvailable() < qint64(messageSize)) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать сообщение целиком
        if (!tcpSocket->waitForReadyRead()) {
            return;
        }
    }

    QString message;
    inStream >> message;    // Читаем сообщение

    ChatHistory chatHistory {false, message};
    chatHistoryList.push_back(chatHistory);
}

void Contact::readHistory(quint32 messageSize, QDataStream &inStream) {
    while (tcpSocket->bytesAvailable() < qint64(messageSize)) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать сообщение целиком
        if (!tcpSocket->waitForReadyRead()) {
            return;
        }
    }

    chatHistoryList.clear();   // Очищаем всю сохраненную историю, чтобы перезаписать полученной историей

    int chatHistorySize = 0;
    inStream >> chatHistorySize;
    for (int i = 0; i < chatHistorySize; i++) {
        ChatHistory chatHistory;
        inStream >> chatHistory.isYourMessage >> chatHistory.message;
        chatHistory.isYourMessage = !chatHistory.isYourMessage; // инвертируем, т.к. для нас этот флаг должен быть обратным
        chatHistoryList.push_back(chatHistory);
    }
}

bool Contact::isConnected() const {
    bool res = false;
    if (tcpSocket != nullptr) {
        if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
            res = true;
        }
    }
    return res;
}

QString Contact::getErrorString() {
    return tcpSocket->errorString();
}
