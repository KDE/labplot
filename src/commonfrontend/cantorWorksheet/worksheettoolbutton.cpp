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

#include "worksheettoolbutton.h"
#include <QPixmap>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>

WorksheetToolButton::WorksheetToolButton(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    m_size = QSize(16, 16);
    setCursor(QCursor(Qt::ArrowCursor));
    m_scale = 0;
}

WorksheetToolButton::~WorksheetToolButton()
{
}

void WorksheetToolButton::setIcon(const QIcon& icon)
{
    m_icon = icon;
}

qreal WorksheetToolButton::width()
{
    return m_size.width();
}

qreal WorksheetToolButton::height()
{
    return m_size.height();
}

QRectF WorksheetToolButton::boundingRect() const
{
    return QRectF(0, 0, m_size.width(), m_size.height());
}

void WorksheetToolButton::setIconScale(qreal scale)
{
    m_scale = scale;
    m_pixmap = m_icon.pixmap(m_size * m_scale);
}

void WorksheetToolButton::paint(QPainter* painter,
                                const QStyleOptionGraphicsItem* option,
                                QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_scale == 0)
        setIconScale(1);
    QRectF rect(QPointF(0,0), m_size);
    painter->drawPixmap(rect, m_pixmap, m_pixmap.rect());
}

void WorksheetToolButton::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    emit pressed();
}

void WorksheetToolButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (boundingRect().contains(event->pos()))
        emit clicked();
}
