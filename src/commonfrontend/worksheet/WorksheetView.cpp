/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2009-2015 Alexander Semke (alexander.semke@web.de)

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
#include "commonfrontend/worksheet/WorksheetView.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetElementGroup.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/3d/Plot3D.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/TextLabel.h"
#include "kdefrontend/worksheet/GridDialog.h"

#include <QApplication>
#include <QMenu>
#include <QToolBar>
#include <QDesktopWidget>
#include <QWheelEvent>
#include <QPrinter>
#include <QSvgGenerator>
#include <QImage>
#include <QToolButton>
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>
#include <QTimeLine>

#include <KAction>
#include <KLocale>
#include <KMessageBox>

#include <QVTKWidget2.h>
#include <vtkGenericOpenGLRenderWindow.h>

/**
 * \class WorksheetView
 * \brief Worksheet view
 */

/*!
  Constructur of the class.
  Creates a view for the Worksheet \c worksheet and initializes the internal model.
*/
WorksheetView::WorksheetView(Worksheet* worksheet, QGLContext* glContext) : QGraphicsView(),
	m_worksheet(worksheet),
	m_mouseMode(SelectionMode),
	m_cartesianPlotActionMode(ApplyActionToSelection),
	m_cartesianPlotMouseMode(CartesianPlot::SelectionMode),
	m_selectionBandIsShown(false),
	m_suppressSelectionChangedEvent(false),
	lastAddedWorksheetElement(0),
	m_fadeInTimeLine(0),
	m_fadeOutTimeLine(0),
	tbNewCartesianPlot(0),
	tbZoom(0),
	glContext(glContext),
	vtkWidget(new QVTKWidget2(glContext)){

	setViewport(vtkWidget);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	vtkWidget->GetRenderWindow()->SetSwapBuffers(0);
	vtkWidget->setAutoBufferSwap(true);
	setScene(m_worksheet->scene());

	setRenderHint(QPainter::Antialiasing);
	setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setMinimumSize(16, 16);
	setFocusPolicy(Qt::StrongFocus);

	if (m_worksheet->useViewSize()) {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}

	viewport()->setAttribute( Qt::WA_OpaquePaintEvent );
	viewport()->setAttribute( Qt::WA_NoSystemBackground );
// 	setAcceptDrops( true );
	setCacheMode(QGraphicsView::CacheBackground);

	m_gridSettings.style = WorksheetView::NoGrid;

	initActions();
	initMenus();
	selectionModeAction->setChecked(true);
	handleCartesianPlotActions();

	changeZoom(zoomOriginAction);
	currentZoomAction=zoomInViewAction;

	//signal/slot connections
	connect(m_worksheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_worksheet, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(selectItem(QGraphicsItem*)) );
	connect(m_worksheet, SIGNAL(itemDeselected(QGraphicsItem*)), this, SLOT(deselectItem(QGraphicsItem*)) );
	connect(m_worksheet, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(selectItem(QGraphicsItem*)) );
	connect(m_worksheet, SIGNAL(itemDeselected(QGraphicsItem*)), this, SLOT(deselectItem(QGraphicsItem*)) );
	connect(m_worksheet, SIGNAL(requestUpdate()), this, SLOT(updateBackground()) );
	connect(m_worksheet, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)), this, SLOT(aspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_worksheet, SIGNAL(useViewSizeRequested()), this, SLOT(useViewSizeRequested()) );
	connect(m_worksheet, SIGNAL(layoutChanged(Worksheet::Layout)), this, SLOT(layoutChanged(Worksheet::Layout)) );
	connect(scene(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()) );
}

