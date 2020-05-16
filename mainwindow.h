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

    void on_newConnection();
    void on_addContact(QString ip, quint16 port, Contact* contact = nullptr);
    void on_tcpSocketError(QAbstractSocket::SocketError socketError);
    void on_sendBtn_clicked();

    void on_delAllContactsBtn_clicked();

    void on_connectionList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::MainWindow *ui;
    void setupServer();
    void addContactToUI(QString ip, QString port);
    void sendMessage(Contact* contact, QString message);
    void readMessage(Contact* contact);

    QTcpServer *mainTcpServer;
    QList<Contact*> contactList;
    QHostAddress mainServerIP;
    uint mainServerPort;
    quint16 blockSize;
};
#endif // MAINWINDOW_H
