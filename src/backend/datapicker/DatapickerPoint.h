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

#include "backend/lib/macros.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/worksheet/WorksheetElement.h"

class DatapickerPoint;

class ErrorBarItem : public QObject, public QGraphicsRectItem {
	Q_OBJECT

public:
	enum ErrorBarType { PlusDeltaX, MinusDeltaX, PlusDeltaY, MinusDeltaY};

	explicit ErrorBarItem(DatapickerPoint* parent = 0, const ErrorBarType& type = PlusDeltaX);

	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void setRectSize(const qreal);

public slots:
	void setPosition(const QPointF&);

private:
	void initRect();
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
	QGraphicsLineItem* barLineItem;
	QRectF m_rect;
	ErrorBarType m_type;
	DatapickerPoint* m_parentItem;
};

class DatapickerPointPrivate;
class DatapickerPoint : public WorksheetElement {
	Q_OBJECT

public:
	enum HorizontalPosition { hPositionLeft, hPositionCenter, hPositionRight, hPositionCustom };
	enum VerticalPosition { vPositionTop, vPositionCenter, vPositionBottom, vPositionCustom };
	enum PointsStyle {Circle, Square, EquilateralTriangle, RightTriangle, Bar, PeakedBar,
	                  SkewedBar, Diamond, Lozenge, Tie, TinyTie, Plus, Boomerang, SmallBoomerang,
	                  Star4, Star5, Line, Cross
	                 };

	struct PositionWrapper {
		QPointF 		   point;
		HorizontalPosition horizontalPosition;
		VerticalPosition   verticalPosition;
	};

	explicit DatapickerPoint(const QString& name );
	~DatapickerPoint();

	virtual QIcon icon() const;
	virtual QMenu* createContextMenu();
	virtual QGraphicsItem *graphicsItem() const;
	void setParentGraphicsItem(QGraphicsItem*);

	void initErrorBar(const DatapickerCurve::Errors&);

	virtual void save(QXmlStreamWriter *) const;
	virtual bool load(XmlStreamReader *);

	CLASS_D_ACCESSOR_DECL(PositionWrapper, position, Position)
	void setPosition(const QPointF&);

	BASIC_D_ACCESSOR_DECL(PointsStyle, pointStyle, PointStyle)
	BASIC_D_ACCESSOR_DECL(qreal, opacity, Opacity)
	BASIC_D_ACCESSOR_DECL(qreal, rotationAngle, RotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, size, Size)
	CLASS_D_ACCESSOR_DECL(QBrush, brush, Brush)
	CLASS_D_ACCESSOR_DECL(QPen, pen, Pen)

	BASIC_D_ACCESSOR_DECL(qreal, errorBarSize, ErrorBarSize)
	CLASS_D_ACCESSOR_DECL(QBrush, errorBarBrush, ErrorBarBrush)
	CLASS_D_ACCESSOR_DECL(QPen, errorBarPen, ErrorBarPen)
	CLASS_D_ACCESSOR_DECL(QPointF, plusDeltaXPos, PlusDeltaXPos)
	CLASS_D_ACCESSOR_DECL(QPointF, minusDeltaXPos, MinusDeltaXPos)
	CLASS_D_ACCESSOR_DECL(QPointF, plusDeltaYPos, PlusDeltaYPos)
	CLASS_D_ACCESSOR_DECL(QPointF, minusDeltaYPos, MinusDeltaYPos)
	BASIC_D_ACCESSOR_DECL(bool, xSymmetricError, XSymmetricError)
	BASIC_D_ACCESSOR_DECL(bool, ySymmetricError, YSymmetricError)



	virtual void setVisible(bool on);
	virtual bool isVisible() const;
	virtual void setPrinting(bool);
	void suppressHoverEvents(bool);

	typedef DatapickerPointPrivate Private;

	static QPainterPath pointPathFromStyle(DatapickerPoint::PointsStyle);
	static QString pointNameFromStyle(DatapickerPoint::PointsStyle);

public slots:
	virtual void retransform();
	virtual void handlePageResize(double horizontalRatio, double verticalRatio);

private slots:
	void visibilityChanged();

protected:
	DatapickerPointPrivate* const d_ptr;
	DatapickerPoint(const QString &name, DatapickerPointPrivate *dd);

private:
	Q_DECLARE_PRIVATE(DatapickerPoint)
	void init();
	void initActions();

	QAction* visibilityAction;
	QList<ErrorBarItem*> m_errorBarItemList;

signals:
	friend class DatapickerPointSetPositionCmd;
	void positionChanged(const DatapickerPoint::PositionWrapper&);
	void visibleChanged(bool);
	void changed();

	friend class DatapickerPointSetPointStyleCmd;
	friend class DatapickerPointSetSizeCmd;
	friend class DatapickerPointSetRotationAngleCmd;
	friend class DatapickerPointSetOpacityCmd;
	friend class DatapickerPointSetBrushCmd;
	friend class DatapickerPointSetPenCmd;
	void pointStyleChanged(DatapickerPoint::PointsStyle);
	void sizeChanged(qreal);
	void rotationAngleChanged(qreal);
	void opacityChanged(qreal);
	void brushChanged(QBrush);
	void penChanged(const QPen&);

	friend class DatapickerPointSetErrorBarSizeCmd;
	friend class DatapickerPointSetErrorBarPenCmd;
	friend class DatapickerPointSetErrorBarBrushCmd;
	friend class DatapickerPointSetPlusDeltaXPosCmd;
	friend class DatapickerPointSetMinusDeltaXPosCmd;
	friend class DatapickerPointSetPlusDeltaYPosCmd;
	friend class DatapickerPointSetMinusDeltaYPosCmd;
	void plusDeltaXPosChanged(const QPointF&);
	void minusDeltaXPosChanged(const QPointF&);
	void plusDeltaYPosChanged(const QPointF&);
	void minusDeltaYPosChanged(const QPointF&);
	void errorBarSizeChanged(qreal);
	void errorBarBrushChanged(QBrush);
	void errorBarPenChanged(const QPen&);
};

#endif
