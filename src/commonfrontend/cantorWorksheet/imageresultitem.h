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

#ifndef IMAGERESULTITEM_H
#define IMAGERESULTITEM_H

#include "resultitem.h"
#include "worksheetimageitem.h"
#include "worksheetentry.h"
#include "epsrenderer.h"

class ImageResultItem : public WorksheetImageItem, public ResultItem
{
  Q_OBJECT
  public:
    ImageResultItem(QGraphicsObject* parent);
    ~ImageResultItem();

    using WorksheetImageItem::setGeometry;
    double setGeometry(double x, double y, double w);
    void populateMenu(QMenu* menu, const QPointF& pos);

    ResultItem* updateFromResult(Cantor::Result* result);

    QRectF boundingRect() const;
    virtual double width() const;
    virtual double height() const;

    void deleteLater();
    EpsRenderer* epsRenderer();
    CommandEntry* parentEntry();
    Cantor::Result* result();

  Q_SIGNALS:
    void removeResult();

  protected Q_SLOTS:
    void saveResult();
};

#endif // IMAGERESULTITEM_H
