/*
    File                 : WorksheetElement.h
    Project              : LabPlot
    Description          : Base class for all Worksheet children.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2012-2015 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
	enum class Orientation {Horizontal, Vertical, Both};
	WorksheetElement(const QString&, AspectType);
	~WorksheetElement() override;

	enum class HorizontalPosition {Left, Center, Right, Custom};
	enum class VerticalPosition {Top, Center, Bottom, Custom};

	enum class HorizontalAlignment {Left, Center, Right};
	enum class VerticalAlignment {Top, Center, Bottom};

	enum class BackgroundType {Color, Image, Pattern};
	enum class BackgroundColorStyle {SingleColor, HorizontalLinearGradient, VerticalLinearGradient,
			TopLeftDiagonalLinearGradient, BottomLeftDiagonalLinearGradient, RadialGradient};
	enum class BackgroundImageStyle {ScaledCropped, Scaled, ScaledAspectRatio, Centered, Tiled, CenterTiled};

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

	QPointF parentPosToRelativePos(QPointF parentPos, QRectF parentRect, QRectF rect,
								   PositionWrapper, HorizontalAlignment, VerticalAlignment) const;
	QPointF relativePosToParentPos(QRectF parentRect, QRectF rect,
								   PositionWrapper, HorizontalAlignment, VerticalAlignment) const;

	QPointF align(QPointF pos, QRectF rect, HorizontalAlignment horAlign, VerticalAlignment vertAlign, bool positive) const;

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
