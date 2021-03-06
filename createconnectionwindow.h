#ifndef CREATECONNECTIONWINDOW_H
#define CREATECONNECTIONWINDOW_H

#include <QDialog>
//#include "Contact.h"
#include <QHostAddress>

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
    void addContact(QHostAddress serverIP, quint16 serverPort);

private:
    Ui::CreateConnectionWindow *ui;
};

#endif // CREATECONNECTIONWINDOW_H
