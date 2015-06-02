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

#ifndef RESULTITEM_H
#define RESULTITEM_H

/*
 * This is a common superclass of all result items. Unfortunately this class
 * cannot inherit QGraphicsItem or QObject, because the subclasses inherit
 * these from an other source (TextResultItem inherits WorksheetTextItem, for
 * example). Therefore this class mainly offers the interface, and the
 * implementations are done in each subclasses, even when the code is literally
 * the same for them.
 */

namespace Cantor {
    class Result;
};

class WorksheetEntry;

class QMenu;
class QObject;
class QPointF;
class QGraphicsObject;

class ResultItem
{
  public:
    ResultItem();
    virtual ~ResultItem();

    static ResultItem* create(WorksheetEntry* parent, Cantor::Result* result);

    virtual double setGeometry(double x, double y, double w) = 0;
    virtual void populateMenu(QMenu* menu, const QPointF& pos) = 0;

    virtual ResultItem* updateFromResult(Cantor::Result* result) = 0;

    virtual void deleteLater() = 0;

    virtual Cantor::Result* result() = 0;

    virtual double width() const = 0;
    virtual double height() const = 0;

    QGraphicsObject* graphicsObject();

  protected:
    static void addCommonActions(QObject* self, QMenu* menu);
};

#endif // RESULTITEM_H
