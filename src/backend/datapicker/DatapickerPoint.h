/***************************************************************************
    File                 : DatapickerPoint.h
    Project              : LabPlot
    Description          : Graphic Item for coordinate points of Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef DATAPICKERPOINT_H
#define DATAPICKERPOINT_H

#include <QObject>
#include <QBrush>
#include <QPen>

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"
#include "backend/datapicker/DatapickerCurve.h"
#include <QGraphicsItem>
#include "backend/worksheet/plots/cartesian/Symbol.h"

class DatapickerPoint;

class ErrorBarItem : public QObject, public QGraphicsRectItem {
	Q_OBJECT

public:
	enum ErrorBarType { PlusDeltaX, MinusDeltaX, PlusDeltaY, MinusDeltaY};

	explicit ErrorBarItem(DatapickerPoint* parent = 0, const ErrorBarType& type = PlusDeltaX);

	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
	void setRectSize(const qreal);

public slots:
	void setPosition(const QPointF&);

private:
	void initRect();
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	QGraphicsLineItem* barLineItem;
	QRectF m_rect;
	ErrorBarType m_type;
	DatapickerPoint* m_parentItem;
};

class DatapickerPointPrivate;
class DatapickerPoint : public AbstractAspect {
	Q_OBJECT

public:
	explicit DatapickerPoint(const QString& name );
	~DatapickerPoint() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem *graphicsItem() const;
	void setParentGraphicsItem(QGraphicsItem*);
	void setPrinting(bool);
	void initErrorBar(const DatapickerCurve::Errors&);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(QPointF, position, Position)
	CLASS_D_ACCESSOR_DECL(QPointF, plusDeltaXPos, PlusDeltaXPos)
	CLASS_D_ACCESSOR_DECL(QPointF, minusDeltaXPos, MinusDeltaXPos)
	CLASS_D_ACCESSOR_DECL(QPointF, plusDeltaYPos, PlusDeltaYPos)
	CLASS_D_ACCESSOR_DECL(QPointF, minusDeltaYPos, MinusDeltaYPos)

	typedef DatapickerPointPrivate Private;

public slots:
	void retransform();

protected:
	DatapickerPointPrivate* const d_ptr;
	DatapickerPoint(const QString &name, DatapickerPointPrivate *dd);
	static QPen selectedPen;
	static float selectedOpacity;

private:
	Q_DECLARE_PRIVATE(DatapickerPoint)
	void init();

	QList<ErrorBarItem*> m_errorBarItemList;

signals:
	friend class DatapickerPointSetPositionCmd;
	void positionChanged(QPointF);

	friend class DatapickerPointSetPlusDeltaXPosCmd;
	friend class DatapickerPointSetMinusDeltaXPosCmd;
	friend class DatapickerPointSetPlusDeltaYPosCmd;
	friend class DatapickerPointSetMinusDeltaYPosCmd;
	void plusDeltaXPosChanged(QPointF);
	void minusDeltaXPosChanged(QPointF);
	void plusDeltaYPosChanged(QPointF);
	void minusDeltaYPosChanged(QPointF);
};

#endif
