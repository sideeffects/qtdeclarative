/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the V4VM module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QV4STRING_H
#define QV4STRING_H

#include <QtCore/qstring.h>
#include <qv4managed.h>

QT_BEGIN_NAMESPACE

namespace QQmlJS {
namespace VM {

struct ExecutionEngine;

struct String : public Managed {
    enum StringType {
        StringType_Unknown,
        StringType_Regular,
        StringType_Identifier,
        StringType_ArrayIndex
    };

    String(const QString &text)
        : _text(text), stringHash(InvalidHashValue)
    { type = Type_String; subtype = StringType_Unknown; }

    inline bool isEqualTo(const String *other) const {
        if (this == other)
            return true;
        if (hashValue() != other->hashValue())
            return false;
        if (subtype == other->subtype) {
            if (subtype == StringType_ArrayIndex)
                return true;
            if (subtype == StringType_Identifier)
                return stringIdentifier == other->stringIdentifier;
        }
        return toQString() == other->toQString();
    }

    inline const QString &toQString() const {
        return _text;
    }

    inline unsigned hashValue() const {
        if (subtype == StringType_Unknown)
            createHashValue();

        return stringHash;
    }
    enum {
        InvalidArrayIndex = 0xffffffff,
        InvalidHashValue  = 0xffffffff
    };
    uint asArrayIndex() const {
        if (subtype == StringType_Unknown)
            createHashValue();
        if (subtype == StringType_ArrayIndex)
            return stringHash;
        return UINT_MAX;
    }
    uint toUInt(bool *ok) const;

    void makeIdentifier(const ExecutionContext *ctx) {
        if (subtype == StringType_Identifier)
            return;
        makeIdentifierImpl(ctx);
    }

    void makeIdentifierImpl(const ExecutionContext *ctx);

private:
    friend struct Identifiers;
    void createHashValue() const;

    QString _text;
    mutable uint stringHash;
};

} // namespace VM
} // namespace QQmlJS

QT_END_NAMESPACE

#endif
