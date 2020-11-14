/***************************************************************************
    File                 : CustomPoint.h
    Project              : LabPlot
    Description          : Custom user-defined point on the plot
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
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

#include <QPen>

#include "backend/lib/macros.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/WorksheetElement.h"

class QBrush;
class CustomPointPrivate;
class CartesianPlot;

class CustomPoint : public WorksheetElement {
	Q_OBJECT

public:
	explicit CustomPoint(const CartesianPlot*, const QString&);
	~CustomPoint() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	BASIC_D_ACCESSOR_DECL(QPointF, position, Position)
	BASIC_D_ACCESSOR_DECL(Symbol::Style, symbolStyle, SymbolStyle)
	BASIC_D_ACCESSOR_DECL(qreal, symbolOpacity, SymbolOpacity)
	BASIC_D_ACCESSOR_DECL(qreal, symbolRotationAngle, SymbolRotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, symbolSize, SymbolSize)
	CLASS_D_ACCESSOR_DECL(QBrush, symbolBrush, SymbolBrush)
	CLASS_D_ACCESSOR_DECL(QPen, symbolPen, SymbolPen)

	void setVisible(bool on) override;
	bool isVisible() const override;
	void setPrinting(bool) override;
	void setParentGraphicsItem(QGraphicsItem* item);

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef CustomPointPrivate Private;

private slots:
	void visibilityChanged();

protected:
	CustomPointPrivate* const d_ptr;
	CustomPoint(const QString& name, CustomPointPrivate* dd);

private:
	Q_DECLARE_PRIVATE(CustomPoint)
	void init();
	void initActions();

	QAction* visibilityAction;

signals:
	friend class CustomPointSetPositionCmd;
	void positionChanged(const QPointF&);
	void visibleChanged(bool);
	void changed();

	void symbolStyleChanged(Symbol::Style);
	void symbolSizeChanged(qreal);
	void symbolRotationAngleChanged(qreal);
	void symbolOpacityChanged(qreal);
	void symbolBrushChanged(QBrush);
	void symbolPenChanged(const QPen&);
};

#endif
