// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQMLDMABSTRACTITEMMODELDATA_P_H
#define QQMLDMABSTRACTITEMMODELDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qqmladaptormodelenginedata_p.h>
#include <private/qqmldelegatemodel_p_p.h>

QT_BEGIN_NAMESPACE

class VDMAbstractItemModelDataType;
class QQmlDMAbstractItemModelData : public QQmlDelegateModelItem
{
    Q_OBJECT
    Q_PROPERTY(bool hasModelChildren READ hasModelChildren CONSTANT)

public:
    QQmlDMAbstractItemModelData(
            const QQmlRefPointer<QQmlDelegateModelItemMetaType> &metaType,
            VDMAbstractItemModelDataType *dataType,
            int index, int row, int column);

    int metaCall(QMetaObject::Call call, int id, void **arguments);
    bool hasModelChildren() const;

    QV4::ReturnedValue get() override;
    void setValue(const QString &role, const QVariant &value) override;
    bool resolveIndex(const QQmlAdaptorModel &model, int idx) override;

    static QV4::ReturnedValue get_property(const QV4::FunctionObject *, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue set_property(const QV4::FunctionObject *, const QV4::Value *thisObject, const QV4::Value *argv, int argc);

    const VDMAbstractItemModelDataType *type() const { return m_type; }

private:
    QVariant value(int role) const;
    void setValue(int role, const QVariant &value);

    VDMAbstractItemModelDataType *m_type;
    QVector<QVariant> m_cachedData;
};

class VDMAbstractItemModelDataType
        : public QQmlRefCount
        , public QQmlAdaptorModel::Accessors
        , public QAbstractDynamicMetaObject
{
public:
    VDMAbstractItemModelDataType(QQmlAdaptorModel *model)
        : model(model)
        , propertyOffset(0)
        , signalOffset(0)
        , hasModelData(false)
    {
    }

    bool notify(
            const QQmlAdaptorModel &,
            const QList<QQmlDelegateModelItem *> &items,
            int index,
            int count,
            const QVector<int> &roles) const override
    {
        bool changed = roles.isEmpty() && !watchedRoles.isEmpty();
        if (!changed && !watchedRoles.isEmpty() && watchedRoleIds.isEmpty()) {
            QList<int> roleIds;
            for (const QByteArray &r : watchedRoles) {
                QHash<QByteArray, int>::const_iterator it = roleNames.find(r);
                if (it != roleNames.end())
                    roleIds << it.value();
            }
            const_cast<VDMAbstractItemModelDataType *>(this)->watchedRoleIds = roleIds;
        }

        QVector<int> signalIndexes;
        for (int i = 0; i < roles.size(); ++i) {
            const int role = roles.at(i);
            if (!changed && watchedRoleIds.contains(role))
                changed = true;

            int propertyId = propertyRoles.indexOf(role);
            if (propertyId != -1)
                signalIndexes.append(propertyId + signalOffset);
        }
        if (roles.isEmpty()) {
            const int propertyRolesCount = propertyRoles.size();
            signalIndexes.reserve(propertyRolesCount);
            for (int propertyId = 0; propertyId < propertyRolesCount; ++propertyId)
                signalIndexes.append(propertyId + signalOffset);
        }

        QVarLengthArray<QQmlGuard<QQmlDelegateModelItem>> guardedItems;
        for (const auto item : items)
            guardedItems.append(item);

        for (const auto &item : std::as_const(guardedItems)) {
            if (item.isNull())
                continue;

            const int idx = item->modelIndex();
            if (idx >= index && idx < index + count) {
                for (int i = 0; i < signalIndexes.size(); ++i)
                    QMetaObject::activate(item, signalIndexes.at(i), nullptr);
            }
        }
        return changed;
    }

    void replaceWatchedRoles(
            QQmlAdaptorModel &,
            const QList<QByteArray> &oldRoles,
            const QList<QByteArray> &newRoles) const override
    {
        VDMAbstractItemModelDataType *dataType = const_cast<VDMAbstractItemModelDataType *>(this);

        dataType->watchedRoleIds.clear();
        for (const QByteArray &oldRole : oldRoles)
            dataType->watchedRoles.removeOne(oldRole);
        dataType->watchedRoles += newRoles;
    }

    static QV4::ReturnedValue get_hasModelChildren(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
    {
        QV4::Scope scope(b);
        QV4::Scoped<QQmlDelegateModelItemObject> o(scope, thisObject->as<QQmlDelegateModelItemObject>());
        if (!o)
            RETURN_RESULT(scope.engine->throwTypeError(QStringLiteral("Not a valid DelegateModel object")));

        const QQmlAdaptorModel *const model
                = static_cast<QQmlDMAbstractItemModelData *>(o->d()->item)->type()->model;
        if (o->d()->item->index >= 0) {
            if (const QAbstractItemModel *const aim = model->aim())
                RETURN_RESULT(QV4::Encode(aim->hasChildren(aim->index(o->d()->item->index, 0, model->rootIndex))));
        }
        RETURN_RESULT(QV4::Encode(false));
    }


    void initializeConstructor(QQmlAdaptorModelEngineData *const data)
    {
        QV4::ExecutionEngine *v4 = data->v4;
        QV4::Scope scope(v4);
        QV4::ScopedObject proto(scope, v4->newObject());
        proto->defineAccessorProperty(QStringLiteral("index"), QQmlAdaptorModelEngineData::get_index, nullptr);
        proto->defineAccessorProperty(QStringLiteral("hasModelChildren"), get_hasModelChildren, nullptr);
        QV4::ScopedProperty p(scope);

        typedef QHash<QByteArray, int>::const_iterator iterator;
        for (iterator it = roleNames.constBegin(), end = roleNames.constEnd(); it != end; ++it) {
            const qsizetype propertyId = propertyRoles.indexOf(it.value());
            const QByteArray &propertyName = it.key();

            QV4::ScopedString name(scope, v4->newString(QString::fromUtf8(propertyName)));
            QV4::ExecutionContext *global = v4->rootContext();
            QV4::ScopedFunctionObject g(scope, v4->memoryManager->allocate<QV4::IndexedBuiltinFunction>(global, propertyId, QQmlDMAbstractItemModelData::get_property));
            QV4::ScopedFunctionObject s(scope, v4->memoryManager->allocate<QV4::IndexedBuiltinFunction>(global, propertyId, QQmlDMAbstractItemModelData::set_property));
            p->setGetter(g);
            p->setSetter(s);
            proto->insertMember(name, p, QV4::Attr_Accessor|QV4::Attr_NotEnumerable|QV4::Attr_NotConfigurable);
        }
        prototype.set(v4, proto);
    }

    // QAbstractDynamicMetaObject

    void objectDestroyed(QObject *) override
    {
        release();
    }

    int metaCall(QObject *object, QMetaObject::Call call, int id, void **arguments) override
    {
        return static_cast<QQmlDMAbstractItemModelData *>(object)->metaCall(call, id, arguments);
    }

    int rowCount(const QQmlAdaptorModel &model) const override
    {
        if (const QAbstractItemModel *aim = model.aim())
            return aim->rowCount(model.rootIndex);
        return 0;
    }

    int columnCount(const QQmlAdaptorModel &model) const override
    {
        if (const QAbstractItemModel *aim = model.aim())
            return aim->columnCount(model.rootIndex);
        return 0;
    }

    void cleanup(QQmlAdaptorModel &) const override
    {
        release();
    }

    QVariant value(const QQmlAdaptorModel &model, int index, const QString &role) const override
    {
        if (!metaObject) {
            VDMAbstractItemModelDataType *dataType = const_cast<VDMAbstractItemModelDataType *>(this);
            dataType->initializeMetaType(model);
        }

        if (const QAbstractItemModel *aim = model.aim()) {
            QHash<QByteArray, int>::const_iterator it = roleNames.find(role.toUtf8());
            if (it != roleNames.end()) {
                return aim->index(model.rowAt(index), model.columnAt(index),
                                  model.rootIndex).data(*it);
            } else if (role == QLatin1String("hasModelChildren")) {
                return QVariant(aim->hasChildren(aim->index(model.rowAt(index),
                                                            model.columnAt(index),
                                                            model.rootIndex)));
            }
        }
        return QVariant();
    }

    QVariant parentModelIndex(const QQmlAdaptorModel &model) const override
    {
        if (const QAbstractItemModel *aim = model.aim())
            return QVariant::fromValue(aim->parent(model.rootIndex));
        return QVariant();
    }

    QVariant modelIndex(const QQmlAdaptorModel &model, int index) const override
    {
        if (const QAbstractItemModel *aim = model.aim())
            return QVariant::fromValue(aim->index(model.rowAt(index), model.columnAt(index),
                                                  model.rootIndex));
        return QVariant();
    }

    bool canFetchMore(const QQmlAdaptorModel &model) const override
    {
        if (const QAbstractItemModel *aim = model.aim())
            return aim->canFetchMore(model.rootIndex);
        return false;
    }

    void fetchMore(QQmlAdaptorModel &model) const override
    {
        if (QAbstractItemModel *aim = model.aim())
            aim->fetchMore(model.rootIndex);
    }

    QQmlDelegateModelItem *createItem(
            QQmlAdaptorModel &model,
            const QQmlRefPointer<QQmlDelegateModelItemMetaType> &metaType,
            int index, int row, int column) override
    {
        if (!metaObject)
            initializeMetaType(model);
        return new QQmlDMAbstractItemModelData(metaType, this, index, row, column);
    }

    void initializeMetaType(const QQmlAdaptorModel &model)
    {
        QMetaObjectBuilder builder;
        QQmlAdaptorModelEngineData::setModelDataType<QQmlDMAbstractItemModelData>(&builder, this);

        const QByteArray propertyType = QByteArrayLiteral("QVariant");
        const QAbstractItemModel *aim = model.aim();
        const QHash<int, QByteArray> names = aim ? aim->roleNames() : QHash<int, QByteArray>();
        for (QHash<int, QByteArray>::const_iterator it = names.begin(), cend = names.end(); it != cend; ++it) {
            const int propertyId = propertyRoles.size();
            propertyRoles.append(it.key());
            roleNames.insert(it.value(), it.key());
            QQmlAdaptorModelEngineData::addProperty(&builder, propertyId, it.value(), propertyType);
        }

        if (propertyRoles.size() == 1) {
            hasModelData = true;
            const int role = names.begin().key();
            const QByteArray propertyName = QByteArrayLiteral("modelData");

            propertyRoles.append(role);
            roleNames.insert(propertyName, role);
            QQmlAdaptorModelEngineData::addProperty(&builder, 1, propertyName, propertyType);
        }

        metaObject.reset(builder.toMetaObject());
        *static_cast<QMetaObject *>(this) = *metaObject;
        propertyCache = QQmlPropertyCache::createStandalone(
                    metaObject.data(), model.modelItemRevision);
    }

    QV4::PersistentValue prototype;
    QList<int> propertyRoles;
    QList<int> watchedRoleIds;
    QList<QByteArray> watchedRoles;
    QHash<QByteArray, int> roleNames;
    QQmlAdaptorModel *model;
    int propertyOffset;
    int signalOffset;
    bool hasModelData;
};

QT_END_NAMESPACE

#endif // QQMLDMABSTRACTITEMMODELDATA_P_H
