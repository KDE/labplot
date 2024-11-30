/*
	File                 : WorksheetElement.h
	Project              : LabPlot
	Description          : Base class for all Worksheet children.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2012-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETELEMENT_H
#define WORKSHEETELEMENT_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"
#include <QPainterPath>

#define D(obj_class) auto* d = static_cast<obj_class##Private*>(d_ptr)

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
	WorksheetElement(const QString& name, AspectType type);
	~WorksheetElement() override;

	enum class Orientation { Horizontal, Vertical, Both };
	enum class HorizontalPosition { Left, Center, Right, Relative }; // Relative: relative to plot area
	enum class VerticalPosition { Top, Center, Bottom, Relative };

	enum class HorizontalAlignment { Left, Center, Right };
	enum class VerticalAlignment { Top, Center, Bottom };

	enum class PositionLimit { None, X, Y };

	struct PositionWrapper {
		PositionWrapper() {
		}
		PositionWrapper(QPointF p, HorizontalPosition hor, VerticalPosition vert, PositionLimit limit)
			: point(p)
			, horizontalPosition(hor)
			, verticalPosition(vert)
			, positionLimit(limit) {
		}

		QPointF point; // range [0 .. 1] for relative position
		HorizontalPosition horizontalPosition{HorizontalPosition::Center};
		VerticalPosition verticalPosition{VerticalPosition::Center};
		PositionLimit positionLimit{PositionLimit::None};
	};

	typedef WorksheetElementPrivate Private;

	void setPositionHorizontal(HorizontalPosition);
	void setPositionVertical(VerticalPosition);
	bool setXPositionRelative(double pos);
	bool setYPositionRelative(double pos);
	CLASS_D_ACCESSOR_DECL(PositionWrapper, position, Position)
	bool setCoordinateBindingEnabled(bool);
	bool coordinateBindingEnabled() const;
	BASIC_D_ACCESSOR_DECL(QPointF, positionLogical, PositionLogical)
	void setPosition(QPointF);
	void setPositionInvalid(bool);
	BASIC_D_ACCESSOR_DECL(HorizontalAlignment, horizontalAlignment, HorizontalAlignment)
	BASIC_D_ACCESSOR_DECL(VerticalAlignment, verticalAlignment, VerticalAlignment)
	BASIC_D_ACCESSOR_DECL(qreal, rotationAngle, RotationAngle)
	qreal scale() const;
	BASIC_D_ACCESSOR_DECL(bool, isLocked, Lock)
	BASIC_D_ACCESSOR_DECL(bool, isHovered, Hover)

	void finalizeAdd() override;

	virtual QGraphicsItem* graphicsItem() const;
	virtual void setParentGraphicsItem(QGraphicsItem* item);
	virtual void setZValue(qreal);
	virtual void setVisible(bool on);
	virtual bool isVisible() const;
	virtual bool isFullyVisible() const;
	void setSuppressRetransform(bool);

	virtual void setPrinting(bool);
	bool isPrinting() const;

	QRectF parentRect() const;
	QPointF parentPosToRelativePos(QPointF parentPos, PositionWrapper) const;
	QPointF relativePosToParentPos(PositionWrapper) const;

	QPointF align(QPointF, QRectF, HorizontalAlignment, VerticalAlignment, bool positive) const;

	virtual QMenu* createContextMenu() override;
	QAction* visibilityAction();
	QAction* lockingAction();

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool) override;
	virtual void loadThemeConfig(const KConfig&);
	virtual void saveThemeConfig(const KConfig&);

	static QPainterPath shapeFromPath(const QPainterPath&, const QPen&);
	virtual void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) = 0;

	CartesianPlot* plot() const; // used in the element docks
	int coordinateSystemIndex() const {
		return m_cSystemIndex;
	}
	void setCoordinateSystemIndex(int, QUndoCommand* parent = nullptr);
	int coordinateSystemCount() const;
	QString coordinateSystemInfo(int index) const;

private:
	void init();
	QAction* m_visibilityAction{nullptr};
	QAction* m_lockingAction{nullptr};

protected:
	WorksheetElement(const QString&, WorksheetElementPrivate* dd, AspectType);
	int m_cSystemIndex{0}; // index of coordinate system used from plot
	const CartesianCoordinateSystem* cSystem{nullptr}; // current cSystem

	virtual void handleAspectUpdated(const QString& path, const AbstractAspect*);
	friend class Project;

public Q_SLOTS:
	virtual void retransform() = 0;

protected:
	WorksheetElementPrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(WorksheetElement)
	QMenu* m_drawingOrderMenu{nullptr};
	QMenu* m_moveBehindMenu{nullptr};
	QMenu* m_moveInFrontOfMenu{nullptr};
	bool m_printing{false};

protected Q_SLOTS:
	void changeVisibility();
	void changeLocking();

private Q_SLOTS:
	void prepareDrawingOrderMenu();
	void execMoveBehind(QAction*);
	void execMoveInFrontOf(QAction*);

Q_SIGNALS:
	friend class AbstractPlotSetHorizontalPaddingCmd;
	friend class AbstractPlotSetVerticalPaddingCmd;
	friend class AbstractPlotSetRightPaddingCmd;
	friend class AbstractPlotSetBottomPaddingCmd;
	friend class AbstractPlotSetSymmetricPaddingCmd;
	void positionChanged(const WorksheetElement::PositionWrapper&) const;
	void horizontalAlignmentChanged(const WorksheetElement::HorizontalAlignment) const;
	void verticalAlignmentChanged(const WorksheetElement::VerticalAlignment) const;
	void coordinateBindingEnabledChanged(bool) const;
	void positionLogicalChanged(QPointF) const;
	void rotationAngleChanged(qreal) const;
	void rotationChanged(qreal) const;
	void visibleChanged(bool) const;
	void lockChanged(bool) const;
	void coordinateSystemIndexChanged(int) const;
	void changed();
	void hoveredChanged(bool) const;

	void objectPositionChanged(); // Position changed, independend of logical or scene, bot are triggering this

	void hovered();
	void unhovered();

	void plotRangeListChanged();

	friend class WorksheetElementTest;
	friend class SetCoordinateSystemIndexCmd;
};

#endif
