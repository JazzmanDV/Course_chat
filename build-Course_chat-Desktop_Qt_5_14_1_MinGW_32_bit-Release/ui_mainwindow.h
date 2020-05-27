/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *addContactBtn;
    QPushButton *delContactBtn;
    QPushButton *delAllContactsBtn;
    QPushButton *sendMessageBtn;
    QLabel *yourIPLabel;
    QPlainTextEdit *inputArea;
    QPlainTextEdit *chatArea;
    QListWidget *connectionList;
    QPushButton *disconnectBtn;
    QPushButton *historyRequestBtn;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(780, 480);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(780, 480));
        MainWindow->setMaximumSize(QSize(780, 480));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        addContactBtn = new QPushButton(centralwidget);
        addContactBtn->setObjectName(QString::fromUtf8("addContactBtn"));
        addContactBtn->setGeometry(QRect(10, 430, 71, 40));
        addContactBtn->setFlat(false);
        delContactBtn = new QPushButton(centralwidget);
        delContactBtn->setObjectName(QString::fromUtf8("delContactBtn"));
        delContactBtn->setGeometry(QRect(90, 430, 81, 40));
        delAllContactsBtn = new QPushButton(centralwidget);
        delAllContactsBtn->setObjectName(QString::fromUtf8("delAllContactsBtn"));
        delAllContactsBtn->setGeometry(QRect(180, 430, 71, 40));
        sendMessageBtn = new QPushButton(centralwidget);
        sendMessageBtn->setObjectName(QString::fromUtf8("sendMessageBtn"));
        sendMessageBtn->setGeometry(QRect(640, 430, 131, 40));
        sendMessageBtn->setMinimumSize(QSize(85, 40));
        yourIPLabel = new QLabel(centralwidget);
        yourIPLabel->setObjectName(QString::fromUtf8("yourIPLabel"));
        yourIPLabel->setGeometry(QRect(10, 10, 161, 16));
        yourIPLabel->setScaledContents(false);
        inputArea = new QPlainTextEdit(centralwidget);
        inputArea->setObjectName(QString::fromUtf8("inputArea"));
        inputArea->setGeometry(QRect(420, 430, 211, 41));
        chatArea = new QPlainTextEdit(centralwidget);
        chatArea->setObjectName(QString::fromUtf8("chatArea"));
        chatArea->setEnabled(true);
        chatArea->setGeometry(QRect(220, 30, 551, 391));
        chatArea->setReadOnly(true);
        connectionList = new QListWidget(centralwidget);
        connectionList->setObjectName(QString::fromUtf8("connectionList"));
        connectionList->setGeometry(QRect(10, 30, 201, 391));
        disconnectBtn = new QPushButton(centralwidget);
        disconnectBtn->setObjectName(QString::fromUtf8("disconnectBtn"));
        disconnectBtn->setGeometry(QRect(260, 430, 71, 40));
        historyRequestBtn = new QPushButton(centralwidget);
        historyRequestBtn->setObjectName(QString::fromUtf8("historyRequestBtn"));
        historyRequestBtn->setGeometry(QRect(340, 430, 71, 40));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        addContactBtn->setText(QCoreApplication::translate("MainWindow", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214\n"
"\320\272\320\276\320\275\321\202\320\260\320\272\321\202", nullptr));
        delContactBtn->setText(QCoreApplication::translate("MainWindow", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214\n"
"\320\272\320\276\320\275\321\202\320\260\320\272\321\202", nullptr));
        delAllContactsBtn->setText(QCoreApplication::translate("MainWindow", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214 \320\262\321\201\320\265\n"
"\320\272\320\276\320\275\321\202\320\260\320\272\321\202\321\213", nullptr));
        sendMessageBtn->setText(QCoreApplication::translate("MainWindow", "\320\236\321\202\320\277\321\200\320\260\320\262\320\270\321\202\321\214 \321\201\320\276\320\276\320\261\321\211\320\265\320\275\320\270\320\265", nullptr));
        yourIPLabel->setText(QString());
        disconnectBtn->setText(QCoreApplication::translate("MainWindow", "\320\240\320\260\320\267\320\276\321\200\320\262\320\260\321\202\321\214\n"
"\321\201\320\276\320\265\320\264\320\270\320\275\320\265\320\275\320\270\320\265", nullptr));
        historyRequestBtn->setText(QCoreApplication::translate("MainWindow", "\320\227\320\260\320\277\321\200\320\276\321\201\320\270\321\202\321\214\n"
"\320\270\321\201\321\202\320\276\321\200\320\270\321\216", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