void WorksheetView::initActions(){
	QActionGroup* addNewActionGroup = new QActionGroup(this);
	QActionGroup* zoomActionGroup = new QActionGroup(this);
	QActionGroup* mouseModeActionGroup = new QActionGroup(this);
	QActionGroup* layoutActionGroup = new QActionGroup(this);
	QActionGroup* gridActionGroup = new QActionGroup(this);
	gridActionGroup->setExclusive(true);

	selectAllAction = new KAction(KIcon("edit-select-all"), i18n("Select all"), this);
	selectAllAction->setShortcut(Qt::CTRL+Qt::Key_A);
	this->addAction(selectAllAction);
	connect(selectAllAction, SIGNAL(triggered()), SLOT(selectAllElements()));

	deleteAction = new KAction(KIcon("edit-delete"), i18n("Delete"), this);
	deleteAction->setShortcut(Qt::Key_Delete);
	this->addAction(deleteAction);
	connect(deleteAction, SIGNAL(triggered()), SLOT(deleteElement()));

	backspaceAction = new KAction(this);
	backspaceAction->setShortcut(Qt::Key_Backspace);
	this->addAction(backspaceAction);
	connect(backspaceAction, SIGNAL(triggered()), SLOT(deleteElement()));

	//Zoom actions
	zoomInViewAction = new KAction(KIcon("zoom-in"), i18n("Zoom in"), zoomActionGroup);
	zoomInViewAction->setShortcut(Qt::CTRL+Qt::Key_Plus);

	zoomOutViewAction = new KAction(KIcon("zoom-out"), i18n("Zoom out"), zoomActionGroup);
	zoomOutViewAction->setShortcut(Qt::CTRL+Qt::Key_Minus);

	zoomOriginAction = new KAction(KIcon("zoom-original"), i18n("Original size"), zoomActionGroup);
	zoomOriginAction->setShortcut(Qt::CTRL+Qt::Key_1);

	zoomFitPageHeightAction = new KAction(KIcon("zoom-fit-height"), i18n("Fit to height"), zoomActionGroup);
	zoomFitPageWidthAction = new KAction(KIcon("zoom-fit-width"), i18n("Fit to width"), zoomActionGroup);
	zoomFitSelectionAction = new KAction(i18n("Fit to selection"), zoomActionGroup);

	// Mouse mode actions
	selectionModeAction = new KAction(KIcon("cursor-arrow"), i18n("Select and Edit"), mouseModeActionGroup);
	selectionModeAction->setCheckable(true);

	navigationModeAction = new KAction(KIcon("input-mouse"), i18n("Navigate"), mouseModeActionGroup);
	navigationModeAction->setCheckable(true);

	zoomSelectionModeAction = new KAction(KIcon("page-zoom"), i18n("Select and Zoom"), mouseModeActionGroup);
	zoomSelectionModeAction->setCheckable(true);

	//TODO implement later "group selection action" where multiple objects can be selected by drawing a rectangular
// 	selectionModeAction = new KAction(KIcon("select-rectangular"), i18n("Selection"), mouseModeActionGroup);
// 	selectionModeAction->setCheckable(true);

	//"Add new" related actions
	addCartesianPlot1Action = new KAction(KIcon("cartesian-plot-four-axes"), i18n("box plot, four axes"), addNewActionGroup);
	addCartesianPlot2Action = new KAction(KIcon("cartesian-plot-two-axes"), i18n("box plot, two axes"), addNewActionGroup);
	addCartesianPlot3Action = new KAction(KIcon("cartesian-plot-two-axes-centered"), i18n("two axes, centered"), addNewActionGroup);
	addCartesianPlot4Action = new KAction(KIcon("cartesian-plot-two-axes-centered-origin"), i18n("two axes, crossing at origin"), addNewActionGroup);
	add3DPlotAction = new KAction(KIcon("office-chart-line"), i18n("3D plot"), addNewActionGroup);
	addTextLabelAction = new KAction(KIcon("draw-text"), i18n("text label"), addNewActionGroup);

	//Layout actions
	verticalLayoutAction = new KAction(KIcon("editvlayout"), i18n("Vertical layout"), layoutActionGroup);
	verticalLayoutAction->setObjectName("verticalLayoutAction");
	verticalLayoutAction->setCheckable(true);

	horizontalLayoutAction = new KAction(KIcon("edithlayout"), i18n("Horizontal layout"), layoutActionGroup);
	horizontalLayoutAction->setObjectName("horizontalLayoutAction");
	horizontalLayoutAction->setCheckable(true);

	gridLayoutAction = new KAction(KIcon("editgrid"), i18n("Grid layout"), layoutActionGroup);
	gridLayoutAction->setObjectName("gridLayoutAction");
	gridLayoutAction->setCheckable(true);

	breakLayoutAction = new KAction(KIcon("editbreaklayout"), i18n("Break layout"), layoutActionGroup);
	breakLayoutAction->setObjectName("breakLayoutAction");
	breakLayoutAction->setEnabled(false);

   //Grid actions
	noGridAction = new KAction(i18n("no grid"), gridActionGroup);
	noGridAction->setObjectName("noGridAction");
	noGridAction->setCheckable(true);
	noGridAction->setChecked(true);
	noGridAction->setData(WorksheetView::NoGrid);

	denseLineGridAction = new KAction(i18n("dense line grid"), gridActionGroup);
	denseLineGridAction->setObjectName("denseLineGridAction");
	denseLineGridAction->setCheckable(true);

	sparseLineGridAction = new KAction(i18n("sparse line grid"), gridActionGroup);
	sparseLineGridAction->setObjectName("sparseLineGridAction");
	sparseLineGridAction->setCheckable(true);

	denseDotGridAction = new KAction(i18n("dense dot grid"), gridActionGroup);
	denseDotGridAction->setObjectName("denseDotGridAction");
	denseDotGridAction->setCheckable(true);

	sparseDotGridAction = new KAction(i18n("sparse dot grid"), gridActionGroup);
	sparseDotGridAction->setObjectName("sparseDotGridAction");
	sparseDotGridAction->setCheckable(true);

	customGridAction = new KAction(i18n("custom grid"), gridActionGroup);
	customGridAction->setObjectName("customGridAction");
	customGridAction->setCheckable(true);

	snapToGridAction = new KAction(i18n("snap to grid"), this);
	snapToGridAction->setCheckable(true);

	//check the action corresponding to the currently active layout in worksheet
	this->layoutChanged(m_worksheet->layout());

	connect(addNewActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(addNew(QAction*)));
	connect(mouseModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(mouseModeChanged(QAction*)));
	connect(zoomActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeZoom(QAction*)));
	connect(layoutActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeLayout(QAction*)));
	connect(gridActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeGrid(QAction*)));
	connect(snapToGridAction, SIGNAL(triggered()), this, SLOT(changeSnapToGrid()));


	//action for cartesian plots
	QActionGroup* cartesianPlotActionModeActionGroup = new QActionGroup(this);
	cartesianPlotActionModeActionGroup->setExclusive(true);
	cartesianPlotApplyToSelectionAction = new KAction(i18n("selected plots"), cartesianPlotActionModeActionGroup);
	cartesianPlotApplyToSelectionAction->setCheckable(true);
	cartesianPlotApplyToSelectionAction->setChecked(true);
	cartesianPlotApplyToAllAction = new KAction(i18n("all plots"), cartesianPlotActionModeActionGroup);
	cartesianPlotApplyToAllAction->setCheckable(true);
	connect(cartesianPlotActionModeActionGroup, SIGNAL(triggered(QAction*)), SLOT(cartesianPlotActionModeChanged(QAction*)));

	QActionGroup* cartesianPlotMouseModeActionGroup = new QActionGroup(this);
	cartesianPlotMouseModeActionGroup->setExclusive(true);
	cartesianPlotSelectionModeAction = new KAction(KIcon("cursor-arrow"), i18n("Select and edit"), cartesianPlotMouseModeActionGroup);
	cartesianPlotSelectionModeAction->setCheckable(true);
	cartesianPlotSelectionModeAction->setChecked(true);

	cartesianPlotZoomSelectionModeAction = new KAction(KIcon("zoom-select"), i18n("Select region and zoom in"), cartesianPlotMouseModeActionGroup);
	cartesianPlotZoomSelectionModeAction->setCheckable(true);

	cartesianPlotZoomXSelectionModeAction = new KAction(KIcon("zoom-select-x"), i18n("Select x-region and zoom in"), cartesianPlotMouseModeActionGroup);
	cartesianPlotZoomXSelectionModeAction->setCheckable(true);

	cartesianPlotZoomYSelectionModeAction = new KAction(KIcon("zoom-select-y"), i18n("Select y-region and zoom in"), cartesianPlotMouseModeActionGroup);
	cartesianPlotZoomYSelectionModeAction->setCheckable(true);

	connect(cartesianPlotMouseModeActionGroup, SIGNAL(triggered(QAction*)), SLOT(cartesianPlotMouseModeChanged(QAction*)));

	addCurveAction = new KAction(KIcon("xy-curve"), i18n("xy-curve"), this);
	addEquationCurveAction = new KAction(KIcon("xy-equation-curve"), i18n("xy-curve from a mathematical equation"), this);
	addFitCurveAction = new KAction(KIcon("xy-fit-curve"), i18n("xy-curve from a fit to data"), this);
	addLegendAction = new KAction(KIcon("text-field"), i18n("legend"), this);
	addHorizontalAxisAction = new KAction(KIcon("axis-horizontal"), i18n("horizontal axis"), this);
	addVerticalAxisAction = new KAction(KIcon("axis-vertical"), i18n("vertical axis"), this);

	scaleAutoAction = new KAction(KIcon("auto-scale-all"), i18n("auto scale"), this);
	scaleAutoXAction = new KAction(KIcon("auto-scale-x"), i18n("auto scale X"), this);
	scaleAutoYAction = new KAction(KIcon("auto-scale-y"), i18n("auto scale Y"), this);
	zoomInAction = new KAction(KIcon("zoom-in"), i18n("zoom in"), this);
	zoomOutAction = new KAction(KIcon("zoom-out"), i18n("zoom out"), this);
	zoomInXAction = new KAction(KIcon("zoom-in-x"), i18n("zoom in X"), this);
	zoomOutXAction = new KAction(KIcon("zoom-out-x"), i18n("zoom out X"), this);
	zoomInYAction = new KAction(KIcon("zoom-in-y"), i18n("zoom in Y"), this);
	zoomOutYAction = new KAction(KIcon("zoom-out-y"), i18n("zoom out Y"), this);
	shiftLeftXAction = new KAction(KIcon("shift-left-x"), i18n("shift left X"), this);
	shiftRightXAction = new KAction(KIcon("shift-right-x"), i18n("shift right X"), this);
	shiftUpYAction = new KAction(KIcon("shift-up-y"), i18n("shift up Y"), this);
	shiftDownYAction = new KAction(KIcon("shift-down-y"), i18n("shift down Y"), this);

	connect(addCurveAction, SIGNAL(triggered()), SLOT(addCurve()));
	connect(addEquationCurveAction, SIGNAL(triggered()), SLOT(addEquationCurve()));
	connect(addFitCurveAction, SIGNAL(triggered()), SLOT(addFitCurve()));
	connect(addLegendAction, SIGNAL(triggered()), SLOT(addLegend()));
	connect(addHorizontalAxisAction, SIGNAL(triggered()), SLOT(addHorizontalAxis()));
	connect(addVerticalAxisAction, SIGNAL(triggered()), SLOT(addVerticalAxis()));

	//zoom actions
	connect(scaleAutoAction, SIGNAL(triggered()), SLOT(scaleAuto()));
	connect(scaleAutoXAction, SIGNAL(triggered()), SLOT(scaleAutoX()));
	connect(scaleAutoYAction, SIGNAL(triggered()), SLOT(scaleAutoY()));
	connect(zoomInAction, SIGNAL(triggered()), SLOT(zoomIn()));
	connect(zoomOutAction, SIGNAL(triggered()), SLOT(zoomOut()));
	connect(zoomInXAction, SIGNAL(triggered()), SLOT(zoomInX()));
	connect(zoomOutXAction, SIGNAL(triggered()), SLOT(zoomOutX()));
	connect(zoomInYAction, SIGNAL(triggered()), SLOT(zoomInY()));
	connect(zoomOutYAction, SIGNAL(triggered()), SLOT(zoomOutY()));
	connect(shiftLeftXAction, SIGNAL(triggered()), SLOT(shiftLeftX()));
	connect(shiftRightXAction, SIGNAL(triggered()), SLOT(shiftRightX()));
	connect(shiftUpYAction, SIGNAL(triggered()), SLOT(shiftUpY()));
	connect(shiftDownYAction, SIGNAL(triggered()), SLOT(shiftDownY()));
}

