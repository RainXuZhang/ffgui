/****************************************************************************
** Meta object code from reading C++ file 'ProjectBinWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/widgets/ProjectBinWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ProjectBinWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN16ProjectBinWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto ProjectBinWidget::qt_create_metaobjectdata<qt_meta_tag_ZN16ProjectBinWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ProjectBinWidget",
        "clipAdded",
        "",
        "MediaClip",
        "clip",
        "clipSelected",
        "clipDoubleClicked",
        "clipRemoved",
        "id",
        "onAddClipClicked",
        "onRemoveClipClicked",
        "onItemDoubleClicked",
        "QListWidgetItem*",
        "item",
        "onItemSelectionChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'clipAdded'
        QtMocHelpers::SignalData<void(const MediaClip &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'clipSelected'
        QtMocHelpers::SignalData<void(const MediaClip &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'clipDoubleClicked'
        QtMocHelpers::SignalData<void(const MediaClip &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'clipRemoved'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Slot 'onAddClipClicked'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRemoveClipClicked'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onItemDoubleClicked'
        QtMocHelpers::SlotData<void(QListWidgetItem *)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Slot 'onItemSelectionChanged'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ProjectBinWidget, qt_meta_tag_ZN16ProjectBinWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ProjectBinWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ProjectBinWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ProjectBinWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16ProjectBinWidgetE_t>.metaTypes,
    nullptr
} };

void ProjectBinWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ProjectBinWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->clipAdded((*reinterpret_cast<std::add_pointer_t<MediaClip>>(_a[1]))); break;
        case 1: _t->clipSelected((*reinterpret_cast<std::add_pointer_t<MediaClip>>(_a[1]))); break;
        case 2: _t->clipDoubleClicked((*reinterpret_cast<std::add_pointer_t<MediaClip>>(_a[1]))); break;
        case 3: _t->clipRemoved((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->onAddClipClicked(); break;
        case 5: _t->onRemoveClipClicked(); break;
        case 6: _t->onItemDoubleClicked((*reinterpret_cast<std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 7: _t->onItemSelectionChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ProjectBinWidget::*)(const MediaClip & )>(_a, &ProjectBinWidget::clipAdded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ProjectBinWidget::*)(const MediaClip & )>(_a, &ProjectBinWidget::clipSelected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ProjectBinWidget::*)(const MediaClip & )>(_a, &ProjectBinWidget::clipDoubleClicked, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ProjectBinWidget::*)(const QString & )>(_a, &ProjectBinWidget::clipRemoved, 3))
            return;
    }
}

const QMetaObject *ProjectBinWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProjectBinWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ProjectBinWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ProjectBinWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void ProjectBinWidget::clipAdded(const MediaClip & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ProjectBinWidget::clipSelected(const MediaClip & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ProjectBinWidget::clipDoubleClicked(const MediaClip & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void ProjectBinWidget::clipRemoved(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
