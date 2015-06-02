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

#ifndef WORKSHEETIMAGEITEM_H
#define WORKSHEETIMAGEITEM_H

#include <QPixmap>
#include <QGraphicsObject>

class Worksheet;
class QMovie;
class QImage;
class QGraphicsSceneContextMenuEvent;
class QMenu;

class WorksheetImageItem : public QGraphicsObject
{
  Q_OBJECT
  public:
    WorksheetImageItem(QGraphicsObject* parent);
    ~WorksheetImageItem();

    enum {Type = UserType + 101};

    int type() const;

    bool imageIsValid();

    virtual qreal setGeometry(qreal x, qreal y, qreal w, bool centered=false);

    qreal height() const;
    qreal width() const;
    QSizeF size();
    void setSize(QSizeF size);
    QSize imageSize();

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void setEps(const QUrl &url);
    void setImage(QImage img);
    void setPixmap(QPixmap pixmap);

    virtual void populateMenu(QMenu *menu, const QPointF& pos);
    Worksheet* worksheet();

  Q_SIGNALS:
    void sizeChanged();
    void menuCreated(QMenu*, const QPointF&);

  protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

  private:
    QPixmap m_pixmap;
    QSizeF m_size;
    qreal m_maxWidth;
};

#endif //WORKSHEETIMAGEITEM_H