void WorksheetView::initMenus(){
	m_addNewCartesianPlotMenu = new QMenu(i18n("xy-plot"));
	m_addNewCartesianPlotMenu->addAction(addCartesianPlot1Action);
	m_addNewCartesianPlotMenu->addAction(addCartesianPlot2Action);
	m_addNewCartesianPlotMenu->addAction(addCartesianPlot3Action);
	m_addNewCartesianPlotMenu->addAction(addCartesianPlot4Action);


	m_addNewMenu = new QMenu(i18n("Add new"));
	m_addNewMenu->addMenu(m_addNewCartesianPlotMenu)->setIcon(KIcon("office-chart-line"));
	m_addNewMenu->addAction(add3DPlotAction);
	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(addTextLabelAction);

	m_viewMouseModeMenu = new QMenu(i18n("Mouse Mode"));
	m_viewMouseModeMenu->setIcon(KIcon("input-mouse"));
	m_viewMouseModeMenu->addAction(selectionModeAction);
	m_viewMouseModeMenu->addAction(navigationModeAction);
	m_viewMouseModeMenu->addAction(zoomSelectionModeAction);

	m_zoomMenu = new QMenu(i18n("Zoom"));
	m_zoomMenu->setIcon(KIcon("zoom-draw"));
	m_zoomMenu->addAction(zoomInViewAction);
	m_zoomMenu->addAction(zoomOutViewAction);
	m_zoomMenu->addAction(zoomOriginAction);
	m_zoomMenu->addAction(zoomFitPageHeightAction);
	m_zoomMenu->addAction(zoomFitPageWidthAction);
	m_zoomMenu->addAction(zoomFitSelectionAction);

	m_layoutMenu = new QMenu(i18n("Layout"));
	m_layoutMenu->addAction(verticalLayoutAction);
	m_layoutMenu->addAction(horizontalLayoutAction);
	m_layoutMenu->addAction(gridLayoutAction);
	m_layoutMenu->addSeparator();
	m_layoutMenu->addAction(breakLayoutAction);

	m_gridMenu = new QMenu(i18n("Grid"));
	m_gridMenu->setIcon(KIcon("view-grid"));
	m_gridMenu->addAction(noGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(sparseLineGridAction);
	m_gridMenu->addAction(denseLineGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(sparseDotGridAction);
	m_gridMenu->addAction(denseDotGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(customGridAction);
	//TODO: implement "snap to grid" and activate this action
// 	m_gridMenu->addSeparator();
// 	m_gridMenu->addAction(snapToGridAction);

	m_cartesianPlotMenu = new QMenu(i18n("Cartesian Plot"));

	m_cartesianPlotMouseModeMenu = new QMenu(i18n("Mouse Mode"));
	m_cartesianPlotMouseModeMenu->setIcon(KIcon("input-mouse"));
	m_cartesianPlotMouseModeMenu->addAction(cartesianPlotSelectionModeAction);
	m_cartesianPlotMouseModeMenu->addAction(cartesianPlotZoomSelectionModeAction);
	m_cartesianPlotMouseModeMenu->addAction(cartesianPlotZoomXSelectionModeAction);
	m_cartesianPlotMouseModeMenu->addAction(cartesianPlotZoomYSelectionModeAction);
	m_cartesianPlotMouseModeMenu->addSeparator();

	m_cartesianPlotAddNewMenu = new QMenu(i18n("Add new"));
	m_cartesianPlotAddNewMenu->addAction(addCurveAction);
	m_cartesianPlotAddNewMenu->addAction(addEquationCurveAction);
	m_cartesianPlotAddNewMenu->addAction(addFitCurveAction);
	m_cartesianPlotAddNewMenu->addAction(addLegendAction);
	m_cartesianPlotAddNewMenu->addSeparator();
	m_cartesianPlotAddNewMenu->addAction(addHorizontalAxisAction);
	m_cartesianPlotAddNewMenu->addAction(addVerticalAxisAction);

	m_cartesianPlotZoomMenu = new QMenu(i18n("Zoom/Navigate"));
	m_cartesianPlotZoomMenu->setIcon(KIcon("zoom-draw"));
	m_cartesianPlotZoomMenu->addAction(scaleAutoAction);
	m_cartesianPlotZoomMenu->addAction(scaleAutoXAction);
	m_cartesianPlotZoomMenu->addAction(scaleAutoYAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(zoomInAction);
	m_cartesianPlotZoomMenu->addAction(zoomOutAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(zoomInXAction);
	m_cartesianPlotZoomMenu->addAction(zoomOutXAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(zoomInYAction);
	m_cartesianPlotZoomMenu->addAction(zoomOutYAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(shiftLeftXAction);
	m_cartesianPlotZoomMenu->addAction(shiftRightXAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(shiftUpYAction);
	m_cartesianPlotZoomMenu->addAction(shiftDownYAction);

	m_cartesianPlotActionModeMenu = new QMenu(i18n("Apply actions to"));
	m_cartesianPlotActionModeMenu->addAction(cartesianPlotApplyToSelectionAction);
	m_cartesianPlotActionModeMenu->addAction(cartesianPlotApplyToAllAction);

	m_cartesianPlotMenu->addMenu(m_cartesianPlotMouseModeMenu);
	m_cartesianPlotMenu->addMenu(m_cartesianPlotAddNewMenu);
	m_cartesianPlotMenu->addMenu(m_cartesianPlotZoomMenu);
	m_cartesianPlotMenu->addSeparator();
	m_cartesianPlotMenu->addMenu(m_cartesianPlotActionModeMenu);
}

/*!
 * Populates the menu \c menu with the worksheet and worksheet view relevant actions.
 * The menu is used
 *   - as the context menu in WorksheetView
 *   - as the "worksheet menu" in the main menu-bar (called form MainWin)
 *   - as a part of the worksheet context menu in project explorer
 */
void WorksheetView::createContextMenu(QMenu* menu) const {
	Q_ASSERT(menu);

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QAction* firstAction = menu->actions().first();
#else
	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);
#endif

	menu->insertMenu(firstAction, m_addNewMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_viewMouseModeMenu);
	menu->insertMenu(firstAction, m_zoomMenu);
	menu->insertMenu(firstAction, m_layoutMenu);
	menu->insertMenu(firstAction, m_gridMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_cartesianPlotMenu);
	menu->insertSeparator(firstAction);
}

void WorksheetView::fillToolBar(QToolBar* toolBar){
	toolBar->addSeparator();
	tbNewCartesianPlot = new QToolButton(toolBar);
	tbNewCartesianPlot->setPopupMode(QToolButton::MenuButtonPopup);
	tbNewCartesianPlot->setMenu(m_addNewCartesianPlotMenu);
	tbNewCartesianPlot->setDefaultAction(addCartesianPlot1Action);
	toolBar->addWidget(tbNewCartesianPlot);
	toolBar->addAction(addTextLabelAction);

	toolBar->addSeparator();
	toolBar->addAction(verticalLayoutAction);
	toolBar->addAction(horizontalLayoutAction);
	toolBar->addAction(gridLayoutAction);
	toolBar->addAction(breakLayoutAction);

	toolBar->addSeparator();
	toolBar->addAction(selectionModeAction);
	toolBar->addAction(navigationModeAction);
	toolBar->addAction(zoomSelectionModeAction);
	tbZoom = new QToolButton(toolBar);
	tbZoom->setPopupMode(QToolButton::MenuButtonPopup);
	tbZoom->setMenu(m_zoomMenu);
	tbZoom->setDefaultAction(currentZoomAction);
	toolBar->addWidget(tbZoom);
}

void WorksheetView::fillCartesianPlotToolBar(QToolBar* toolBar) {
	toolBar->addAction(cartesianPlotSelectionModeAction);
	toolBar->addAction(cartesianPlotZoomSelectionModeAction);
	toolBar->addAction(cartesianPlotZoomXSelectionModeAction);
	toolBar->addAction(cartesianPlotZoomYSelectionModeAction);
	toolBar->addSeparator();
	toolBar->addAction(addCurveAction);
	toolBar->addAction(addEquationCurveAction);
	toolBar->addAction(addFitCurveAction);
	toolBar->addAction(addLegendAction);
	toolBar->addSeparator();
	toolBar->addAction(addHorizontalAxisAction);
	toolBar->addAction(addVerticalAxisAction);
	toolBar->addSeparator();
	toolBar->addAction(scaleAutoAction);
	toolBar->addAction(scaleAutoXAction);
	toolBar->addAction(scaleAutoYAction);
	toolBar->addAction(zoomInAction);
	toolBar->addAction(zoomOutAction);
	toolBar->addAction(zoomInXAction);
	toolBar->addAction(zoomOutXAction);
	toolBar->addAction(zoomInYAction);
	toolBar->addAction(zoomOutYAction);
	toolBar->addAction(shiftLeftXAction);
	toolBar->addAction(shiftRightXAction);
	toolBar->addAction(shiftUpYAction);
	toolBar->addAction(shiftDownYAction);
}

void WorksheetView::setScene(QGraphicsScene* scene) {
  QGraphicsView::setScene(scene);
  setTransform(QTransform());
}

void WorksheetView::drawForeground(QPainter* painter, const QRectF& rect) {
	if (m_mouseMode==ZoomSelectionMode && m_selectionBandIsShown) {
		painter->save();
		const QRectF& selRect = mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect();
		painter->setPen(QPen(Qt::black, 5));
		painter->drawRect(selRect);
		painter->setBrush(Qt::blue);
		painter->setOpacity(0.2);
		painter->drawRect(selRect);
		painter->restore();
	}
	QGraphicsView::drawForeground(painter, rect);
}

void WorksheetView::drawBackground(QPainter* painter, const QRectF& rect) {
	painter->save();

	//painter->setRenderHint(QPainter::Antialiasing);
	QRectF scene_rect = sceneRect();

	if (!m_worksheet->useViewSize()) {
		// background
		if (!scene_rect.contains(rect))
			painter->fillRect(rect, Qt::lightGray);

		//shadow
		int shadowSize = scene_rect.width()*0.02;
		QRectF rightShadowRect(scene_rect.right(), scene_rect.top() + shadowSize,
											shadowSize, scene_rect.height());
		QRectF bottomShadowRect(scene_rect.left() + shadowSize, scene_rect.bottom(),
											scene_rect.width(), shadowSize);

		painter->fillRect(rightShadowRect.intersected(rect), Qt::darkGray);
		painter->fillRect(bottomShadowRect.intersected(rect), Qt::darkGray);
	}

	// canvas
	painter->setOpacity(m_worksheet->backgroundOpacity());
	if (m_worksheet->backgroundType() == PlotArea::Color){
		switch (m_worksheet->backgroundColorStyle()){
			case PlotArea::SingleColor:{
				painter->setBrush(QBrush(m_worksheet->backgroundFirstColor()));
				break;
			}
			case PlotArea::HorizontalLinearGradient:{
				QLinearGradient linearGrad(scene_rect.topLeft(), scene_rect.topRight());
				linearGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				linearGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::VerticalLinearGradient:{
				QLinearGradient linearGrad(scene_rect.topLeft(), scene_rect.bottomLeft());
				linearGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				linearGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::TopLeftDiagonalLinearGradient:{
				QLinearGradient linearGrad(scene_rect.topLeft(), scene_rect.bottomRight());
				linearGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				linearGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::BottomLeftDiagonalLinearGradient:{
				QLinearGradient linearGrad(scene_rect.bottomLeft(), scene_rect.topRight());
				linearGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				linearGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::RadialGradient:{
				QRadialGradient radialGrad(scene_rect.center(), scene_rect.width()/2);
				radialGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				radialGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(radialGrad));
				break;
			}
			default:
				painter->setBrush(QBrush(m_worksheet->backgroundFirstColor()));
		}
		painter->drawRect(scene_rect);
	}else if (m_worksheet->backgroundType() == PlotArea::Image){
		const QString& backgroundFileName = m_worksheet->backgroundFileName().trimmed();
		if ( !backgroundFileName.isEmpty() ) {
			QPixmap pix(backgroundFileName);
			switch (m_worksheet->backgroundImageStyle()){
				case PlotArea::ScaledCropped:
					pix = pix.scaled(scene_rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
					painter->drawPixmap(scene_rect.topLeft(),pix);
					break;
				case PlotArea::Scaled:
					pix = pix.scaled(scene_rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
					painter->drawPixmap(scene_rect.topLeft(),pix);
					break;
				case PlotArea::ScaledAspectRatio:
					pix = pix.scaled(scene_rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
					painter->drawPixmap(scene_rect.topLeft(),pix);
					break;
				case PlotArea::Centered:
					painter->drawPixmap(QPointF(scene_rect.center().x()-pix.size().width()/2,scene_rect.center().y()-pix.size().height()/2),pix);
					break;
				case PlotArea::Tiled:
					painter->drawTiledPixmap(scene_rect,pix);
					break;
				case PlotArea::CenterTiled:
					painter->drawTiledPixmap(scene_rect,pix,QPoint(scene_rect.size().width()/2,scene_rect.size().height()/2));
					break;
				default:
					painter->drawPixmap(scene_rect.topLeft(),pix);
			}
		}
	}else if (m_worksheet->backgroundType() == PlotArea::Pattern){
		painter->setBrush(QBrush(m_worksheet->backgroundFirstColor(),m_worksheet->backgroundBrushStyle()));
		painter->drawRect(scene_rect);
	}

  //grid
	if (m_gridSettings.style != WorksheetView::NoGrid){
		QColor c=m_gridSettings.color;
 		c.setAlphaF(m_gridSettings.opacity);
		painter->setPen(c);

		qreal x, y;
		qreal left = scene_rect.left();
		qreal right = scene_rect.right();
		qreal top = scene_rect.top();
		qreal bottom = scene_rect.bottom();

		if (m_gridSettings.style==WorksheetView::LineGrid){
			QLineF line;

			//horizontal lines
			y = top + m_gridSettings.verticalSpacing;
			while (y < bottom) {
				line.setLine( left, y,  right, y );
				painter->drawLine(line);
				y += m_gridSettings.verticalSpacing;
			}

			//vertical lines
			x = left + m_gridSettings.horizontalSpacing;
			while (x < right) {
				line.setLine( x, top,  x, bottom );
				painter->drawLine(line);
				x += m_gridSettings.horizontalSpacing;
			}
		}else{ //DotGrid
			y = top + m_gridSettings.verticalSpacing;
			while (y < bottom){
				x = left;// + m_gridSettings.horizontalSpacing;
				while (x < right){
					x += m_gridSettings.horizontalSpacing;
					painter->drawPoint(x, y);
				}
				y += m_gridSettings.verticalSpacing;
			}
		}
	}

	invalidateScene(rect, QGraphicsScene::BackgroundLayer);
	painter->restore();

	painter->beginNativePainting();
	vtkWidget->GetRenderWindow()->PushState();
	vtkWidget->GetRenderWindow()->OpenGLInitState();
	vtkWidget->GetRenderWindow()->Render();
	vtkWidget->GetRenderWindow()->PopState();
	painter->endNativePainting();
}

//##############################################################################
//####################################  Events   ###############################
//##############################################################################
void WorksheetView::resizeEvent(QResizeEvent *event) {
	if (m_worksheet->useViewSize())
		this->processResize();

	QGraphicsView::resizeEvent(event);
	vtkWidget->GetRenderWindow()->SetSize(event->size().width(), event->size().height());
}

void WorksheetView::wheelEvent(QWheelEvent *event) {
  if (m_mouseMode == ZoomSelectionMode){
	if (event->delta() > 0)
		scale(1.2, 1.2);
	else if (event->delta() < 0)
		scale(1.0/1.2, 1.0/1.2);
  }else{
	QGraphicsView::wheelEvent(event);
  }
}

void WorksheetView::mousePressEvent(QMouseEvent* event) {
	if (m_mouseMode == ZoomSelectionMode) {
		m_selectionStart = event->pos();
		m_selectionBandIsShown = true;
	}

	//prevent the deselection of items when context menu event
	//was triggered (right button click)
	if (event->button() != Qt::LeftButton) {
        event->accept();
        return;
    }

	// select the worksheet in the project explorer if the view was clicked
	// and there is no selection currently. We need this for the case when
	// there is a single worksheet in the project and we change from the project-node
	// in the project explorer to the worksheet-node by clicking the view.
	if ( scene()->selectedItems().empty() )
		m_worksheet->setSelectedInView(true);

	QGraphicsView::mousePressEvent(event);
}

void WorksheetView::mouseReleaseEvent(QMouseEvent* event) {
	if (m_mouseMode == ZoomSelectionMode) {
		m_selectionBandIsShown = false;
		viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());

		//don't zoom if very small region was selected, avoid occasional/unwanted zooming
		m_selectionEnd = event->pos();
		if ( abs(m_selectionEnd.x()-m_selectionStart.x())>20 && abs(m_selectionEnd.y()-m_selectionStart.y())>20 )
			fitInView(mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect(), Qt::KeepAspectRatio);
	}
	QGraphicsView::mouseReleaseEvent(event);
}

void WorksheetView::mouseMoveEvent(QMouseEvent* event) {
	if (m_mouseMode == SelectionMode && m_cartesianPlotMouseMode != CartesianPlot::SelectionMode ) {
		//check whether there is a cartesian plot under the cursor
		bool plot = false;
		QGraphicsItem* item = itemAt(event->pos());
		if (item) {
			plot = item->data(0).toInt() == WorksheetElement::NameCartesianPlot;
			if (!plot && item->parentItem())
				plot = item->parentItem()->data(0).toInt() == WorksheetElement::NameCartesianPlot;
		}

		//set the cursor appearance according to the current mouse mode for the cartesian plots
		if (plot) {
			if (m_cartesianPlotMouseMode == CartesianPlot::ZoomSelectionMode) {
				setCursor(Qt::CrossCursor);
			} else if (m_cartesianPlotMouseMode == CartesianPlot::ZoomXSelectionMode) {
				setCursor(Qt::SizeHorCursor);
			} else if (m_cartesianPlotMouseMode == CartesianPlot::ZoomYSelectionMode) {
				setCursor(Qt::SizeVerCursor);
			}
		} else {
			setCursor(Qt::ArrowCursor);
		}
	} else if (m_mouseMode == SelectionMode && m_cartesianPlotMouseMode == CartesianPlot::SelectionMode ) {
		setCursor(Qt::ArrowCursor);
	} else if (m_selectionBandIsShown) {
		m_selectionEnd = event->pos();
		viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());
	}

	QGraphicsView::mouseMoveEvent(event);
}

void WorksheetView::contextMenuEvent(QContextMenuEvent* e) {
	if ( !itemAt(e->pos()) ){
		//no item under the cursor -> show the context menu for the worksheet
		QMenu *menu = new QMenu(this);
		this->createContextMenu(menu);
		menu->exec(QCursor::pos());
	}else{
		//propagate the event to the scene and graphics items
		QGraphicsView::contextMenuEvent(e);
	}
}

//##############################################################################
//####################################  SLOTs   ################################
//##############################################################################
void WorksheetView::useViewSizeRequested() {
	if (m_worksheet->useViewSize()) {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		zoomFitPageHeightAction->setVisible(false);
		zoomFitPageWidthAction->setVisible(false);
		currentZoomAction = zoomInViewAction;
		if (tbZoom)
			tbZoom->setDefaultAction(zoomInViewAction);

		//determine and set the current view size
		this->processResize();
	} else {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		zoomFitPageHeightAction->setVisible(true);
		zoomFitPageWidthAction->setVisible(true);
	}
}

void WorksheetView::processResize() {
	if (size() != sceneRect().size()) {
		static const float hscale = QApplication::desktop()->physicalDpiX()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		static const float vscale = QApplication::desktop()->physicalDpiY()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		m_worksheet->setUndoAware(false);
		m_worksheet->setPageRect(QRectF(0.0, 0.0, width()/hscale, height()/vscale));
		m_worksheet->setUndoAware(true);
	}
}

void WorksheetView::changeZoom(QAction* action){
	if (action==zoomInViewAction){
		scale(1.2, 1.2);
	}else if (action==zoomOutViewAction){
		scale(1.0/1.2, 1.0/1.2);
	}else if (action==zoomOriginAction){
		static const float hscale = QApplication::desktop()->physicalDpiX()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		static const float vscale = QApplication::desktop()->physicalDpiY()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		setTransform(QTransform::fromScale(hscale, vscale));
	}else if (action==zoomFitPageWidthAction){
		float scaleFactor = viewport()->width()/scene()->sceneRect().width();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
	}else if (action==zoomFitPageHeightAction){
		float scaleFactor = viewport()->height()/scene()->sceneRect().height();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
	}else if (action==zoomFitSelectionAction){
		fitInView(scene()->selectionArea().boundingRect(),Qt::KeepAspectRatio);
	}
	currentZoomAction=action;
	if (tbZoom)
		tbZoom->setDefaultAction(action);
}

void WorksheetView::mouseModeChanged(QAction* action) {
	if (action==selectionModeAction) {
		m_mouseMode = SelectionMode;
		setInteractive(true);
		setDragMode(QGraphicsView::NoDrag);
	} else if (action==navigationModeAction) {
		m_mouseMode = NavigationMode;
		setInteractive(false);
		setDragMode(QGraphicsView::ScrollHandDrag);
	} else {
		m_mouseMode = ZoomSelectionMode;
		setInteractive(false);
		setDragMode(QGraphicsView::NoDrag);
	}
}

//"Add new" related slots
void WorksheetView::addNew(QAction* action){
	WorksheetElement* aspect = 0;
	if ( action == addCartesianPlot1Action ){
		CartesianPlot* plot = new CartesianPlot(i18n("xy-plot"));
		plot->initDefault(CartesianPlot::FourAxes);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewCartesianPlot)
			tbNewCartesianPlot->setDefaultAction(addCartesianPlot1Action);
	}else if ( action == addCartesianPlot2Action ){
		CartesianPlot* plot = new CartesianPlot(i18n("xy-plot"));
		plot->initDefault(CartesianPlot::TwoAxes);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewCartesianPlot)
			tbNewCartesianPlot->setDefaultAction(addCartesianPlot2Action);
	}else if ( action == addCartesianPlot3Action ){
		CartesianPlot* plot = new CartesianPlot(i18n("xy-plot"));
		plot->initDefault(CartesianPlot::TwoAxesCentered);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewCartesianPlot)
			tbNewCartesianPlot->setDefaultAction(addCartesianPlot3Action);
	}else if ( action == addCartesianPlot4Action ){
		CartesianPlot* plot = new CartesianPlot(i18n("xy-plot"));
		plot->initDefault(CartesianPlot::TwoAxesCenteredZero);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewCartesianPlot)
			tbNewCartesianPlot->setDefaultAction(addCartesianPlot4Action);
	}else if ( action == add3DPlotAction ){
		// TODO: Create a 3D plot widget
		qDebug() << Q_FUNC_INFO << "Add3dPlot menu clicked";
		Plot3D* plot = new Plot3D(i18n("3D Plot"), glContext);
		aspect = plot;
	}else if ( action == addTextLabelAction ){
		TextLabel* l = new TextLabel(i18n("text label"));
		l->setText(i18n("text label"));
		aspect = l;
	}

	if (!aspect)
		return;

	m_worksheet->addChild(aspect);
	handleCartesianPlotActions();

	if (!m_fadeInTimeLine) {
		m_fadeInTimeLine = new QTimeLine(1000, this);
		m_fadeInTimeLine->setFrameRange(0, 100);
		connect(m_fadeInTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(fadeIn(qreal)));
	}

	//if there is already an element fading in, stop the time line and show the element with the full opacity.
	if (m_fadeInTimeLine->state() == QTimeLine::Running) {
		m_fadeInTimeLine->stop();
		QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
		effect->setOpacity(1);
		lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
	}

	//fade-in the newly added element
	lastAddedWorksheetElement = aspect;
	QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
	effect->setOpacity(0);
	lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
	m_fadeInTimeLine->start();
}

/*!
 * select all top-level items
 */
void WorksheetView::selectAllElements() {
	//deselect all previously selected items since there can be some non top-level items belong them
	m_suppressSelectionChangedEvent = true;
	QList<QGraphicsItem*> items = scene()->selectedItems();
	foreach ( QGraphicsItem* item , m_selectedItems ){
		m_worksheet->setItemSelectedInView(item, false);
	}

	//select top-level items
	items = scene()->items();
	foreach(QGraphicsItem* item, items){
		if (!item->parentItem())
			item->setSelected(true);
	}
	m_suppressSelectionChangedEvent = false;
	this->selectionChanged();
}

/*!
 * deletes selected worksheet elements
 */
void WorksheetView::deleteElement() {
	QList<QGraphicsItem*> items = scene()->selectedItems();
	if (items.size()==0)
		return;

	int rc = KMessageBox::warningYesNo( this,
			i18np("Do you really want to delete the selected object?", "Do you really want to delete the selected %1 objects?", items.size()),
			i18n("Delete selected objects"));

	if (rc==KMessageBox::No)
		return;

	m_suppressSelectionChangedEvent = true;
	m_worksheet->beginMacro(i18n("%1: Remove selected worksheet elements.", m_worksheet->name()));
	foreach ( QGraphicsItem* item , m_selectedItems ) {
		m_worksheet->deleteAspectFromGraphicsItem(item);
	}
	m_worksheet->endMacro();
	m_suppressSelectionChangedEvent = false;
}

void WorksheetView::aspectAboutToBeRemoved(const AbstractAspect* aspect){
	lastAddedWorksheetElement = dynamic_cast<WorksheetElement*>(const_cast<AbstractAspect*>(aspect));
	if (!lastAddedWorksheetElement)
		return;

	//FIXME: fading-out doesn't work
	//also, the following code collides with undo/redo of the deletion
	//of a worksheet element (after redoing the element is not shown with the full opacity
/*
	if (!m_fadeOutTimeLine) {
		m_fadeOutTimeLine = new QTimeLine(1000, this);
		m_fadeOutTimeLine->setFrameRange(0, 100);
		connect(m_fadeOutTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(fadeOut(qreal)));
	}

	//if there is already an element fading out, stop the time line
	if (m_fadeOutTimeLine->state() == QTimeLine::Running)
		m_fadeOutTimeLine->stop();

	m_fadeOutTimeLine->start();
*/
}

void WorksheetView::fadeIn(qreal value) {
	QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
	effect->setOpacity(value);
	if (lastAddedWorksheetElement->graphicsItem())
		lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
}

void WorksheetView::fadeOut(qreal value) {
	QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
	effect->setOpacity(1-value);
	if (lastAddedWorksheetElement->graphicsItem())
		lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
}

/*!
 * called when one of the layout-actions in WorkseetView was triggered.
 * sets the layout in Worksheet and enables/disables the layout actions.
 */
void WorksheetView::changeLayout(QAction* action){
	if (action==breakLayoutAction){
		verticalLayoutAction->setEnabled(true);
		verticalLayoutAction->setChecked(false);

		horizontalLayoutAction->setEnabled(true);
		horizontalLayoutAction->setChecked(false);

		gridLayoutAction->setEnabled(true);
		gridLayoutAction->setChecked(false);

		breakLayoutAction->setEnabled(false);

		m_worksheet->setLayout(Worksheet::NoLayout);
	}else{
		verticalLayoutAction->setEnabled(false);
		horizontalLayoutAction->setEnabled(false);
		gridLayoutAction->setEnabled(false);
		breakLayoutAction->setEnabled(true);

		if (action == verticalLayoutAction){
			verticalLayoutAction->setChecked(true);
			m_worksheet->setLayout(Worksheet::VerticalLayout);
		}else if (action == horizontalLayoutAction){
			horizontalLayoutAction->setChecked(true);
			m_worksheet->setLayout(Worksheet::HorizontalLayout);
		}else{
			gridLayoutAction->setChecked(true);
			m_worksheet->setLayout(Worksheet::GridLayout);
		}
	}
}

void WorksheetView::changeGrid(QAction* action){
	QString name = action->objectName();

	if (name == "noGridAction"){
		m_gridSettings.style = WorksheetView::NoGrid;
		snapToGridAction->setEnabled(false);
	}else if (name == "sparseLineGridAction"){
		m_gridSettings.style = WorksheetView::LineGrid;
		m_gridSettings.color = Qt::gray;
		m_gridSettings.opacity = 0.7;
		m_gridSettings.horizontalSpacing = 15;
		m_gridSettings.verticalSpacing = 15;
	}else if (name == "denseLineGridAction"){
		m_gridSettings.style = WorksheetView::LineGrid;
		m_gridSettings.color = Qt::gray;
		m_gridSettings.opacity = 0.7;
		m_gridSettings.horizontalSpacing = 5;
		m_gridSettings.verticalSpacing = 5;
	}else if (name == "denseDotGridAction"){
		m_gridSettings.style = WorksheetView::DotGrid;
		m_gridSettings.color = Qt::black;
		m_gridSettings.opacity = 0.7;
		m_gridSettings.horizontalSpacing = 5;
		m_gridSettings.verticalSpacing = 5;
	}else if (name == "sparseDotGridAction"){
		m_gridSettings.style = WorksheetView::DotGrid;
		m_gridSettings.color = Qt::black;
		m_gridSettings.opacity = 0.7;
		m_gridSettings.horizontalSpacing = 15;
		m_gridSettings.verticalSpacing = 15;
	}else if (name == "customGridAction"){
		GridDialog* dlg = new GridDialog(this);
		if (dlg->exec() == QDialog::Accepted)
			dlg->save(m_gridSettings);
		else
			return;
	}

	if (m_gridSettings.style == WorksheetView::NoGrid)
		snapToGridAction->setEnabled(false);
	else
		snapToGridAction->setEnabled(true);

	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}

//TODO
void WorksheetView::changeSnapToGrid(){

}

/*!
 *  Selects the QGraphicsItem \c item in \c WorksheetView.
 * 	The selection in \c ProjectExplorer is forwarded to  \c Worksheet
 *  and is finally handled here.
 */
void WorksheetView::selectItem(QGraphicsItem* item){
	m_suppressSelectionChangedEvent = true;
	item->setSelected(true);
	m_selectedItems<<item;
	handleCartesianPlotActions();
	m_suppressSelectionChangedEvent = false;
}

/*!
 *  Deselects the \c QGraphicsItem \c item in \c WorksheetView.
 * 	The deselection in \c ProjectExplorer is forwarded to \c Worksheet
 *  and is finally handled here.
 */
void WorksheetView::deselectItem(QGraphicsItem* item){
	m_suppressSelectionChangedEvent = true;
	item->setSelected(false);
	m_selectedItems.removeOne(item);
	handleCartesianPlotActions();
	m_suppressSelectionChangedEvent = false;
}

/*!
 *  Called on selection changes in the view.
 *   Determines which items were selected and deselected
 *  and forwards these changes to \c Worksheet
 */
void WorksheetView::selectionChanged(){
	if (m_suppressSelectionChangedEvent)
		return;

	QList<QGraphicsItem*> items = scene()->selectedItems();

	//When making a graphics item invisible, it gets deselected in the scene.
	//In this case we don't want to deselect the item in the project explorer.
	bool invisibleDeselected = false;

	//check, whether the previously selected items were deselected now.
	//Forward the deselection prior to the selection of new items
	//in order to avoid the unwanted multiple selection in project explorer
	foreach ( QGraphicsItem* item, m_selectedItems ){
		if ( items.indexOf(item) == -1 ) {
			if (item->isVisible())
				m_worksheet->setItemSelectedInView(item, false);
			else
				invisibleDeselected = true;
		}
	}

	//select new items
	if (items.size() == 0 && invisibleDeselected == false){
		//no items selected -> select the worksheet again.
		m_worksheet->setSelectedInView(true);
	}else{
		foreach (const QGraphicsItem* item, items)
			m_worksheet->setItemSelectedInView(item, true);

		//items selected -> deselect the worksheet in the project explorer
		//prevents unwanted multiple selection with worksheet (if it was selected before)
		m_worksheet->setSelectedInView(false);
	}

	m_selectedItems = items;
	handleCartesianPlotActions();
}

//check whether we have cartesian plots selected and activate/deactivate
void WorksheetView::handleCartesianPlotActions() {
	bool plot = false;
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		//check whether we have cartesian plots selected
		foreach (QGraphicsItem* item , m_selectedItems) {
			if (item->data(0).toInt() == WorksheetElement::NameCartesianPlot) {
				plot = true;
				break;
			}
		}
	} else {
		//actions are applied to all available plots -> check whether we have plots
		plot = (m_worksheet->children<CartesianPlot>().size() != 0);
	}

	cartesianPlotSelectionModeAction->setEnabled(plot);
	cartesianPlotZoomSelectionModeAction->setEnabled(plot);
	cartesianPlotZoomXSelectionModeAction->setEnabled(plot);
	cartesianPlotZoomYSelectionModeAction->setEnabled(plot);

	addCurveAction->setEnabled(plot);
	addEquationCurveAction->setEnabled(plot);
	addFitCurveAction->setEnabled(plot);
	addHorizontalAxisAction->setEnabled(plot);
	addVerticalAxisAction->setEnabled(plot);
	addLegendAction->setEnabled(plot);

	scaleAutoXAction->setEnabled(plot);
	scaleAutoYAction->setEnabled(plot);
	scaleAutoAction->setEnabled(plot);
	zoomInAction->setEnabled(plot);
	zoomOutAction->setEnabled(plot);
	zoomInXAction->setEnabled(plot);
	zoomOutXAction->setEnabled(plot);
	zoomInYAction->setEnabled(plot);
	zoomOutYAction->setEnabled(plot);
	shiftLeftXAction->setEnabled(plot);
	shiftRightXAction->setEnabled(plot);
	shiftUpYAction->setEnabled(plot);
	shiftDownYAction->setEnabled(plot);
}

void WorksheetView::exportToFile(const QString& path, const ExportFormat format,
								 const ExportArea area, const bool background, const int resolution) {
	QRectF sourceRect;

	//determine the rectangular to print
	if (area==WorksheetView::ExportBoundingBox){
		sourceRect = scene()->itemsBoundingRect();
	}else if (area==WorksheetView::ExportSelection){
		//TODO doesn't work: rect = scene()->selectionArea().boundingRect();
		foreach(QGraphicsItem* item, m_selectedItems) {
			sourceRect = sourceRect.united( item->mapToScene(item->boundingRect()).boundingRect() );
		}
	}else{
		sourceRect = scene()->sceneRect();
	}

	//print
	if (format==WorksheetView::Pdf || format==WorksheetView::Eps){
		QPrinter printer(QPrinter::HighResolution);
		if (format==WorksheetView::Pdf)
			printer.setOutputFormat(QPrinter::PdfFormat);
		else
			printer.setOutputFormat(QPrinter::PostScriptFormat);

		printer.setOutputFileName(path);
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Millimeter);
		printer.setPaperSize( QSizeF(w, h), QPrinter::Millimeter);
		printer.setPageMargins(0,0,0,0, QPrinter::Millimeter);
		printer.setPrintRange(QPrinter::PageRange);
		printer.setCreator( QString("LabPlot ") + LVERSION );

		QPainter painter(&printer);
		painter.setRenderHint(QPainter::Antialiasing);
		QRectF targetRect(0, 0, painter.device()->width(),painter.device()->height());
		painter.begin(&printer);
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();
	}else if (format==WorksheetView::Svg){
		QSvgGenerator generator;
		generator.setFileName(path);
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Millimeter);
		w = w*QApplication::desktop()->physicalDpiX()/25.4;
		h = h*QApplication::desktop()->physicalDpiY()/25.4;

		generator.setSize(QSize(w, h));
		QRectF targetRect(0, 0, w, h);
		generator.setViewBox(targetRect);

		QPainter painter;
		painter.begin(&generator);
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();
	}else{
		//PNG
		//TODO add all formats supported by Qt in QImage
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Millimeter);
		w = w*resolution/25.4;
		h = h*resolution/25.4;
		QImage image(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
		image.fill(Qt::transparent);
		QRectF targetRect(0, 0, w, h);

		QPainter painter;
		painter.begin(&image);
		painter.setRenderHint(QPainter::Antialiasing);
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();

		image.save(path, "png");
	}
}

void WorksheetView::exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect, const bool background) {
	//draw the background
	if (background) {
		painter->save();
		painter->scale(targetRect.width()/sourceRect.width(), targetRect.height()/sourceRect.height());
		drawBackground(painter, sourceRect);
		painter->restore();
	}

	//draw the scene items
	m_worksheet->setPrinting(true);
	scene()->render(painter, QRectF(), sourceRect);
	m_worksheet->setPrinting(false);
}

void WorksheetView::print(QPrinter* printer) const{
	m_worksheet->setPrinting(true);
	QPainter painter(printer);
	painter.setRenderHint(QPainter::Antialiasing);
	scene()->render(&painter);
	m_worksheet->setPrinting(false);
}

void WorksheetView::updateBackground(){
	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}

/*!
 * called when the layout was changed in Worksheet,
 * enables the corresponding action
 */
void WorksheetView::layoutChanged(Worksheet::Layout layout) {
	if (layout==Worksheet::NoLayout){
		verticalLayoutAction->setEnabled(true);
		verticalLayoutAction->setChecked(false);

		horizontalLayoutAction->setEnabled(true);
		horizontalLayoutAction->setChecked(false);

		gridLayoutAction->setEnabled(true);
		gridLayoutAction->setChecked(false);

		breakLayoutAction->setEnabled(false);
	}else{
		verticalLayoutAction->setEnabled(false);
		horizontalLayoutAction->setEnabled(false);
		gridLayoutAction->setEnabled(false);
		breakLayoutAction->setEnabled(true);

		if (layout==Worksheet::VerticalLayout)
			verticalLayoutAction->setChecked(true);
		else if (layout==Worksheet::HorizontalLayout)
			horizontalLayoutAction->setChecked(true);
		else
			gridLayoutAction->setChecked(true);
	}
}


//##############################################################################
//########################  SLOTs for cartesian plots   ########################
//##############################################################################
void WorksheetView::cartesianPlotActionModeChanged(QAction* action) {
	if (action == cartesianPlotApplyToSelectionAction)
		m_cartesianPlotActionMode = ApplyActionToSelection;
	else
		m_cartesianPlotActionMode = ApplyActionToAll;

	handleCartesianPlotActions();
}

void WorksheetView::cartesianPlotMouseModeChanged(QAction* action) {
	if (action==cartesianPlotSelectionModeAction) {
		m_cartesianPlotMouseMode = CartesianPlot::SelectionMode;
	} else if (action==cartesianPlotZoomSelectionModeAction) {
		m_cartesianPlotMouseMode = CartesianPlot::ZoomSelectionMode;
	} else if (action==cartesianPlotZoomXSelectionModeAction) {
		m_cartesianPlotMouseMode = CartesianPlot::ZoomXSelectionMode;
	} else if (action==cartesianPlotZoomYSelectionModeAction) {
		m_cartesianPlotMouseMode = CartesianPlot::ZoomYSelectionMode;
	}

	foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
		plot->setMouseMode(m_cartesianPlotMouseMode);
	}
}

void WorksheetView::addCurve() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->addCurve();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->addCurve();
		}
	}
}

