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
    QByteArrayData data[21];
    char stringdata0[355];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainDialog_t qt_meta_stringdata_MainDialog = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainDialog"
QT_MOC_LITERAL(1, 11, 19), // "on_btnClose_clicked"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 21), // "slot_btnImage_clicked"
QT_MOC_LITERAL(4, 54, 30), // "on_chkSyncWindows_stateChanged"
QT_MOC_LITERAL(5, 85, 4), // "arg1"
QT_MOC_LITERAL(6, 90, 23), // "wheelEventInImageWindow"
QT_MOC_LITERAL(7, 114, 12), // "QWheelEvent*"
QT_MOC_LITERAL(8, 127, 5), // "event"
QT_MOC_LITERAL(9, 133, 8), // "windowID"
QT_MOC_LITERAL(10, 142, 28), // "mousePressEventInImageWindow"
QT_MOC_LITERAL(11, 171, 12), // "QMouseEvent*"
QT_MOC_LITERAL(12, 184, 27), // "mouseMoveEventInImageWindow"
QT_MOC_LITERAL(13, 212, 30), // "mouseReleaseEventInImageWindow"
QT_MOC_LITERAL(14, 243, 23), // "keyPressedInImageWindow"
QT_MOC_LITERAL(15, 267, 10), // "QKeyEvent*"
QT_MOC_LITERAL(16, 278, 24), // "keyReleasedInImageWindow"
QT_MOC_LITERAL(17, 303, 4), // "exec"
QT_MOC_LITERAL(18, 308, 23), // "imageWindowDeletedAFile"
QT_MOC_LITERAL(19, 332, 8), // "QString&"
QT_MOC_LITERAL(20, 341, 13) // "trashLocation"

    },
    "MainDialog\0on_btnClose_clicked\0\0"
    "slot_btnImage_clicked\0"
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
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    1,   71,    2, 0x08 /* Private */,
       6,    2,   74,    2, 0x08 /* Private */,
      10,    2,   79,    2, 0x08 /* Private */,
      12,    2,   84,    2, 0x08 /* Private */,
      13,    2,   89,    2, 0x08 /* Private */,
      14,    2,   94,    2, 0x08 /* Private */,
      16,    2,   99,    2, 0x08 /* Private */,
      17,    0,  104,    2, 0x0a /* Public */,
      18,    1,  105,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, 0x80000000 | 7, QMetaType::Int,    8,    9,
    QMetaType::Void, 0x80000000 | 11, QMetaType::Int,    8,    9,
    QMetaType::Void, 0x80000000 | 11, QMetaType::Int,    8,    9,
    QMetaType::Void, 0x80000000 | 11, QMetaType::Int,    8,    9,
    QMetaType::Void, 0x80000000 | 15, QMetaType::Int,    8,    9,
    QMetaType::Void, 0x80000000 | 15, QMetaType::Int,    8,    9,
    QMetaType::Int,
    QMetaType::Void, 0x80000000 | 19,   20,

       0        // eod
};

void MainDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_btnClose_clicked(); break;
        case 1: _t->slot_btnImage_clicked(); break;
        case 2: _t->on_chkSyncWindows_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->wheelEventInImageWindow((*reinterpret_cast< QWheelEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->mousePressEventInImageWindow((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->mouseMoveEventInImageWindow((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->mouseReleaseEventInImageWindow((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->keyPressedInImageWindow((*reinterpret_cast< QKeyEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->keyReleasedInImageWindow((*reinterpret_cast< QKeyEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: { int _r = _t->exec();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 10: _t->imageWindowDeletedAFile((*reinterpret_cast< QString(*)>(_a[1]))); break;
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
