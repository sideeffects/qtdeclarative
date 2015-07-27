/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKSWITCH_P_H
#define QQUICKSWITCH_P_H

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

#include <QtQuickControls/private/qquickcheckable_p.h>

QT_BEGIN_NAMESPACE

class QQuickSwitchPrivate;

class Q_QUICKCONTROLS_EXPORT QQuickSwitch : public QQuickCheckable
{
    Q_OBJECT
    Q_PROPERTY(qreal position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(qreal visualPosition READ visualPosition NOTIFY visualPositionChanged FINAL)

public:
    explicit QQuickSwitch(QQuickItem *parent = Q_NULLPTR);

    qreal position() const;
    void setPosition(qreal position);

    qreal visualPosition() const;

Q_SIGNALS:
    void positionChanged();
    void visualPositionChanged();

protected:
    void mirrorChange() Q_DECL_OVERRIDE;
    bool childMouseEventFilter(QQuickItem *child, QEvent *event) Q_DECL_OVERRIDE;

    bool handleMousePressEvent(QQuickItem *child, QMouseEvent *event);
    bool handleMouseMoveEvent(QQuickItem *child, QMouseEvent *event);
    bool handleMouseReleaseEvent(QQuickItem *child, QMouseEvent *event);
    bool handleMouseUngrabEvent(QQuickItem *child);

    virtual qreal positionAt(const QPoint &point) const;

private:
    Q_DISABLE_COPY(QQuickSwitch)
    Q_DECLARE_PRIVATE(QQuickSwitch)
};

Q_DECLARE_TYPEINFO(QQuickSwitch, Q_COMPLEX_TYPE);

QT_END_NAMESPACE

#endif // QQUICKSWITCH_P_H
