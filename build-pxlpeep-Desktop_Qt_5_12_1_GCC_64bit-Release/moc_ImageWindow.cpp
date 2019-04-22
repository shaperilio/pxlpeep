/****************************************************************************
** Meta object code from reading C++ file 'ImageWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ImageWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ImageWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ImageWindow_t {
    QByteArrayData data[16];
    char stringdata0[225];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ImageWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ImageWindow_t qt_meta_stringdata_ImageWindow = {
    {
QT_MOC_LITERAL(0, 0, 11), // "ImageWindow"
QT_MOC_LITERAL(1, 12, 16), // "signalWheelEvent"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 12), // "QWheelEvent*"
QT_MOC_LITERAL(4, 43, 5), // "event"
QT_MOC_LITERAL(5, 49, 8), // "windowID"
QT_MOC_LITERAL(6, 58, 21), // "signalMousePressEvent"
QT_MOC_LITERAL(7, 80, 12), // "QMouseEvent*"
QT_MOC_LITERAL(8, 93, 20), // "signalMouseMoveEvent"
QT_MOC_LITERAL(9, 114, 23), // "signalMouseReleaseEvent"
QT_MOC_LITERAL(10, 138, 16), // "signalKeyPressed"
QT_MOC_LITERAL(11, 155, 10), // "QKeyEvent*"
QT_MOC_LITERAL(12, 166, 17), // "signalKeyReleased"
QT_MOC_LITERAL(13, 184, 17), // "signalFileDeleted"
QT_MOC_LITERAL(14, 202, 8), // "QString&"
QT_MOC_LITERAL(15, 211, 13) // "trashLocation"

    },
    "ImageWindow\0signalWheelEvent\0\0"
    "QWheelEvent*\0event\0windowID\0"
    "signalMousePressEvent\0QMouseEvent*\0"
    "signalMouseMoveEvent\0signalMouseReleaseEvent\0"
    "signalKeyPressed\0QKeyEvent*\0"
    "signalKeyReleased\0signalFileDeleted\0"
    "QString&\0trashLocation"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ImageWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   49,    2, 0x06 /* Public */,
       6,    2,   54,    2, 0x06 /* Public */,
       8,    2,   59,    2, 0x06 /* Public */,
       9,    2,   64,    2, 0x06 /* Public */,
      10,    2,   69,    2, 0x06 /* Public */,
      12,    2,   74,    2, 0x06 /* Public */,
      13,    1,   79,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 7, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 7, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 7, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 11, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 11, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 14,   15,

       0        // eod
};

void ImageWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ImageWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signalWheelEvent((*reinterpret_cast< QWheelEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->signalMousePressEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->signalMouseMoveEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->signalMouseReleaseEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->signalKeyPressed((*reinterpret_cast< QKeyEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->signalKeyReleased((*reinterpret_cast< QKeyEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->signalFileDeleted((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ImageWindow::*)(QWheelEvent * , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ImageWindow::signalWheelEvent)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ImageWindow::*)(QMouseEvent * , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ImageWindow::signalMousePressEvent)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ImageWindow::*)(QMouseEvent * , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ImageWindow::signalMouseMoveEvent)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ImageWindow::*)(QMouseEvent * , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ImageWindow::signalMouseReleaseEvent)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ImageWindow::*)(QKeyEvent * , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ImageWindow::signalKeyPressed)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ImageWindow::*)(QKeyEvent * , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ImageWindow::signalKeyReleased)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (ImageWindow::*)(QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ImageWindow::signalFileDeleted)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ImageWindow::staticMetaObject = { {
    &QGraphicsView::staticMetaObject,
    qt_meta_stringdata_ImageWindow.data,
    qt_meta_data_ImageWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ImageWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ImageWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ImageWindow.stringdata0))
        return static_cast<void*>(this);
    return QGraphicsView::qt_metacast(_clname);
}

int ImageWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void ImageWindow::signalWheelEvent(QWheelEvent * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ImageWindow::signalMousePressEvent(QMouseEvent * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ImageWindow::signalMouseMoveEvent(QMouseEvent * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ImageWindow::signalMouseReleaseEvent(QMouseEvent * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ImageWindow::signalKeyPressed(QKeyEvent * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void ImageWindow::signalKeyReleased(QKeyEvent * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void ImageWindow::signalFileDeleted(QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
