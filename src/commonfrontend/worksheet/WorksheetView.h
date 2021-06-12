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

#ifdef Q_OS_MAC
class KDMacTouchBar;
#endif

class WorksheetView : public QGraphicsView {
	Q_OBJECT

public:
	explicit WorksheetView(Worksheet* worksheet);

	enum class ExportFormat {PDF, SVG, PNG, JPG, BMP, PPM, XBM, XPM};
	enum class GridStyle {NoGrid, Line, Dot};
	enum class ExportArea {BoundingBox, Selection, Worksheet};

	struct GridSettings {
		GridStyle style;
		QColor color;
		int horizontalSpacing;
		int verticalSpacing;
		double opacity;
	};

	enum class MouseMode {Selection, Navigation, ZoomSelection};

	void setScene(QGraphicsScene*);
	void exportToFile(const QString&, const ExportFormat, const ExportArea, const bool, const int);
	void exportToClipboard(const ExportFormat, const ExportArea, const bool, const int);
	void exportToClipboard();
	void setIsClosing();
	void setIsBeingPresented(bool presenting);
	void setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode mode);
	void setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode mode);
	void setPlotLock(bool lock);
	void suppressSelectionChangedEvent(bool);
	WorksheetElement* selectedElement() const;

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
	void handleAxisSelected(const Axis* a);
	void handleCartesianPlotSelected();
	void handleXYCurveSelected();

	//events
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
	int magnificationFactor{0};
	QGraphicsPixmapItem* m_magnificationWindow{nullptr};
	GridSettings m_gridSettings;
	QList<QGraphicsItem*> m_selectedItems;
	WorksheetElement* m_selectedElement{nullptr}; // used to determine which range should be used for navigation
	bool m_suppressSelectionChangedEvent{false};
	WorksheetElement* lastAddedWorksheetElement{nullptr};
	QTimeLine* m_fadeInTimeLine{nullptr};
	QTimeLine* m_fadeOutTimeLine{nullptr};
	bool m_isClosing{false};
	bool m_actionsInitialized{false};
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
	QAction* selectAllAction{nullptr};
	QAction* deleteAction{nullptr};
	QAction* backspaceAction{nullptr};

	QAction* zoomInViewAction{nullptr};
	QAction* zoomOutViewAction{nullptr};
	QAction* zoomOriginAction{nullptr};
	QAction* zoomFitPageHeightAction{nullptr};
	QAction* zoomFitPageWidthAction{nullptr};
	QAction* zoomFitSelectionAction{nullptr};

	QAction* navigationModeAction{nullptr};
	QAction* zoomSelectionModeAction{nullptr};
	QAction* selectionModeAction{nullptr};

	QAction* addCartesianPlot1Action{nullptr};
	QAction* addCartesianPlot2Action{nullptr};
	QAction* addCartesianPlot3Action{nullptr};
	QAction* addCartesianPlot4Action{nullptr};
	QAction* addTextLabelAction{nullptr};
	QAction* addImageAction{nullptr};
	QAction* addGlobalInfoElementAction{nullptr};
	QAction* addHistogram{nullptr};

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

	QAction* plotsLockedAction{nullptr};
	QAction* showPresenterMode{nullptr};

	//Actions for cartesian plots
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

	QAction* addCurveAction{nullptr};
	QAction* addHistogramAction{nullptr};
	QAction* addEquationCurveAction{nullptr};
	QAction* addDataOperationCurveAction{nullptr};
	QAction* addDataReductionCurveAction{nullptr};
	QAction* addDifferentiationCurveAction{nullptr};
	QAction* addIntegrationCurveAction{nullptr};
	QAction* addInterpolationCurveAction{nullptr};
	QAction* addSmoothCurveAction{nullptr};
	QAction* addFitCurveAction{nullptr};
	QAction* addFourierFilterCurveAction{nullptr};
	QAction* addFourierTransformCurveAction{nullptr};
	QAction* addConvolutionCurveAction{nullptr};
	QAction* addCorrelationCurveAction{nullptr};

	QAction* addHorizontalAxisAction{nullptr};
	QAction* addVerticalAxisAction{nullptr};
	QAction* addLegendAction{nullptr};
	QAction* addPlotTextLabelAction{nullptr};
	QAction* addPlotImageAction{nullptr};
	QAction* addCustomPointAction{nullptr};

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

	// Analysis menu
	QAction* addDataOperationAction{nullptr};
	QAction* addDataReductionAction{nullptr};
	QAction* addDifferentiationAction{nullptr};
	QAction* addIntegrationAction{nullptr};
	QAction* addInterpolationAction{nullptr};
	QAction* addSmoothAction{nullptr};
	QAction* addFitAction{nullptr};
	QAction* addFourierFilterAction{nullptr};
	QAction* addFourierTransformAction{nullptr};
	QAction* addHilbertTransformAction{nullptr};
	QAction* addConvolutionAction{nullptr};
	QAction* addCorrelationAction{nullptr};

public slots:
	void createContextMenu(QMenu*);
	void createAnalysisMenu(QMenu*);
	void fillToolBar(QToolBar*);
#ifdef Q_OS_MAC
	void fillTouchBar(KDMacTouchBar*);
#endif
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
	void propertiesExplorerRequested();
};

#endif
