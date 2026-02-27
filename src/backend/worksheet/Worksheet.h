/*
	File                 : Worksheet.h
	Project              : LabPlot
	Description          : Worksheet (2D visualization) part
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEET_H
#define WORKSHEET_H

#include "backend/core/AbstractPart.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

class QGraphicsItem;
class QGraphicsScene;
class QRectF;

class Background;
class WorksheetPrivate;
class WorksheetView;
class TreeModel;
class XYCurve;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Worksheet : public AbstractPart {
#else
class Worksheet : public AbstractPart {
#endif
	Q_OBJECT

public:
	explicit Worksheet(const QString& name, bool loading = false);
	~Worksheet() override;

	enum class Unit { Millimeter, Centimeter, Inch, Point };
	enum class Layout { NoLayout, VerticalLayout, HorizontalLayout, GridLayout };
	enum class CartesianPlotActionMode {
		ApplyActionToSelection, // Apply to only the selected plot
		ApplyActionToAll, // Apply action to all plots for all dimensions
		ApplyActionToAllX, // Apply action to all plots, but only for the x ranges
		ApplyActionToAllY // Apply action to all plots, but only for the y ranges
	};
	enum class ZoomFit { None, Fit, FitToHeight, FitToWidth, FitToSelection };
	enum class ExportFormat { PDF, SVG, PNG, JPG, BMP, PPM, XBM, XPM };
	enum class ExportArea { BoundingBox, Selection, Worksheet };

	static double convertToSceneUnits(const double value, const Worksheet::Unit unit);
	static double convertFromSceneUnits(const double value, const Worksheet::Unit unit);

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	void fillElementsContextMenu(QMenu*);
	QWidget* view() const override;

	QVector<AbstractAspect*> dependsOn() const override;
	WorksheetElement* currentSelection();
	QVector<AspectType> pasteTypes() const override;

	bool exportToFile(const QString&,
					  const ExportFormat,
					  const ExportArea area = ExportArea::Worksheet,
					  const bool background = true,
					  const int resolution = 100) const;
	bool exportView() const override;
	bool exportView(QPixmap&) const;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	QRectF pageRect() const;
	void setPageRect(const QRectF&);
	QGraphicsScene* scene() const;
	double zoomFactor() const;
	void update();
	void setPrinting(bool) const;

	void setItemSelectedInView(const QGraphicsItem*, const bool);
	void setSelectedInView(const bool);
	void deleteAspectFromGraphicsItem(const QGraphicsItem*);
	void setIsClosing();
	void suppressSelectionChangedEvent(bool);

	CartesianPlotActionMode cartesianPlotActionMode() const;
	void setCartesianPlotActionMode(CartesianPlotActionMode mode);
	CartesianPlotActionMode cartesianPlotCursorMode() const;
	void setCartesianPlotCursorMode(CartesianPlotActionMode mode);
	void setInteractive(bool);
	void setPlotsInteractive(bool);
	bool plotsInteractive() const;
	int plotCount();
	CartesianPlot* plot(int index);
	TreeModel* cursorModel();

	void cursorModelPlotAdded(const QString& name);
	void cursorModelPlotRemoved(const QString& name);

	Background* background() const;
	BASIC_D_ACCESSOR_DECL(bool, scaleContent, ScaleContent)
	BASIC_D_ACCESSOR_DECL(bool, useViewSize, UseViewSize)
	BASIC_D_ACCESSOR_DECL(ZoomFit, zoomFit, ZoomFit)
	BASIC_D_ACCESSOR_DECL(Worksheet::Layout, layout, Layout)
	BASIC_D_ACCESSOR_DECL(double, layoutTopMargin, LayoutTopMargin)
	BASIC_D_ACCESSOR_DECL(double, layoutBottomMargin, LayoutBottomMargin)
	BASIC_D_ACCESSOR_DECL(double, layoutLeftMargin, LayoutLeftMargin)
	BASIC_D_ACCESSOR_DECL(double, layoutRightMargin, LayoutRightMargin)
	BASIC_D_ACCESSOR_DECL(double, layoutHorizontalSpacing, LayoutHorizontalSpacing)
	BASIC_D_ACCESSOR_DECL(double, layoutVerticalSpacing, LayoutVerticalSpacing)
	BASIC_D_ACCESSOR_DECL(int, layoutRowCount, LayoutRowCount)
	BASIC_D_ACCESSOR_DECL(int, layoutColumnCount, LayoutColumnCount)

	QString theme() const;

	void setSuppressLayoutUpdate(bool);
	void updateLayout();

	void registerShortcuts() override;
	void unregisterShortcuts() override;

	typedef WorksheetPrivate Private;

public Q_SLOTS:
	void setTheme(const QString&);
	void cartesianPlotAxisShift(int delta, Dimension dim, int index);
	void cartesianPlotWheelEvent(const QPointF& sceneRelPos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim);
	void cartesianPlotMousePressZoomSelectionMode(QPointF logicPos);
	void cartesianPlotMousePressCursorMode(int cursorNumber, QPointF logicPos);
	void cartesianPlotMouseMoveZoomSelectionMode(QPointF logicPos);
	void cartesianPlotMouseMoveSelectionMode(QPointF logicStart, QPointF logicEnd);
	void cartesianPlotMouseMoveCursorMode(int cursorNumber, QPointF logicPos);
	void cartesianPlotMouseReleaseZoomSelectionMode();
	void cartesianPlotMouseHoverZoomSelectionMode(QPointF logicPos);
	void cartesianPlotMouseHoverOutsideDataRect();
	void cartesianPlotMouseModeChangedSlot(CartesianPlot::MouseMode);

	// slots needed by the cursor
	void updateCurveBackground(QColor, const QString& curveName);
	void updateCompleteCursorTreeModel();
	void cursorPosChanged(int cursorNumber, double xPos);
	void curveDataChanged(const XYCurve* curve);

private:
	void init();
	WorksheetElement* aspectFromGraphicsItem(const WorksheetElement*, const QGraphicsItem*) const;
	void loadTheme(const QString&);

	Q_DECLARE_PRIVATE(Worksheet)
	WorksheetPrivate* const d_ptr;
	mutable WorksheetView* m_view{nullptr};
	friend class WorksheetPrivate;

private Q_SLOTS:
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void handleAspectMoved();
	void changeSelectedVisibility();

	void childSelected(const AbstractAspect*) override;
	void childDeselected(const AbstractAspect*) override;

Q_SIGNALS:
	void requestProjectContextMenu(QMenu*);
	void itemSelected(QGraphicsItem*);
	void itemDeselected(QGraphicsItem*);
	void requestUpdate();
	void cartesianPlotMouseModeChanged(CartesianPlot::MouseMode);
	void showCursorDock(TreeModel*, QVector<CartesianPlot*>);
	void propertiesExplorerRequested();
	void childContextMenuRequested(AspectType, QMenu*);

	void scaleContentChanged(bool);
	void useViewSizeChanged(bool);
	void pageRectChanged(const QRectF&);

	void layoutChanged(Worksheet::Layout);
	void layoutTopMarginChanged(double);
	void layoutBottomMarginChanged(double);
	void layoutLeftMarginChanged(double);
	void layoutRightMarginChanged(double);
	void layoutVerticalSpacingChanged(double);
	void layoutHorizontalSpacingChanged(double);
	void layoutRowCountChanged(int);
	void layoutColumnCountChanged(int);

	void changed();
	void themeChanged(const QString&);

	friend class WorksheetTest;
};

#endif
