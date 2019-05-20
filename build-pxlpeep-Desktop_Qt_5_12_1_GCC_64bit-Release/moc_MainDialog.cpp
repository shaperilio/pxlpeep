/****************************************************************************
** Meta object code from reading C++ file 'MainDialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../MainDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainDialog_t {
    QByteArrayData data[22];
    char stringdata0[377];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainDialog_t qt_meta_stringdata_MainDialog = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainDialog"
QT_MOC_LITERAL(1, 11, 18), // "on_btnExit_clicked"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 22), // "on_btnCloseAll_clicked"
QT_MOC_LITERAL(4, 54, 21), // "slot_btnImage_clicked"
QT_MOC_LITERAL(5, 76, 30), // "on_chkSyncWindows_stateChanged"
QT_MOC_LITERAL(6, 107, 4), // "arg1"
QT_MOC_LITERAL(7, 112, 23), // "wheelEventInImageWindow"
QT_MOC_LITERAL(8, 136, 12), // "QWheelEvent*"
QT_MOC_LITERAL(9, 149, 5), // "event"
QT_MOC_LITERAL(10, 155, 8), // "windowID"
QT_MOC_LITERAL(11, 164, 28), // "mousePressEventInImageWindow"
QT_MOC_LITERAL(12, 193, 12), // "QMouseEvent*"
QT_MOC_LITERAL(13, 206, 27), // "mouseMoveEventInImageWindow"
QT_MOC_LITERAL(14, 234, 30), // "mouseReleaseEventInImageWindow"
QT_MOC_LITERAL(15, 265, 23), // "keyPressedInImageWindow"
QT_MOC_LITERAL(16, 289, 10), // "QKeyEvent*"
QT_MOC_LITERAL(17, 300, 24), // "keyReleasedInImageWindow"
QT_MOC_LITERAL(18, 325, 4), // "exec"
QT_MOC_LITERAL(19, 330, 23), // "imageWindowDeletedAFile"
QT_MOC_LITERAL(20, 354, 8), // "QString&"
QT_MOC_LITERAL(21, 363, 13) // "trashLocation"

    },
    "MainDialog\0on_btnExit_clicked\0\0"
    "on_btnCloseAll_clicked\0slot_btnImage_clicked\0"
    "on_chkSyncWindows_stateChanged\0arg1\0"
    "wheelEventInImageWindow\0QWheelEvent*\0"
    "event\0windowID\0mousePressEventInImageWindow\0"
    "QMouseEvent*\0mouseMoveEventInImageWindow\0"
    "mouseReleaseEventInImageWindow\0"
    "keyPressedInImageWindow\0QKeyEvent*\0"
    "keyReleasedInImageWindow\0exec\0"
    "imageWindowDeletedAFile\0QString&\0"
    "trashLocation"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x08 /* Private */,
       3,    0,   75,    2, 0x08 /* Private */,
       4,    0,   76,    2, 0x08 /* Private */,
       5,    1,   77,    2, 0x08 /* Private */,
       7,    2,   80,    2, 0x08 /* Private */,
      11,    2,   85,    2, 0x08 /* Private */,
      13,    2,   90,    2, 0x08 /* Private */,
      14,    2,   95,    2, 0x08 /* Private */,
      15,    2,  100,    2, 0x08 /* Private */,
      17,    2,  105,    2, 0x08 /* Private */,
      18,    0,  110,    2, 0x0a /* Public */,
      19,    1,  111,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,    9,   10,
    QMetaType::Void, 0x80000000 | 12, QMetaType::Int,    9,   10,
    QMetaType::Void, 0x80000000 | 12, QMetaType::Int,    9,   10,
    QMetaType::Void, 0x80000000 | 12, QMetaType::Int,    9,   10,
    QMetaType::Void, 0x80000000 | 16, QMetaType::Int,    9,   10,
    QMetaType::Void, 0x80000000 | 16, QMetaType::Int,    9,   10,
    QMetaType::Int,
    QMetaType::Void, 0x80000000 | 20,   21,

       0        // eod
};

void MainDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_btnExit_clicked(); break;
        case 1: _t->on_btnCloseAll_clicked(); break;
        case 2: _t->slot_btnImage_clicked(); break;
        case 3: _t->on_chkSyncWindows_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->wheelEventInImageWindow((*reinterpret_cast< QWheelEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->mousePressEventInImageWindow((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->mouseMoveEventInImageWindow((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->mouseReleaseEventInImageWindow((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->keyPressedInImageWindow((*reinterpret_cast< QKeyEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: _t->keyReleasedInImageWindow((*reinterpret_cast< QKeyEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 10: { int _r = _t->exec();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 11: _t->imageWindowDeletedAFile((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainDialog::staticMetaObject = { {
    &QDialog::staticMetaObject,
    qt_meta_stringdata_MainDialog.data,
    qt_meta_data_MainDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int MainDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
