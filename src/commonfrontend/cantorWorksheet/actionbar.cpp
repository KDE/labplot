/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "actionbar.h"
#include "worksheet.h"
#include "worksheetentry.h"
#include "worksheettoolbutton.h"

#include <QGraphicsProxyWidget>

ActionBar::ActionBar(WorksheetEntry* parent)
    : QGraphicsObject(parent)
{
    m_pos = 0;
    m_height = 0;
    QPointF p = worksheet()->worksheetView()->viewRect().topRight();
    qreal w = qMin(parent->size().width(),
                   parent->mapFromScene(p).x());
    setPos(w, 0);
    connect(worksheet()->worksheetView(), SIGNAL(viewRectChanged(QRectF)),
            this, SLOT(updatePosition()));
}

ActionBar::~ActionBar()
{
}

WorksheetToolButton* ActionBar::addButton(const QIcon& icon, QString toolTip,
                                   QObject* receiver, const char* method )
{
    WorksheetToolButton* button = new WorksheetToolButton(this);
    button->setIcon(icon);
    button->setIconScale(worksheet()->epsRenderer()->scale());
    button->setToolTip(toolTip);
    if (receiver && method)
        connect(button, SIGNAL(clicked()), receiver, method);
    m_pos -= button->width() + 2;
    m_height = (m_height > button->height()) ? m_height : button->height();
    button->setPos(m_pos, 4);
    m_buttons.append(button);
    return button;
}

void ActionBar::addSpace()
{
    m_pos -= 8;
}

void ActionBar::updatePosition()
{
    if (!parentEntry())
        return;
    QPointF p = worksheet()->worksheetView()->viewRect().topRight();
    qreal w = qMin(parentEntry()->size().width(),
                   parentEntry()->mapFromScene(p).x());
    setPos(w, 0);
    const qreal scale = worksheet()->epsRenderer()->scale();
    foreach(WorksheetToolButton* button, m_buttons) {
        button->setIconScale(scale);
    }
}

WorksheetEntry* ActionBar::parentEntry()
{
    return qobject_cast<WorksheetEntry*>(parentObject());
}

QRectF ActionBar::boundingRect() const
{
    return QRectF(m_pos, 0, -m_pos, m_height);
}

void ActionBar::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{
}

Worksheet* ActionBar::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}
