#ifndef CONTACT_H
#define CONTACT_H

#include <QString>
#include <QTcpSocket>
#include <QList>
#include <QHostAddress>

struct ChatHistory {
    bool isYourMessage;
    QString message;
};

class Contact {
private:
    QTcpSocket* tcpSocket;
    QHostAddress serverIP;
    quint16 serverPort;
    QString socketString;
    QList<ChatHistory> chatHistoryList;

    quint32 readMessageSize(QDataStream& inStream);
    int readDataType(QDataStream& inStream);
    void readMessage(quint32 messageSize, QDataStream& inStream);
    void readHistory(quint32 messageSize, QDataStream& inStream);
    void sendHistory();

public:
    Contact();
    Contact(QHostAddress serverIP, quint16 serverPort, QString socketString);
    ~Contact();

    void setTcpSocket(QTcpSocket* socket);

    QTcpSocket* getTcpSocket();

    QHostAddress getServerIP() const;
    void setServerIP(const QHostAddress &serverIP);

    quint16 getServerPort() const;
    void setServerPort(const quint16 &serverPort);

    QString getSocketString() const;
    void setSocketString(const QString &socketString);

    QList<ChatHistory> getChatHistoryList() const;
    void setChatHistoryList(const QList<ChatHistory> &chatHistoryList);
    void addChatHistory(ChatHistory chatHistory);

    void disconnectFromHost();
    void deleteTcpSocket();
    void sendMessage(QString message);
    void sendHistoryRequest();

    void readData();

    bool isConnected() const;

signals:
    void updateReceiveMessageNotification(Contact* contact, QString message);
    void updateChatArea(Contact* contact);
};

#endif // CONTACT_H
