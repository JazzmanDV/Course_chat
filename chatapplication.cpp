#include "chatapplication.h"

ChatApplication::ChatApplication() {
}

ChatApplication& ChatApplication::getInstance() {
    static ChatApplication instance;
    return instance;
}

QTcpServer& ChatApplication::getMainTcpServer() {
    return mainTcpServer;
}

QList<Contact *>& ChatApplication::getContactList() {
    return contactList;
}
