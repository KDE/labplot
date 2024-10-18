/*
	File                 : WorksheetView.h
	Project              : LabPlot
	Description          : Worksheet view
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2023 Alexander Semke <alexander.semke@web.de>
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

class AbstractAspect;
class WorksheetElement;

#ifdef HAVE_TOUCHBAR
class KDMacTouchBar;
#endif

class WorksheetView : public QGraphicsView {
	Q_OBJECT

public:
	explicit WorksheetView(Worksheet* worksheet);

	enum class ExportFormat { PDF, SVG, PNG, JPG, BMP, PPM, XBM, XPM };
	enum class GridStyle { NoGrid, Line, Dot };
	enum class ExportArea { BoundingBox, Selection, Worksheet };

	struct GridSettings {
		GridStyle style;
		QColor color;
		int horizontalSpacing;
		int verticalSpacing;
		double opacity;
	};

	enum class MouseMode { Selection, Navigation, ZoomSelection };

	void setScene(QGraphicsScene*);
	void exportToFile(const QString&, const ExportFormat, const ExportArea, const bool, const int);
	void exportToClipboard();
	void exportToPixmap(QPixmap& pixmap);
	void setIsClosing();
	void setIsBeingPresented(bool presenting);
	void setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode);
	void setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode);
	void setPlotInteractive(bool);
	void suppressSelectionChangedEvent(bool);
	WorksheetElement* selectedElement() const;
	QList<QGraphicsItem*> selectedItems() const;
	double zoomFactor() const;
	void processResize();

	Worksheet::CartesianPlotActionMode getCartesianPlotActionMode() const;
	void registerShortcuts();
	void unregisterShortcuts();
	void initActions();
	void initPlotNavigationActions();

private:
	void initBasicActions();
	void initMenus();
	void drawForeground(QPainter*, const QRectF&) override;
	void drawBackground(QPainter*, const QRectF&) override;
	void drawBackgroundItems(QPainter*, const QRectF&);
	CartesianPlot* plotAt(QPoint) const;
	void exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect, const bool background, const bool selection = false);
	void cartesianPlotAdd(CartesianPlot*, QAction*);
	void handleAxisSelected(const Axis*);
	void handleCartesianPlotSelected(CartesianPlot*);
	void handlePlotSelected();
	void handleReferenceLineSelected();
	void handleReferenceRangeSelected();
	void handleReferences(bool vertical);
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

	Worksheet* m_worksheet;
	MouseMode m_mouseMode{MouseMode::Selection};
	CartesianPlot::MouseMode m_cartesianPlotMouseMode{CartesianPlot::MouseMode::Selection};
	bool m_selectionBandIsShown{false};
	QPoint m_selectionStart;
	QPoint m_selectionEnd;
	QPointF m_cursorPos;
	bool m_calledFromContextMenu{false};
	int magnificationFactor{0};
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
	bool m_plotActionsInitialized{false};
	bool m_menusInitialized{false};
	int m_numScheduledScalings{0};
	bool m_suppressMouseModeChange{false};

	// Menus
	QMenu* m_addNewMenu{nullptr};
	QMenu* m_addNewCartesianPlotMenu{nullptr};
	QMenu* m_zoomMenu{nullptr};
	QMenu* m_magnificationMenu{nullptr};
	QMenu* m_layoutMenu{nullptr};
	QMenu* m_gridMenu{nullptr};
	QMenu* m_themeMenu{nullptr};
	QMenu* m_viewMouseModeMenu{nullptr};
	QMenu* m_cartesianPlotMenu{nullptr};
	QMenu* m_cartesianPlotMouseModeMenu{nullptr};
	QMenu* m_cartesianPlotAddNewMenu{nullptr};
	QMenu* m_cartesianPlotZoomMenu{nullptr};
	QMenu* m_cartesianPlotActionModeMenu{nullptr};
	QMenu* m_cartesianPlotCursorModeMenu{nullptr};

	QToolButton* tbCartesianPlotAddNew{nullptr};
	QToolButton* tbNewCartesianPlot{nullptr};
	QToolButton* tbZoom{nullptr};
	QToolButton* tbMagnification{nullptr};
	QAction* currentZoomAction{nullptr};
	QAction* currentMagnificationAction{nullptr};

	// Actions
	QAction* selectAllAction{nullptr};
	QAction* deleteAction{nullptr};
	QAction* backspaceAction{nullptr};

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
	QAction* addGlobalInfoElementAction{nullptr};

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

	QAction* noMagnificationAction{nullptr};
	QAction* twoTimesMagnificationAction{nullptr};
	QAction* threeTimesMagnificationAction{nullptr};
	QAction* fourTimesMagnificationAction{nullptr};
	QAction* fiveTimesMagnificationAction{nullptr};

	QAction* plotsInteractiveAction{nullptr};
	QAction* showPresenterMode{nullptr};

	// Actions for cartesian plots
	QAction* cartesianPlotAddNewAction{nullptr};
	QAction* cartesianPlotApplyToSelectionAction{nullptr};
	QAction* cartesianPlotApplyToAllAction{nullptr};
	QAction* cartesianPlotApplyToAllXAction{nullptr};
	QAction* cartesianPlotApplyToAllYAction{nullptr};
	QAction* cartesianPlotApplyToAllCursor{nullptr};
	QAction* cartesianPlotApplyToSelectionCursor{nullptr};
	QAction* cartesianPlotSelectionModeAction{nullptr};
	QAction* cartesianPlotCrosshairModeAction{nullptr};
	QAction* cartesianPlotZoomSelectionModeAction{nullptr};
	QAction* cartesianPlotZoomXSelectionModeAction{nullptr};
	QAction* cartesianPlotZoomYSelectionModeAction{nullptr};
	QAction* cartesianPlotCursorModeAction{nullptr};

	QAction* scaleAutoXAction{nullptr};
	QAction* scaleAutoYAction{nullptr};
	QAction* scaleAutoAction{nullptr};
	QAction* zoomInAction{nullptr};
	QAction* zoomOutAction{nullptr};
	QAction* zoomInXAction{nullptr};
	QAction* zoomOutXAction{nullptr};
	QAction* zoomInYAction{nullptr};
	QAction* zoomOutYAction{nullptr};
	QAction* shiftLeftXAction{nullptr};
	QAction* shiftRightXAction{nullptr};
	QAction* shiftUpYAction{nullptr};
	QAction* shiftDownYAction{nullptr};

public Q_SLOTS:
	void createContextMenu(QMenu*);
	void fillToolBar(QToolBar*);
#ifdef HAVE_TOUCHBAR
	void fillTouchBar(KDMacTouchBar*);
#endif
	void fillCartesianPlotToolBar(QToolBar*);
	void fillCartesianPlotNavigationToolBar(QToolBar*, bool enableCursor = true) const;
	void print(QPrinter*);
	void selectItem(QGraphicsItem*);
	void presenterMode();
	void cartesianPlotMouseModeChangedSlot(CartesianPlot::MouseMode); // from cartesian plot
	void childContextMenuRequested(AspectType, QMenu*);
	void cartesianPlotMouseModeChanged(QAction*);

private Q_SLOTS:
	void addNew(QAction*);
	void aspectAboutToBeRemoved(const AbstractAspect*);
	void selectAllElements();
	void deleteElement();

	void mouseModeChanged(QAction*);
	void useViewSizeChanged(bool);
	void changeZoom(QAction*);
	void fitChanged(QAction*);
	void updateFit();
	void magnificationChanged(QAction*);
	void changeLayout(QAction*);
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
	void cartesianPlotNavigationChanged(QAction*);
	void handleCartesianPlotActions();

Q_SIGNALS:
	void statusInfo(const QString&);
	void propertiesExplorerRequested();

	friend class RetransformTest;
	friend class MultiRangeTest2;
	friend class CartesianPlotTest;
};

#endif
