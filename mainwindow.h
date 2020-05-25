#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QDialog>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QTcpSocket>
#include <QListWidgetItem>
#include "createconnectionwindow.h"
#include "TcpConnection.h"

const QString PSEUDO_HASH = "tgj9834q8g5h8thgjjrtgfd8jg8yjut8ajg9";

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addContactBtn_clicked();
    void on_delContactBtn_clicked();

    void on_disconnected(Contact* contact, int row);

    void on_newConnection();
    void on_addContact(QString ip, quint16 port);
    void on_tcpSocketError(Contact* contact, int row);
    void on_sendMessageBtn_clicked();

    void on_delAllContactsBtn_clicked();

    void on_connectionList_currentItemChanged(QListWidgetItem *currentItem);

    void on_disconnectBtn_clicked();

    void on_historyRequestBtn_clicked();
    void on_readyRead(Contact* contact);

private:
    Ui::MainWindow *ui;

    void setupServer();
    void addContactToUI(QString socketString);
    void sendMessage(Contact* contact, QString message);
    void sendHistoryRequest(Contact* contact);
    void sendHistory(Contact* contact);
    void readMessage(Contact* contact, quint32 messageSize, QDataStream& inStream);
    void readHistory(Contact* contact, quint32 messageSize, QDataStream& inStream);

    void updateChatArea(QList<ChatHistory>& chatHistoryList);

    QTcpServer* mainTcpServer;
    QList<Contact*> contactList;
    QHostAddress mainServerIP;
    uint mainServerPort;
};
#endif // MAINWINDOW_H
