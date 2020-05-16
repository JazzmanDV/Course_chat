#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <QString>
#include <QTcpSocket>
#include <QList>
#include <QHostAddress>

struct ChatHistory {
    bool isYourMessage;
    QString message;
};

struct Contact {
    QTcpSocket* tcpSocket;
    QHostAddress serverIP;
    quint16 serverPort;
    QList<ChatHistory> chatHistoryList;
};

#endif // TCPCONNECTION_H


