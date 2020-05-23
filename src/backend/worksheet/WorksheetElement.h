/***************************************************************************
    File                 : WorksheetElement.h
    Project              : LabPlot
    Description          : Base class for all Worksheet children.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2015 by Alexander Semke (alexander.semke@web.de)

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

#ifndef WORKSHEETELEMENT_H
#define WORKSHEETELEMENT_H

#include "backend/core/AbstractAspect.h"
#include <QPainterPath>

class QAction;
class QGraphicsItem;
class QPen;
class KConfig;

class WorksheetElement : public AbstractAspect {
	Q_OBJECT

public:
	WorksheetElement(const QString&, AspectType);
	~WorksheetElement() override;

	enum class WorksheetElementName {NameCartesianPlot = 1};

	enum class HorizontalPosition {Left, Center, Right, Custom};
	enum class VerticalPosition {Top, Center, Bottom, Custom};

	enum class HorizontalAlignment {Left, Center, Right};
	enum class VerticalAlignment {Top, Center, Bottom};

	struct PositionWrapper {
		QPointF point;
		WorksheetElement::HorizontalPosition horizontalPosition;
		WorksheetElement::VerticalPosition verticalPosition;
	};

	virtual QGraphicsItem* graphicsItem() const = 0;
	virtual void setZValue(qreal);
	virtual void setVisible(bool on) = 0;
	virtual bool isVisible() const = 0;
	virtual bool isFullyVisible() const;
	virtual void setPrinting(bool) = 0;

	QMenu* createContextMenu() override;

	virtual void loadThemeConfig(const KConfig&);
	virtual void saveThemeConfig(const KConfig&);

	static QPainterPath shapeFromPath(const QPainterPath&, const QPen&);
	virtual void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) = 0;

public slots:
	virtual void retransform() = 0;

private:
	QMenu* m_drawingOrderMenu;
	QMenu* m_moveBehindMenu;
	QMenu* m_moveInFrontOfMenu;

private slots:
	void prepareMoveBehindMenu();
	void prepareMoveInFrontOfMenu();
	void execMoveBehind(QAction*);
	void execMoveInFrontOf(QAction*);

signals:
	friend class AbstractPlotSetHorizontalPaddingCmd;
	friend class AbstractPlotSetVerticalPaddingCmd;
	friend class AbstractPlotSetRightPaddingCmd;
	friend class AbstractPlotSetBottomPaddingCmd;
	friend class AbstractPlotSetSymmetricPaddingCmd;
	void horizontalPaddingChanged(float);
	void verticalPaddingChanged(float);
	void rightPaddingChanged(double);
	void bottomPaddingChanged(double);
	void symmetricPaddingChanged(double);

	void hovered();
	void unhovered();
};

#endif
