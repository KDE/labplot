/*
	File                 : WorksheetElement.h
	Project              : LabPlot
	Description          : Base class for all Worksheet children.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2012-2025 Alexander Semke <alexander.semke@web.de>
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

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT WorksheetElement : public AbstractAspect {
#else
class WorksheetElement : public AbstractAspect {
#endif
	Q_OBJECT

public:
	WorksheetElement(const QString& name, AspectType type);
	~WorksheetElement() override;

	enum class Orientation { Horizontal, Vertical, Both };
	Q_ENUM(Orientation)
	enum class HorizontalPosition { Left, Center, Right, Relative }; // Relative: relative to plot area
	Q_ENUM(HorizontalPosition)
	enum class VerticalPosition { Top, Center, Bottom, Relative };
	Q_ENUM(VerticalPosition)

	enum class HorizontalAlignment { Left, Center, Right };
	Q_ENUM(HorizontalAlignment)
	enum class VerticalAlignment { Top, Center, Bottom };
	Q_ENUM(VerticalAlignment)

	enum class PositionLimit { None, X, Y };
	Q_ENUM(PositionLimit)

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

	enum class CoordinateSystemSource {Plot, Custom};

	BASIC_D_ACCESSOR_DEL(CoordinateSystemSource, coordinateSystemSource, CoordinateSystemSource)
	void setCoordinateSystem(const CartesianCoordinateSystem*, bool undo = true);
	const CartesianCoordinateSystem* coordinateSystem() const;
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
	virtual void updateLocale() { };
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
	int m_cSystemIndex{0}; // index of the coordinate system used from plot
	const CartesianCoordinateSystem* cSystem{nullptr}; // current cSystem

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
	virtual void handleAspectUpdated(const QString& path, const AbstractAspect*);

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
	void coordinateSystemChanged(const CartesianCoordinateSystem*) const;
	void coordinateSystemSourceChanged(CoordinateSystemSource) const;

	void objectPositionChanged(); // Position changed, independent of logical or scene, both are triggering this

	void hovered();
	void unhovered();

	void plotRangeListChanged();

	friend class WorksheetElementTest;
	friend class SetCoordinateSystemIndexCmd;
};

#endif
