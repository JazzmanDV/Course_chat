#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QDialog>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QTcpSocket>
#include <QTableWidgetItem>
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
    void on_addNewConnectionBtn_clicked();
    void on_newConnection();
    void on_addNewConnection(QString ip, quint16 port, Contact* contact = nullptr);
    void on_tcpSocketError(QAbstractSocket::SocketError socketError);
    void on_connectionList_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void on_sendBtn_clicked();

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
