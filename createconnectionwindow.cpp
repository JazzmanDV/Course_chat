#include "createconnectionwindow.h"
#include "ui_createconnectionwindow.h"

CreateConnectionWindow::CreateConnectionWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateConnectionWindow)
{
    ui->setupUi(this);
}

CreateConnectionWindow::~CreateConnectionWindow()
{
    delete ui;
}

void CreateConnectionWindow::on_okCancelBtn_accepted()
{
    emit addNewConnection(this->ui->IPLine->text(), this->ui->PortLine->text().toUInt());
    this->close();
}

void CreateConnectionWindow::on_okCancelBtn_rejected()
{
    this->~CreateConnectionWindow();
}
