/****************************************************************************
** Meta object code from reading C++ file 'scanworker.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../scanworker.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scanworker.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ScanWorker_t {
    QByteArrayData data[9];
    char stringdata0[95];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ScanWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ScanWorker_t qt_meta_stringdata_ScanWorker = {
    {
QT_MOC_LITERAL(0, 0, 10), // "ScanWorker"
QT_MOC_LITERAL(1, 11, 12), // "cardDetected"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 20), // "CardReader::CardData"
QT_MOC_LITERAL(4, 46, 8), // "cardData"
QT_MOC_LITERAL(5, 55, 12), // "scanProgress"
QT_MOC_LITERAL(6, 68, 7), // "message"
QT_MOC_LITERAL(7, 76, 13), // "startScanning"
QT_MOC_LITERAL(8, 90, 4) // "stop"

    },
    "ScanWorker\0cardDetected\0\0CardReader::CardData\0"
    "cardData\0scanProgress\0message\0"
    "startScanning\0stop"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ScanWorker[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,
       5,    1,   37,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   40,    2, 0x0a /* Public */,
       8,    0,   41,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ScanWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ScanWorker *_t = static_cast<ScanWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->cardDetected((*reinterpret_cast< CardReader::CardData(*)>(_a[1]))); break;
        case 1: _t->scanProgress((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->startScanning(); break;
        case 3: _t->stop(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (ScanWorker::*_t)(CardReader::CardData );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ScanWorker::cardDetected)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (ScanWorker::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ScanWorker::scanProgress)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject ScanWorker::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ScanWorker.data,
      qt_meta_data_ScanWorker,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ScanWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScanWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ScanWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ScanWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ScanWorker::cardDetected(CardReader::CardData _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ScanWorker::scanProgress(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
