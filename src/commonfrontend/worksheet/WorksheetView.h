/***************************************************************************
    File                 : WorksheetView.h
    Project              : LabPlot
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2016 by Alexander Semke (alexander.semke@web.de)

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

	enum ExportFormat {Pdf, Eps, Svg, Png};
	enum GridStyle {NoGrid, LineGrid, DotGrid};
	enum ExportArea {ExportBoundingBox, ExportSelection, ExportWorksheet};

	struct GridSettings {
		GridStyle style;
		QColor color;
		int horizontalSpacing;
		int verticalSpacing;
		float opacity;
	};

	void setScene(QGraphicsScene*);
	void exportToFile(const QString&, const ExportFormat, const ExportArea, const bool, const int);
	void setIsClosing();
	void setIsBeingPresented(bool presenting);
private:
	enum MouseMode {SelectionMode, NavigationMode, ZoomSelectionMode};
	enum CartesianPlotActionMode {ApplyActionToSelection, ApplyActionToAll};

	void initActions();
	void initMenus();
	void processResize();
	void drawForeground(QPainter*, const QRectF&);
	void drawBackground(QPainter*, const QRectF&);
	void drawBackgroundItems(QPainter*, const QRectF&);
	void exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect, const bool);
	void cartesianPlotAdd(CartesianPlot*, QAction*);

	//events
	void resizeEvent(QResizeEvent*);
	void contextMenuEvent(QContextMenuEvent*);
	void wheelEvent(QWheelEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);

	Worksheet* m_worksheet;
	MouseMode m_mouseMode;
	CartesianPlotActionMode m_cartesianPlotActionMode;
	CartesianPlot::MouseMode m_cartesianPlotMouseMode;
	bool m_selectionBandIsShown;
	QPoint m_selectionStart;
	QPoint m_selectionEnd;
	int magnificationFactor;
	QGraphicsPixmapItem* m_magnificationWindow;
	GridSettings m_gridSettings;
	QList<QGraphicsItem*> m_selectedItems;
	bool m_suppressSelectionChangedEvent;
	WorksheetElement* lastAddedWorksheetElement;
	QTimeLine* m_fadeInTimeLine;
	QTimeLine* m_fadeOutTimeLine;
	bool m_isClosing;
	bool m_isBeingPresented;
	//Menus
	QMenu* m_addNewMenu;
	QMenu* m_addNewCartesianPlotMenu;
	QMenu* m_zoomMenu;
	QMenu* m_magnificationMenu;
	QMenu* m_layoutMenu;
	QMenu* m_gridMenu;
	QMenu* m_viewMouseModeMenu;
	QMenu* m_cartesianPlotMenu;
	QMenu* m_cartesianPlotMouseModeMenu;
	QMenu* m_cartesianPlotAddNewMenu;
	QMenu* m_cartesianPlotZoomMenu;
	QMenu* m_cartesianPlotActionModeMenu;
	// Data manipulation menu
	QMenu* m_dataManipulationMenu;

	QToolButton* tbNewCartesianPlot;
	QToolButton* tbZoom;
	QToolButton* tbMagnification;
	QAction* currentZoomAction;
	QAction* currentMagnificationAction;

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

	QAction* showPresenterMode;
	//Actions for cartesian plots
	QAction* cartesianPlotApplyToSelectionAction;
	QAction* cartesianPlotApplyToAllAction;
	QAction* cartesianPlotSelectionModeAction;
	QAction* cartesianPlotZoomSelectionModeAction;
	QAction* cartesianPlotZoomXSelectionModeAction;
	QAction* cartesianPlotZoomYSelectionModeAction;

	QAction* addCurveAction;
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
	QAction* addHorizontalAxisAction;
	QAction* addVerticalAxisAction;
	QAction* addLegendAction;
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

public slots:
	void createContextMenu(QMenu*) const;
	void createAnalysisMenu(QMenu*) const;
	void fillToolBar(QToolBar*);
	void fillCartesianPlotToolBar(QToolBar*);
	void print(QPrinter*);
	void selectItem(QGraphicsItem*);

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

	void deselectItem(QGraphicsItem*);
	void selectionChanged();
	void updateBackground();
	void layoutChanged(Worksheet::Layout);

	void fadeIn(qreal);
	void fadeOut(qreal);

	void presenterMode();

	//SLOTs for cartesian plots
	void cartesianPlotActionModeChanged(QAction*);
	void cartesianPlotMouseModeChanged(QAction*);
	void cartesianPlotNavigationChanged(QAction*);
	void cartesianPlotAddNew(QAction*);
	void handleCartesianPlotActions();

signals:
	void statusInfo(const QString&);
};

#endif
