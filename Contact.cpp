#include <Contact.h>
#include <QDataStream>

Contact::Contact() {
    tcpSocket = nullptr;
}

Contact::Contact(QHostAddress serverIP, quint16 serverPort, QString socketString) {
    tcpSocket = nullptr;

    this->serverIP = serverIP;
    this->serverPort = serverPort;
    this->socketString = socketString;
}

Contact::~Contact()
{
    if (tcpSocket != nullptr) {
        delete tcpSocket;
    }
}

void Contact::setTcpSocket(QTcpSocket *socket)
{
    this->tcpSocket = socket;
}

QHostAddress Contact::getServerIP() const
{
    return serverIP;
}

void Contact::setServerIP(const QHostAddress &serverIP)
{
    this->serverIP = serverIP;
}

quint16 Contact::getServerPort() const
{
    return serverPort;
}

void Contact::setServerPort(const quint16 &serverPort)
{
    this->serverPort = serverPort;
}

QString Contact::getSocketString() const
{
    return socketString;
}

void Contact::setSocketString(const QString &socketString)
{
    this->socketString = socketString;
}

QList<ChatHistory> Contact::getChatHistoryList() const
{
    return chatHistoryList;
}

void Contact::setChatHistoryList(const QList<ChatHistory> &chatHistoryList)
{
    this->chatHistoryList = chatHistoryList;
}

void Contact::addChatHistory(ChatHistory chatHistory) {
    this->chatHistoryList.push_back(chatHistory);
}

void Contact::disconnectFromHost()
{
    tcpSocket->disconnectFromHost();
    //tcpSocket->deleteLater();
}

void Contact::deleteTcpSocket()
{
    if (tcpSocket != nullptr) {
        delete tcpSocket;
        tcpSocket = nullptr;
    }
}

void Contact::sendMessage(QString message)
{
    if (isConnected()) {
        QByteArray data;
        QDataStream outStream(&data, QIODevice::ReadWrite);

        outStream << quint32(0) << int(0);
        outStream << message;    // Заносим резерв под размер + тип сообщения 0 (т.е. отправка обычного сообщения) + сообщение
        outStream.device()->seek(0);    // Переносим указатель на место резерва
        outStream << quint32(data.size() - sizeof(int) - sizeof(quint32)); // Заносим вместо резерва сам размер сообщения без учета резерва (размера) и типа сообщения
        tcpSocket->write(data);    // Отправляем собеседнику сообщение

        ChatHistory chatHistory {true, message};
        chatHistoryList.push_back(chatHistory);
    }
}

void Contact::sendHistoryRequest()
{
    if (isConnected())  {
        QByteArray data;
        QDataStream outStream(&data, QIODevice::ReadWrite);

        outStream << quint32(0) << int(1);    // Заносим резерв под размер и тип сообщения 1 (т.е. отправка запроса истории)
        outStream.device()->seek(0);    // Переносим указатель на место резерва
        outStream << quint32(data.size() - sizeof(int) - sizeof(quint32)); // Заносим вместо резерва сам размер сообщения без учета резерва (размера) и типа сообщения
        tcpSocket->write(data);    // Отправляем собеседнику запрос
    }
}

void Contact::sendHistory()
{
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

void Contact::readData()
{
    QDataStream inStream(tcpSocket);

    quint32 messageSize = readMessageSize(inStream);

    int dataType = readDataType(inStream);

    if (dataType == 0) {  // Если обычное сообщение, то читаем его
        readMessage(messageSize, inStream);
    }
    else if (dataType == 1) {    // Если нам пришел запрос истории, то отправляем историю
        sendHistory();
    }
    else if (dataType == 2) {    // Если нам пришла история по нашему запросу истории, то читаем историю
        readHistory(messageSize, inStream);
    }
}

quint32 Contact::readMessageSize(QDataStream& inStream)
{
    while (tcpSocket->bytesAvailable() < qint64(sizeof(quint32))) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать размер сообщения
        if (!tcpSocket->waitForReadyRead()) {  // Если не готовы для чтения, то выходим
            return -1;
        }
    }

    quint32 messageSize = 0;
    inStream >> messageSize;    // Читаем размер сообщения

    return messageSize;
}

int Contact::readDataType(QDataStream& inStream)
{
    while (tcpSocket->bytesAvailable() < qint64(sizeof(int))) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать тип сообщения
        if (!tcpSocket->waitForReadyRead()) {
            return -1;
        }
    }

    int dataType;
    inStream >> dataType;    // Читаем тип сообщения

    return dataType;
}

void Contact::readMessage(quint32 messageSize, QDataStream &inStream)
{
    while (tcpSocket->bytesAvailable() < qint64(messageSize)) {    // Ожидаем, пока кол-во доступных для чтения байтов будет достаточно, чтобы прочитать сообщение целиком
        if (!tcpSocket->waitForReadyRead()) {
            return;
        }
    }

    QString message;
    inStream >> message;    // Читаем сообщение

    ChatHistory chatHistory {false, message};
    chatHistoryList.push_back(chatHistory);

    emit updateReceiveMessageNotification(this, message);   // Обновляем UI (выводим полученное сообщение, если текущий контакт выбран, иначе добавляем звездочку к имени контакта)
}

void Contact::readHistory(quint32 messageSize, QDataStream &inStream)
{
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

    emit updateChatArea(this);
}

bool Contact::isConnected() const
{
    bool res = false;
    if (tcpSocket != nullptr) {
        if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
            res = true;
        }
    }
    return res;
}
