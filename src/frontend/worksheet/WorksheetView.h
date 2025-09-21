/*
	File                 : WorksheetView.h
	Project              : LabPlot
	Description          : Worksheet view
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETVIEW_H
#define WORKSHEETVIEW_H

#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include <QGraphicsView>

class QPrinter;
class QMenu;
class QToolBar;
class QToolButton;
class QWheelEvent;
class QTimeLine;

class ToggleActionMenu;
class WorksheetElement;

#ifdef HAVE_TOUCHBAR
class KDMacTouchBar;
#endif

class WorksheetView : public QGraphicsView {
	Q_OBJECT

public:
	explicit WorksheetView(Worksheet*);

	enum class GridStyle { NoGrid, Line, Dot };
	enum class ExportArea { BoundingBox, Selection, Worksheet };
	enum class ZoomMode { ZoomIn, ZoomOut, ZoomOrigin };
	enum class MouseMode { Selection, Navigation, ZoomSelection };
	enum class AddNewMode { PlotAreaFourAxes, PlotAreaTwoAxes, PlotAreaTwoAxesCentered, PlotAreaTwoAxesCenteredZero, PlotAreaFromTemplate, TextLabel, Image};

	struct GridSettings {
		GridStyle style;
		QColor color;
		int horizontalSpacing;
		int verticalSpacing;
		double opacity;
	};

	void setScene(QGraphicsScene*);
	bool exportToFile(const QString&, const Worksheet::ExportFormat, const Worksheet::ExportArea, const bool, const int);
	void exportToClipboard();
	void exportToPixmap(QPixmap&);
	void setIsClosing();
	void setIsBeingPresented(bool);
	void setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode);
	Worksheet::CartesianPlotActionMode getCartesianPlotActionMode() const;
	void setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode);
	void setPlotInteractive(bool);
	void suppressSelectionChangedEvent(bool);
	WorksheetElement* selectedElement() const;
	QList<QGraphicsItem*> selectedItems() const;
	double zoomFactor() const;
	void processResize();

	void registerShortcuts();
	void unregisterShortcuts();

	void initActions();
	void initPlotNavigationActions();
	void registerCartesianPlotActions(QActionGroup* mouseModeActionGroup, QActionGroup* navigationActionGroup);

	Worksheet::Layout layout() const;
	MouseMode mouseMode() const;
	ZoomMode zoomMode() const;
	AddNewMode addNewMode() const;
	int magnification() const;

	void fillAddNewPlotMenu(ToggleActionMenu*) const;
	void fillZoomMenu(ToggleActionMenu*) const;
	void fillMagnificationMenu(ToggleActionMenu*) const;
	void fillCartesianPlotNavigationToolBar(QToolBar*);
	QMenu* plotAddNewMenu() const;

private:
	void initBasicActions();
	void initMenus();
	void drawForeground(QPainter*, const QRectF&) override;
	void drawBackground(QPainter*, const QRectF&) override;
	void drawBackgroundItems(QPainter*, const QRectF&);
	CartesianPlot* plotAt(QPoint) const;
	void exportPaint(QPainter*, const QRectF& targetRect, const QRectF& sourceRect, const bool background, const bool selection = false);
	void cartesianPlotAdd(CartesianPlot*, QAction*);

	void updateCartesianPlotActions();
	void updateCartesianPlotActions(const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup);
	void handleAxisSelected(const Axis*, const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup);
	void handleCartesianPlotSelected(const CartesianPlot*, const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup);
	void handlePlotSelected(const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup);
	void handleReferences(WorksheetElement::Orientation, const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup);

	bool eventFilter(QObject* watched, QEvent*) override;
	void updateLabelsZoom() const;
	void updateScrollBarPolicy();
	void updateMagnificationWindow(const QPointF& pos);

	// events
	void resizeEvent(QResizeEvent*) override;
	void contextMenuEvent(QContextMenuEvent*) override;
	void wheelEvent(QWheelEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseDoubleClickEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;
	void dragEnterEvent(QDragEnterEvent*) override;
	void dragMoveEvent(QDragMoveEvent*) override;
	void dropEvent(QDropEvent*) override;

	Worksheet* m_worksheet{nullptr};
	MouseMode m_mouseMode{MouseMode::Selection};
	ZoomMode m_zoomMode{ZoomMode::ZoomIn};
	AddNewMode m_addNewMode{AddNewMode::PlotAreaFourAxes};
	CartesianPlot::MouseMode m_cartesianPlotMouseMode{CartesianPlot::MouseMode::Selection};
	bool m_selectionBandIsShown{false};
	QPoint m_selectionStart;
	QPoint m_selectionEnd;
	QPointF m_cursorPos;
	bool m_calledFromContextMenu{false};
	int m_magnificationFactor{0};
	QGraphicsPixmapItem* m_magnificationWindow{nullptr};
	GridSettings m_gridSettings;
	QList<QGraphicsItem*> m_selectedItems;
	WorksheetElement* m_selectedElement{nullptr}; // used to determine which range should be used for navigation
	bool m_suppressSelectionChangedEvent{false};
	WorksheetElement* lastAddedWorksheetElement{nullptr};
	QTimeLine* m_fadeInTimeLine{nullptr};
	QTimeLine* m_fadeOutTimeLine{nullptr};
	QTimeLine* m_zoomTimeLine{nullptr};
	bool m_isClosing{false};
	bool m_isPrinting{false};
	bool m_actionsInitialized{false};
	bool m_menusInitialized{false};
	int m_numScheduledScalings{0};
	bool m_suppressMouseModeChange{false};

	// Menus
	QMenu* m_addNewMenu{nullptr};
	QMenu* m_zoomMenu{nullptr};
	QMenu* m_magnificationMenu{nullptr};
	QMenu* m_layoutMenu{nullptr};
	QMenu* m_gridMenu{nullptr};
	QMenu* m_themeMenu{nullptr};
	QMenu* m_viewMouseModeMenu{nullptr};

	// Menus for cartesan plot
	QMenu* m_addNewPlotMenu{nullptr};
	QMenu* m_plotMenu{nullptr};
	QMenu* m_plotMouseModeMenu{nullptr};
	QMenu* m_plotZoomMenu{nullptr};
	QMenu* m_plotActionModeMenu{nullptr};
	QMenu* m_plotCursorModeMenu{nullptr};
	QToolButton* tbNewPlot{nullptr};

	// Actions
	QAction* selectAllAction{nullptr};
	QAction* deleteAction{nullptr};
	QAction* backspaceAction{nullptr};

	QActionGroup* zoomActionGroup{nullptr};
	QAction* zoomInViewAction{nullptr};
	QAction* zoomOutViewAction{nullptr};
	QAction* zoomOriginAction{nullptr};
	QAction* zoomFitNoneAction{nullptr};
	QAction* zoomFitPageHeightAction{nullptr};
	QAction* zoomFitPageWidthAction{nullptr};
	QAction* zoomFitSelectionAction{nullptr};
	QAction* zoomFitAction{nullptr};

	QAction* navigationModeAction{nullptr};
	QAction* zoomSelectionModeAction{nullptr};
	QAction* selectionModeAction{nullptr};

	QAction* addCartesianPlot1Action{nullptr};
	QAction* addCartesianPlot2Action{nullptr};
	QAction* addCartesianPlot3Action{nullptr};
	QAction* addCartesianPlot4Action{nullptr};
	QAction* addCartesianPlotTemplateAction{nullptr};
	QAction* addTextLabelAction{nullptr};
	QAction* addImageAction{nullptr};

	QAction* verticalLayoutAction{nullptr};
	QAction* horizontalLayoutAction{nullptr};
	QAction* gridLayoutAction{nullptr};
	QAction* breakLayoutAction{nullptr};

	QAction* noGridAction{nullptr};
	QAction* denseLineGridAction{nullptr};
	QAction* sparseLineGridAction{nullptr};
	QAction* denseDotGridAction{nullptr};
	QAction* sparseDotGridAction{nullptr};
	QAction* customGridAction{nullptr};
	QAction* snapToGridAction{nullptr};

	QActionGroup* magnificationActionGroup{nullptr};

	QAction* plotsInteractiveAction{nullptr};
	QAction* showPresenterMode{nullptr};

	// Actions for cartesian plots
	QAction* plotApplyToSelectionAction{nullptr};
	QAction* plotApplyToAllAction{nullptr};
	QAction* plotApplyToAllXAction{nullptr};
	QAction* plotApplyToAllYAction{nullptr};
	QAction* plotApplyToAllCursor{nullptr};
	QAction* plotApplyToSelectionCursor{nullptr};

	QActionGroup* m_plotMouseModeActionGroup{nullptr};
	QActionGroup* m_plotMouseModeActionGroupExternal{nullptr};
	QAction* plotSelectionModeAction{nullptr};
	QAction* plotCrosshairModeAction{nullptr};
	QAction* plotZoomSelectionModeAction{nullptr};
	QAction* plotZoomXSelectionModeAction{nullptr};
	QAction* plotZoomYSelectionModeAction{nullptr};
	QAction* plotCursorModeAction{nullptr};

	QActionGroup* m_plotNavigationActionGroup{nullptr};
	QActionGroup* m_plotNavigationActionGroupExternal{nullptr};
	QAction* plotScaleAutoXAction{nullptr};
	QAction* plotScaleAutoYAction{nullptr};
	QAction* plotScaleAutoAction{nullptr};
	QAction* plotZoomInAction{nullptr};
	QAction* plotZoomOutAction{nullptr};
	QAction* plotZoomInXAction{nullptr};
	QAction* plotZoomOutXAction{nullptr};
	QAction* plotZoomInYAction{nullptr};
	QAction* plotZoomOutYAction{nullptr};
	QAction* plotShiftLeftXAction{nullptr};
	QAction* plotShiftRightXAction{nullptr};
	QAction* plotShiftUpYAction{nullptr};
	QAction* plotShiftDownYAction{nullptr};

public Q_SLOTS:
	void createContextMenu(QMenu*);
#ifdef HAVE_TOUCHBAR
	void fillTouchBar(KDMacTouchBar*);
#endif
	void print(QPrinter*);
	void selectItem(QGraphicsItem*);
	void presenterMode();
	void cartesianPlotMouseModeChangedSlot(CartesianPlot::MouseMode); // from cartesian plot
	void childContextMenuRequested(AspectType, QMenu*);

	void addNew(QAction*);
	void changeLayout(QAction*) const;
	void changeMouseMode(QAction*);
	void changeZoom(QAction*);
	void changeZoomFit(QAction*);
	void changeMagnification(QAction*);
	void changePlotMouseMode(QAction*);
	void changePlotNavigation(QAction*);

private Q_SLOTS:
	void aspectAboutToBeRemoved(const AbstractAspect*);
	void selectAllElements();
	void deleteElement();

	void useViewSizeChanged(bool);
	void updateFit();
	void changeGrid(QAction*);
	void changeSnapToGrid();
	void plotsInteractiveActionChanged(bool checked);

	void deselectItem(QGraphicsItem*);
	void selectionChanged();
	void updateBackground();
	void layoutChanged(Worksheet::Layout);

	void fadeIn(qreal);
	void fadeOut(qreal);

	void zoom(int);
	void scalingTime();
	void animFinished();

	// SLOTs for cartesian plots
	void cartesianPlotActionModeChanged(QAction*);
	void cartesianPlotCursorModeChanged(QAction*);

Q_SIGNALS:
	void statusInfo(const QString&);
	void propertiesExplorerRequested();

	friend class RetransformTest;
	friend class MultiRangeTest;
	friend class CartesianPlotTest;
};

#endif
