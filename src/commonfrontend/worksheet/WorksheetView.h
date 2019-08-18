/***************************************************************************
    File                 : WorksheetView.h
    Project              : LabPlot
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2019 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2018 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef WORKSHEETVIEW_H
#define WORKSHEETVIEW_H

#include <QGraphicsView>
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

class QPrinter;
class QMenu;
class QToolBar;
class QToolButton;
class QWheelEvent;
class QTimeLine;

class AbstractAspect;
class WorksheetElement;

class WorksheetView : public QGraphicsView {
	Q_OBJECT

public:
	explicit WorksheetView(Worksheet* worksheet);

	enum ExportFormat {Pdf, Svg, Png};
	enum GridStyle {NoGrid, LineGrid, DotGrid};
	enum ExportArea {ExportBoundingBox, ExportSelection, ExportWorksheet};

	struct GridSettings {
		GridStyle style;
		QColor color;
		int horizontalSpacing;
		int verticalSpacing;
		double opacity;
	};

	enum MouseMode {SelectionMode, NavigationMode, ZoomSelectionMode};

	void setScene(QGraphicsScene*);
	void exportToFile(const QString&, const ExportFormat, const ExportArea, const bool, const int);
	void exportToClipboard();
	void setIsClosing();
	void setIsBeingPresented(bool presenting);
	void setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode mode);
	void setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode mode);
	void setPlotLock(bool lock);

	Worksheet::CartesianPlotActionMode getCartesianPlotActionMode();
	void registerShortcuts();
	void unregisterShortcuts();
private:

	void initBasicActions();
	void initActions();
	void initMenus();
	void processResize();
	void drawForeground(QPainter*, const QRectF&) override;
	void drawBackground(QPainter*, const QRectF&) override;
	void drawBackgroundItems(QPainter*, const QRectF&);
	bool isPlotAtPos(QPoint) const;
	CartesianPlot* plotAt(QPoint) const;
	void exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect, const bool);
	void cartesianPlotAdd(CartesianPlot*, QAction*);

	//events
	void resizeEvent(QResizeEvent*) override;
	void contextMenuEvent(QContextMenuEvent*) override;
	void wheelEvent(QWheelEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;
	void dragEnterEvent(QDragEnterEvent*) override;
	void dragMoveEvent(QDragMoveEvent*) override;
	void dropEvent(QDropEvent*) override;

	Worksheet* m_worksheet;
	MouseMode m_mouseMode{SelectionMode};
	CartesianPlot::MouseMode m_cartesianPlotMouseMode{CartesianPlot::SelectionMode};
	bool m_selectionBandIsShown{false};
	QPoint m_selectionStart;
	QPoint m_selectionEnd;
	int magnificationFactor{0};
	QGraphicsPixmapItem* m_magnificationWindow{nullptr};
	GridSettings m_gridSettings;
	QList<QGraphicsItem*> m_selectedItems;
	bool m_suppressSelectionChangedEvent{false};
	WorksheetElement* lastAddedWorksheetElement{nullptr};
	QTimeLine* m_fadeInTimeLine{nullptr};
	QTimeLine* m_fadeOutTimeLine{nullptr};
	bool m_isClosing{false};
	bool m_menusInitialized{false};
	int m_numScheduledScalings{0};
	bool m_suppressMouseModeChange{false};

	//Menus
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
	QMenu* m_cartesianPlotAddNewAnalysisMenu{nullptr};
	QMenu* m_cartesianPlotZoomMenu{nullptr};
	QMenu* m_cartesianPlotActionModeMenu{nullptr};
	QMenu* m_cartesianPlotCursorModeMenu{nullptr};
	QMenu* m_dataManipulationMenu{nullptr};

	QToolButton* tbNewCartesianPlot{nullptr};
	QToolButton* tbZoom{nullptr};
	QToolButton* tbMagnification{nullptr};
	QAction* currentZoomAction{nullptr};
	QAction* currentMagnificationAction{nullptr};

	//Actions
	QAction* selectAllAction;
	QAction* deleteAction;
	QAction* backspaceAction;

	QAction* zoomInViewAction;
	QAction* zoomOutViewAction;
	QAction* zoomOriginAction;
	QAction* zoomFitPageHeightAction;
	QAction* zoomFitPageWidthAction;
	QAction* zoomFitSelectionAction;

	QAction* navigationModeAction;
	QAction* zoomSelectionModeAction;
	QAction* selectionModeAction;

	QAction* addCartesianPlot1Action;
	QAction* addCartesianPlot2Action;
	QAction* addCartesianPlot3Action;
	QAction* addCartesianPlot4Action;
	QAction* addTextLabelAction;
	QAction* addHistogram;
	QAction* addBarChartPlot;

	QAction* verticalLayoutAction;
	QAction* horizontalLayoutAction;
	QAction* gridLayoutAction;
	QAction* breakLayoutAction;

	QAction* noGridAction;
	QAction* denseLineGridAction;
	QAction* sparseLineGridAction;
	QAction* denseDotGridAction;
	QAction* sparseDotGridAction;
	QAction* customGridAction;
	QAction* snapToGridAction;

	QAction* noMagnificationAction;
	QAction* twoTimesMagnificationAction;
	QAction* threeTimesMagnificationAction;
	QAction* fourTimesMagnificationAction;
	QAction* fiveTimesMagnificationAction;

	QAction* plotsLockedAction;
	QAction* showPresenterMode;

	//Actions for cartesian plots
	QAction* cartesianPlotApplyToSelectionAction;
	QAction* cartesianPlotApplyToAllAction;
	QAction* cartesianPlotApplyToAllCursor;
	QAction* cartesianPlotApplyToSelectionCursor;
	QAction* cartesianPlotSelectionModeAction;
	QAction* cartesianPlotZoomSelectionModeAction;
	QAction* cartesianPlotZoomXSelectionModeAction;
	QAction* cartesianPlotZoomYSelectionModeAction;
	QAction* cartesianPlotCursorModeAction;

	QAction* addCurveAction;
	QAction* addHistogramAction;
	QAction* addEquationCurveAction;
	QAction* addDataOperationCurveAction;
	QAction* addDataReductionCurveAction;
	QAction* addDifferentiationCurveAction;
	QAction* addIntegrationCurveAction;
	QAction* addInterpolationCurveAction;
	QAction* addSmoothCurveAction;
	QAction* addFitCurveAction;
	QAction* addFourierFilterCurveAction;
	QAction* addFourierTransformCurveAction;
	QAction* addConvolutionCurveAction;
	QAction* addCorrelationCurveAction;

	QAction* addHorizontalAxisAction;
	QAction* addVerticalAxisAction;
	QAction* addLegendAction;
	QAction* addPlotTextLabelAction;
	QAction* addCustomPointAction;

	QAction* scaleAutoXAction;
	QAction* scaleAutoYAction;
	QAction* scaleAutoAction;
	QAction* zoomInAction;
	QAction* zoomOutAction;
	QAction* zoomInXAction;
	QAction* zoomOutXAction;
	QAction* zoomInYAction;
	QAction* zoomOutYAction;
	QAction* shiftLeftXAction;
	QAction* shiftRightXAction;
	QAction* shiftUpYAction;
	QAction* shiftDownYAction;

	// Analysis menu
	QAction* addDataOperationAction;
	QAction* addDataReductionAction;
	QAction* addDifferentiationAction;
	QAction* addIntegrationAction;
	QAction* addInterpolationAction;
	QAction* addSmoothAction;
	QAction* addFitAction;
	QAction* addFourierFilterAction;
	QAction* addFourierTransformAction;
	QAction* addConvolutionAction;
	QAction* addCorrelationAction;

public slots:
	void createContextMenu(QMenu*);
	void createAnalysisMenu(QMenu*);
	void fillToolBar(QToolBar*);
	void fillCartesianPlotToolBar(QToolBar*);
	void print(QPrinter*);
	void selectItem(QGraphicsItem*);
	void presenterMode();
	void cartesianPlotMouseModeChangedSlot(CartesianPlot::MouseMode mouseMode); // from cartesian Plot

private slots:
	void addNew(QAction*);
	void aspectAboutToBeRemoved(const AbstractAspect*);
	void selectAllElements();
	void deleteElement();

	void mouseModeChanged(QAction*);
	void useViewSizeRequested();
	void changeZoom(QAction*);
	void magnificationChanged(QAction*);
	void changeLayout(QAction*);
	void changeGrid(QAction*);
	void changeSnapToGrid();
	void plotsLockedActionChanged(bool checked);

	void deselectItem(QGraphicsItem*);
	void selectionChanged();
	void updateBackground();
	void layoutChanged(Worksheet::Layout);

	void fadeIn(qreal);
	void fadeOut(qreal);

	void zoom(int);
	void scalingTime();
	void animFinished();

	//SLOTs for cartesian plots
	void cartesianPlotActionModeChanged(QAction*);
	void cartesianPlotCursorModeChanged(QAction*);
	void cartesianPlotMouseModeChanged(QAction*);
	void cartesianPlotNavigationChanged(QAction*);
	void cartesianPlotAddNew(QAction*);
	void handleCartesianPlotActions();

signals:
	void statusInfo(const QString&);
};

#endif
