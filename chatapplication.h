#ifndef CHATAPPLICATION_H
#define CHATAPPLICATION_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include "Contact.h"

class ChatApplication
{
private:
    ChatApplication();
    ChatApplication(const ChatApplication&) = delete;
    ChatApplication& operator=(ChatApplication&) = delete;

    QTcpServer mainTcpServer;
    QList<Contact*> contactList;
public:
    static ChatApplication& getInstance();
    static int setMainTcpServer();
};

#endif // CHATAPPLICATION_H
