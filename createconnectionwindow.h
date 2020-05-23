#ifndef CREATECONNECTIONWINDOW_H
#define CREATECONNECTIONWINDOW_H

#include <QDialog>
#include "TcpConnection.h"

namespace Ui {
class CreateConnectionWindow;
}

class CreateConnectionWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CreateConnectionWindow(QWidget *parent = nullptr);
    ~CreateConnectionWindow();

private slots:
    void on_okCancelBtn_accepted();
    void on_okCancelBtn_rejected();

signals:
    void addContact(QString ip, quint16 port);

private:
    Ui::CreateConnectionWindow *ui;
};

#endif // CREATECONNECTIONWINDOW_H
