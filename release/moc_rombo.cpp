/****************************************************************************
** Meta object code from reading C++ file 'rombo.h'
**
** Created:
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../rombo.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rombo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,
      41,   11,   11,   11, 0x05,
      58,   11,   11,   11, 0x05,
      86,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
     111,   11,   11,   11, 0x08,
     121,   11,   11,   11, 0x08,
     133,   11,  128,   11, 0x08,
     140,   11,  128,   11, 0x08,
     149,   11,   11,   11, 0x08,
     162,   11,   11,   11, 0x08,
     175,   11,   11,   11, 0x08,
     188,   11,   11,   11, 0x08,
     202,   11,   11,   11, 0x08,
     225,   11,   11,   11, 0x08,
     233,   11,   11,   11, 0x08,
     263,  255,   11,   11, 0x08,
     287,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0sceneFileLoaded(std::string)\0"
    "sceneUnloading()\0renderVerbosityChanged(int)\0"
    "renderStatusChanged(int)\0newFile()\0"
    "open()\0bool\0save()\0saveAs()\0closeScene()\0"
    "renderPlay()\0renderStop()\0renderPause()\0"
    "renderVerbose(QString)\0about()\0"
    "documentWasModified()\0iStatus\0"
    "setRenderStatusBar(int)\0openRecentFile()\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->sceneFileLoaded((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 1: _t->sceneUnloading(); break;
        case 2: _t->renderVerbosityChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->renderStatusChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->newFile(); break;
        case 5: _t->open(); break;
        case 6: { bool _r = _t->save();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 7: { bool _r = _t->saveAs();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 8: _t->closeScene(); break;
        case 9: _t->renderPlay(); break;
        case 10: _t->renderStop(); break;
        case 11: _t->renderPause(); break;
        case 12: _t->renderVerbose((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 13: _t->about(); break;
        case 14: _t->documentWasModified(); break;
        case 15: _t->setRenderStatusBar((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->openRecentFile(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::sceneFileLoaded(std::string _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MainWindow::sceneUnloading()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void MainWindow::renderVerbosityChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MainWindow::renderStatusChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
