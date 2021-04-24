/***************************************************************************
    File                 : WorksheetElement.h
    Project              : LabPlot
    Description          : Base class for all Worksheet children.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2015 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)

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

class CartesianPlot;
class CartesianCoordinateSystem;
class KConfig;
class QAction;
class QGraphicsItem;
class QPen;

class WorksheetElement : public AbstractAspect {
	Q_OBJECT

public:
	enum class Orientation {Horizontal, Vertical};
	WorksheetElement(const QString&, AspectType);
	~WorksheetElement() override;

	enum class WorksheetElementName {NameCartesianPlot = 1};
    // TODO: remove Horizontal position or horizontal alignment
    enum class HorizontalPosition {Left, Center, Right, Custom};
    enum class VerticalPosition {Top, Center, Bottom, Custom};

    enum class HorizontalAlignment {Left, Center, Right};
    enum class VerticalAlignment {Top, Center, Bottom};

	struct PositionWrapper {
		QPointF point;
		WorksheetElement::HorizontalPosition horizontalPosition;
		WorksheetElement::VerticalPosition verticalPosition;
	};

	void finalizeAdd() override;

	virtual QGraphicsItem* graphicsItem() const = 0;
	virtual void setZValue(qreal);
	virtual void setVisible(bool on) = 0;
	virtual bool isVisible() const = 0;
	virtual bool isFullyVisible() const;

	virtual void setPrinting(bool);
	bool isPrinting() const;

    /*!
     * \brief parentPosToRelativePos
     * Converts the absolute position of the element in parent coordinates into the distance between the
     * alignement point of the parent and the element
     * \param parentPos Element position in parent coordinates
     * \param parentRect Parent data rect
     * \param worksheetElementRect element rect
     * \param position contains the alignement of the element to the parent
     * \return distance between the parent position to the element
     */
    QPointF parentPosToRelativePos(QPointF parentPos, QRectF parentRect, QRectF worksheetElementRect, WorksheetElement::PositionWrapper position, WorksheetElement::HorizontalAlignment horAlign, WorksheetElement::VerticalAlignment vertAlign) const;
    /*!
     * \brief relativePosToParentPos
     * \param parentRect
     * \param worksheetElementRect element rect
     * \param position contains the alignement of the element to the parent
     * \return parent position
     */
    QPointF relativePosToParentPos(QRectF parentRect, QRectF worksheetElementRect, PositionWrapper position, HorizontalAlignment horAlign, VerticalAlignment vertAlign) const;
	QMenu* createContextMenu() override;

	virtual void loadThemeConfig(const KConfig&);
	virtual void saveThemeConfig(const KConfig&);

	static QPainterPath shapeFromPath(const QPainterPath&, const QPen&);
	virtual void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) = 0;

	CartesianPlot* plot() const { return m_plot; }	// used in the element docks
	int coordinateSystemIndex() const { return m_cSystemIndex; }
	void setCoordinateSystemIndex(int);
	int coordinateSystemCount() const;
	QString coordinateSystemInfo(int index) const;

protected:
	int m_cSystemIndex{0};	// index of coordinate system used from plot
	// parent plot if available
	// not const because of prepareGeometryChange()
	// normally set in finalizeAdd()
	CartesianPlot* m_plot{nullptr};
	const CartesianCoordinateSystem* cSystem{nullptr};	//current cSystem

public slots:
	virtual void retransform() = 0;

private:
	QMenu* m_drawingOrderMenu;
	QMenu* m_moveBehindMenu;
	QMenu* m_moveInFrontOfMenu;
	bool m_printing{false};

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
	// needed in the worksheet info element, because execMoveInFrontOf and execMoveBehind
	// call also child removed but this is only temporary
	void moveBegin(); // called, at the begin of execMoveInFrontOf or execMoveBehind is called
	void moveEnd(); // called, at the end of execMoveInFrontOf or execMoveBehind is called

	void plotRangeListChanged();
};

#endif
