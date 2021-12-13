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

#include "backend/lib/macros.h"
#include "backend/core/AbstractAspect.h"
#include <QPainterPath>

#define D_OBJ(obj_class) static_cast<obj_class##Private*>(d_ptr)
#define D(obj_class) auto * d = static_cast<obj_class##Private*>(d_ptr)

class CartesianPlot;
class CartesianCoordinateSystem;
class WorksheetElementPrivate;
class KConfig;

class QAction;
class QGraphicsItem;
class QPen;

class WorksheetElement : public AbstractAspect {
	Q_OBJECT

public:
//	WorksheetElement(const QString&, AspectType);
	~WorksheetElement() override;

	enum class Orientation {Horizontal, Vertical, Both};
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

	typedef WorksheetElementPrivate Private;

	CLASS_D_ACCESSOR_DECL(PositionWrapper, position, Position)
// 	BASIC_D_ACCESSOR_DELC(bool, coordinateBindingEnabled, CoordinateBindingEnabled)
	void setCoordinateBindingEnabled(bool);
	bool coordinateBindingEnabled() const;
	BASIC_D_ACCESSOR_DECL(QPointF, positionLogical, PositionLogical)
	void setPosition(QPointF);
	void setPositionInvalid(bool);
	BASIC_D_ACCESSOR_DECL(HorizontalAlignment, horizontalAlignment, HorizontalAlignment)
	BASIC_D_ACCESSOR_DECL(VerticalAlignment, verticalAlignment, VerticalAlignment)
	BASIC_D_ACCESSOR_DECL(qreal, rotationAngle, RotationAngle)

	void finalizeAdd() override;

	virtual QGraphicsItem* graphicsItem() const = 0;
	virtual void setZValue(qreal);
	virtual void setVisible(bool on);
	virtual bool isVisible() const;
	virtual bool isFullyVisible() const;

	virtual void setPrinting(bool);
	bool isPrinting() const;

	QRectF parentRect() const;
	QPointF parentPosToRelativePos(QPointF parentPos, QRectF rect,
								   PositionWrapper, HorizontalAlignment, VerticalAlignment) const;
	QPointF relativePosToParentPos(QRectF rect, PositionWrapper,
								   HorizontalAlignment, VerticalAlignment) const;

	QPointF align(QPointF pos, QRectF rect, HorizontalAlignment horAlign, VerticalAlignment vertAlign, bool positive) const;

	QMenu* createContextMenu() override;

	virtual void save(QXmlStreamWriter*) const;
	virtual bool load(XmlStreamReader*);
	virtual void loadThemeConfig(const KConfig&);
	virtual void saveThemeConfig(const KConfig&);

	static QPainterPath shapeFromPath(const QPainterPath&, const QPen&);
	virtual void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) = 0;

	CartesianPlot* plot() const { return m_plot; }	// used in the element docks
	int coordinateSystemIndex() const { return m_cSystemIndex; }
	void setCoordinateSystemIndex(int);
	int coordinateSystemCount() const;
	QString coordinateSystemInfo(int index) const;

private:
	void init();

protected:

	WorksheetElement(const QString&, WorksheetElementPrivate* dd, AspectType);
	int m_cSystemIndex{0};	// index of coordinate system used from plot
	// parent plot if available
	// not const because of prepareGeometryChange()
	// normally set in finalizeAdd()
	CartesianPlot* m_plot{nullptr};
	const CartesianCoordinateSystem* cSystem{nullptr};	//current cSystem

public slots:
	virtual void retransform() = 0;

protected:

	WorksheetElementPrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(WorksheetElement)
	QMenu* m_drawingOrderMenu{nullptr};
	QMenu* m_moveBehindMenu{nullptr};
	QMenu* m_moveInFrontOfMenu{nullptr};
	bool m_printing{false};

private slots:
	void prepareDrawingOrderMenu();
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
	void positionChanged(const WorksheetElement::PositionWrapper&) const;
	void horizontalAlignmentChanged(const WorksheetElement::HorizontalAlignment) const;
	void verticalAlignmentChanged(const WorksheetElement::VerticalAlignment) const;
	void coordinateBindingEnabledChanged(bool) const;
	void positionLogicalChanged(QPointF) const;
	void rotationAngleChanged(qreal) const;
	void visibleChanged(bool) const;
	void changed();

	void hovered();
	void unhovered();
	// needed in the worksheet info element, because execMoveInFrontOf and execMoveBehind
	// call also child removed but this is only temporary
	void moveBegin(); // called, at the begin of execMoveInFrontOf or execMoveBehind is called
	void moveEnd(); // called, at the end of execMoveInFrontOf or execMoveBehind is called

	void plotRangeListChanged();
};

#endif
