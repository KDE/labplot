/***************************************************************************
    File                 : CustomPoint.h
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

#ifndef CUSTOMPOINT_H
#define CUSTOMPOINT_H

#include <QObject>
#include <QBrush>
#include <QPen>

#include "backend/lib/macros.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/worksheet/WorksheetElement.h"

class CustomPoint;

class ErrorBarItem : public QObject, public QGraphicsRectItem {
	Q_OBJECT

public:
	enum ErrorBarType { PlusDeltaX, MinusDeltaX, PlusDeltaY, MinusDeltaY};

    explicit ErrorBarItem(CustomPoint* parent = 0, const ErrorBarType& type = PlusDeltaX);

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
    CustomPoint* m_parentItem;
};

class CustomPointPrivate;
class CustomPoint : public WorksheetElement {
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

    explicit CustomPoint(const QString& name );
    ~CustomPoint();

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

    typedef CustomPointPrivate Private;

    static QPainterPath pointPathFromStyle(CustomPoint::PointsStyle);
    static QString pointNameFromStyle(CustomPoint::PointsStyle);

public slots:
	virtual void retransform();
	virtual void handlePageResize(double horizontalRatio, double verticalRatio);

private slots:
	void visibilityChanged();

protected:
    CustomPointPrivate* const d_ptr;
    CustomPoint(const QString &name, CustomPointPrivate *dd);

private:
    Q_DECLARE_PRIVATE(CustomPoint)
	void init();
	void initActions();

	QAction* visibilityAction;
	QList<ErrorBarItem*> m_errorBarItemList;

signals:
    friend class CustomPointSetPositionCmd;
    void positionChanged(const CustomPoint::PositionWrapper&);
	void visibleChanged(bool);
	void changed();

    friend class CustomPointSetPointStyleCmd;
    friend class CustomPointSetSizeCmd;
    friend class CustomPointSetRotationAngleCmd;
    friend class CustomPointSetOpacityCmd;
    friend class CustomPointSetBrushCmd;
    friend class CustomPointSetPenCmd;
    void pointStyleChanged(CustomPoint::PointsStyle);
	void sizeChanged(qreal);
	void rotationAngleChanged(qreal);
	void opacityChanged(qreal);
	void brushChanged(QBrush);
	void penChanged(const QPen&);

    friend class CustomPointSetErrorBarSizeCmd;
    friend class CustomPointSetErrorBarPenCmd;
    friend class CustomPointSetErrorBarBrushCmd;
    friend class CustomPointSetPlusDeltaXPosCmd;
    friend class CustomPointSetMinusDeltaXPosCmd;
    friend class CustomPointSetPlusDeltaYPosCmd;
    friend class CustomPointSetMinusDeltaYPosCmd;
	void plusDeltaXPosChanged(const QPointF&);
	void minusDeltaXPosChanged(const QPointF&);
	void plusDeltaYPosChanged(const QPointF&);
	void minusDeltaYPosChanged(const QPointF&);
	void errorBarSizeChanged(qreal);
	void errorBarBrushChanged(QBrush);
	void errorBarPenChanged(const QPen&);
};

#endif