void WorksheetView::addEquationCurve() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->addEquationCurve();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->addEquationCurve();
		}
	}
}

void WorksheetView::addFitCurve() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->addFitCurve();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->addFitCurve();
		}
	}
}

void WorksheetView::addLegend() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->addLegend();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->addLegend();
		}
	}
}

void WorksheetView::addHorizontalAxis() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->addHorizontalAxis();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->addHorizontalAxis();
		}
	}
}

void WorksheetView::addVerticalAxis() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->addVerticalAxis();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->addVerticalAxis();
		}
	}
}

void WorksheetView::scaleAuto() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->scaleAuto();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->scaleAuto();
		}
	}
}

void WorksheetView::scaleAutoX() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->scaleAutoX();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->scaleAutoX();
		}
	}
}

void WorksheetView::scaleAutoY() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->scaleAutoY();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->scaleAutoY();
		}
	}
}

void WorksheetView::zoomIn() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->zoomIn();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->zoomIn();
		}
	}
}

void WorksheetView::zoomOut() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->zoomOut();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->zoomOut();
		}
	}
}

void WorksheetView::zoomInX() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->zoomInX();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->zoomInX();
		}
	}
}

void WorksheetView::zoomOutX() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->zoomOutX();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->zoomOutX();
		}
	}
}

void WorksheetView::zoomInY() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->zoomInY();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->zoomInY();
		}
	}
}

void WorksheetView::zoomOutY() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->zoomOutY();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->zoomOutY();
		}
	}
}

void WorksheetView::shiftLeftX() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->shiftLeftX();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->shiftLeftX();
		}
	}
}

void WorksheetView::shiftRightX() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->shiftRightX();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->shiftRightX();
		}
	}
}

void WorksheetView::shiftUpY() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->shiftUpY();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->shiftUpY();
		}
	}
}

void WorksheetView::shiftDownY() {
	if (m_cartesianPlotActionMode == ApplyActionToSelection) {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			if (m_selectedItems.indexOf(plot->graphicsItem())!=-1)
				plot->shiftDownY();
		}
	} else {
		foreach(CartesianPlot* plot, m_worksheet->children<CartesianPlot>() ){
			plot->shiftDownY();
		}
	}
}
