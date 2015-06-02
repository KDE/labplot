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

#ifndef WORKSHEETTOOLBUTTON_H
#define WORKSHEETTOOLBUTTON_H

#include <QGraphicsObject>
#include <QPixmap>

#include <QIcon>

class WorksheetToolButton : public QGraphicsObject
{
  Q_OBJECT
  public:
    WorksheetToolButton(QGraphicsItem* parent);
    ~WorksheetToolButton();

    void setIcon(const QIcon& icon);

    qreal width();
    qreal height();
    QRectF boundingRect() const;
    void setIconScale(qreal scale);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = 0);

  Q_SIGNALS:
    void clicked();
    void pressed();

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

  private:
    QSize m_size;
    QPixmap m_pixmap;
    QIcon m_icon;
    qreal m_scale;
};

#endif //WORKSHEETTOOLBUTTON_H
