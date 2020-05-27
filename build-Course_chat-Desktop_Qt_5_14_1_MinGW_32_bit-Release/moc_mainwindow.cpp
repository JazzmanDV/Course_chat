/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "C:/Users/danap/YandexDisk/Projects_Qt/Course_chat/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[21];
    char stringdata0[353];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 16), // "on_newConnection"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 24), // "on_addContactBtn_clicked"
QT_MOC_LITERAL(4, 54, 13), // "on_addContact"
QT_MOC_LITERAL(5, 68, 12), // "QHostAddress"
QT_MOC_LITERAL(6, 81, 8), // "serverIP"
QT_MOC_LITERAL(7, 90, 10), // "serverPort"
QT_MOC_LITERAL(8, 101, 24), // "on_delContactBtn_clicked"
QT_MOC_LITERAL(9, 126, 28), // "on_delAllContactsBtn_clicked"
QT_MOC_LITERAL(10, 155, 36), // "on_connectionList_currentItem..."
QT_MOC_LITERAL(11, 192, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(12, 209, 11), // "currentItem"
QT_MOC_LITERAL(13, 221, 25), // "on_sendMessageBtn_clicked"
QT_MOC_LITERAL(14, 247, 12), // "on_readyRead"
QT_MOC_LITERAL(15, 260, 8), // "Contact*"
QT_MOC_LITERAL(16, 269, 7), // "contact"
QT_MOC_LITERAL(17, 277, 17), // "on_tcpSocketError"
QT_MOC_LITERAL(18, 295, 3), // "row"
QT_MOC_LITERAL(19, 299, 24), // "on_disconnectBtn_clicked"
QT_MOC_LITERAL(20, 324, 28) // "on_historyRequestBtn_clicked"

    },
    "MainWindow\0on_newConnection\0\0"
    "on_addContactBtn_clicked\0on_addContact\0"
    "QHostAddress\0serverIP\0serverPort\0"
    "on_delContactBtn_clicked\0"
    "on_delAllContactsBtn_clicked\0"
    "on_connectionList_currentItemChanged\0"
    "QListWidgetItem*\0currentItem\0"
    "on_sendMessageBtn_clicked\0on_readyRead\0"
    "Contact*\0contact\0on_tcpSocketError\0"
    "row\0on_disconnectBtn_clicked\0"
    "on_historyRequestBtn_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    2,   71,    2, 0x08 /* Private */,
       8,    0,   76,    2, 0x08 /* Private */,
       9,    0,   77,    2, 0x08 /* Private */,
      10,    1,   78,    2, 0x08 /* Private */,
      13,    0,   81,    2, 0x08 /* Private */,
      14,    1,   82,    2, 0x08 /* Private */,
      17,    2,   85,    2, 0x08 /* Private */,
      19,    0,   90,    2, 0x08 /* Private */,
      20,    0,   91,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5, QMetaType::UShort,    6,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, 0x80000000 | 15, QMetaType::Int,   16,   18,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_newConnection(); break;
        case 1: _t->on_addContactBtn_clicked(); break;
        case 2: _t->on_addContact((*reinterpret_cast< QHostAddress(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 3: _t->on_delContactBtn_clicked(); break;
        case 4: _t->on_delAllContactsBtn_clicked(); break;
        case 5: _t->on_connectionList_currentItemChanged((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 6: _t->on_sendMessageBtn_clicked(); break;
        case 7: _t->on_readyRead((*reinterpret_cast< Contact*(*)>(_a[1]))); break;
        case 8: _t->on_tcpSocketError((*reinterpret_cast< Contact*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: _t->on_disconnectBtn_clicked(); break;
        case 10: _t->on_historyRequestBtn_clicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
