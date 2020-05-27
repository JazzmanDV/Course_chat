/********************************************************************************
** Form generated from reading UI file 'createconnectionwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATECONNECTIONWINDOW_H
#define UI_CREATECONNECTIONWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_CreateConnectionWindow
{
public:
    QDialogButtonBox *okCancelBtn;
    QLineEdit *IPLine;
    QLineEdit *PortLine;
    QLabel *enterIPLabel;
    QLabel *enterPortLabel;

    void setupUi(QDialog *CreateConnectionWindow)
    {
        if (CreateConnectionWindow->objectName().isEmpty())
            CreateConnectionWindow->setObjectName(QString::fromUtf8("CreateConnectionWindow"));
        CreateConnectionWindow->resize(258, 91);
        okCancelBtn = new QDialogButtonBox(CreateConnectionWindow);
        okCancelBtn->setObjectName(QString::fromUtf8("okCancelBtn"));
        okCancelBtn->setGeometry(QRect(-90, 60, 341, 32));
        okCancelBtn->setOrientation(Qt::Horizontal);
        okCancelBtn->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        IPLine = new QLineEdit(CreateConnectionWindow);
        IPLine->setObjectName(QString::fromUtf8("IPLine"));
        IPLine->setGeometry(QRect(10, 30, 161, 20));
        PortLine = new QLineEdit(CreateConnectionWindow);
        PortLine->setObjectName(QString::fromUtf8("PortLine"));
        PortLine->setGeometry(QRect(180, 30, 71, 20));
        enterIPLabel = new QLabel(CreateConnectionWindow);
        enterIPLabel->setObjectName(QString::fromUtf8("enterIPLabel"));
        enterIPLabel->setGeometry(QRect(10, 10, 61, 16));
        enterPortLabel = new QLabel(CreateConnectionWindow);
        enterPortLabel->setObjectName(QString::fromUtf8("enterPortLabel"));
        enterPortLabel->setGeometry(QRect(180, 10, 71, 16));

        retranslateUi(CreateConnectionWindow);

        QMetaObject::connectSlotsByName(CreateConnectionWindow);
    } // setupUi

    void retranslateUi(QDialog *CreateConnectionWindow)
    {
        CreateConnectionWindow->setWindowTitle(QCoreApplication::translate("CreateConnectionWindow", "Dialog", nullptr));
        IPLine->setInputMask(QCoreApplication::translate("CreateConnectionWindow", "000.000.000.000;_", nullptr));
        PortLine->setInputMask(QCoreApplication::translate("CreateConnectionWindow", "00000;_", nullptr));
        enterIPLabel->setText(QCoreApplication::translate("CreateConnectionWindow", "\320\222\320\262\320\265\320\264\320\270\321\202\320\265 IP", nullptr));
        enterPortLabel->setText(QCoreApplication::translate("CreateConnectionWindow", "\320\222\320\262\320\265\320\264\320\270\321\202\320\265 \320\277\320\276\321\200\321\202", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CreateConnectionWindow: public Ui_CreateConnectionWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATECONNECTIONWINDOW_H
