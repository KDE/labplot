/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2010 by Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)

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

#include "worksheet/WorksheetView.h"
#include "worksheet/Worksheet.h"
#include "worksheet/WorksheetModel.h"
#include "worksheet/WorksheetElementGroup.h"
#include "worksheet/DecorationPlot.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "worksheet/WorksheetRectangleElement.h"
#include "worksheet/LinearAxis.h"
#include "worksheet/LogAxis.h"
#include "worksheet/LineSymbolCurve.h"
#include "worksheet/DropLineCurve.h"
#include "worksheet/PlotArea.h"
#include "lib/ActionManager.h"
#include "core/column/Column.h"

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KAction>
#include <KLocale>
#endif

/**
 * \class WorksheetView
 * \brief Worksheet view
 *
 *
 */

/*!
  Constructur of the class.
  Creates a view for the Worksheet \c worksheet and initializes the internal model.
*/
WorksheetView::WorksheetView(Worksheet *worksheet) : QGraphicsView()
, m_worksheet(worksheet), m_scaleFactor(1.0){
  
  m_model = new WorksheetModel(worksheet);

  m_scaleFactorUpperLimit = 100;
  m_scaleFactorLowerLimit = 0.1;

  setScene(m_model->scene());
  connect(this, SIGNAL(scaleFactorChanged(qreal)), this, SLOT(handleScaleFactorChange(qreal)));

  setRenderHint(QPainter::Antialiasing);
  setInteractive(true);
  setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);
  setMinimumSize(16, 16);
  setFocusPolicy(Qt::StrongFocus);
//   setFocus();

  viewport()->setAttribute( Qt::WA_OpaquePaintEvent );
  viewport()->setAttribute( Qt::WA_NoSystemBackground );
  setAcceptDrops( true );
  
  createActions();
  navigationModeAction->setChecked(true);

  // TODO: remove the test code later
  QTimer::singleShot(0, this, SLOT(startTestCode()));
 }

WorksheetView::~WorksheetView(){
	delete m_model;
}

//! Private ctor for initActionManager() only
WorksheetView::WorksheetView(){
	m_model = NULL;
	createActions();
}


void WorksheetView::createActions() {
 #ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
 //Zoom actions
 zoomInAction = new QAction(tr("Zoom in"), this);
 zoomInAction->setShortcut(Qt::CTRL+Qt::Key_Plus);
 connect(zoomInAction, SIGNAL(triggered()), SLOT(zoomIn()));
 
 zoomOutAction = new QAction(tr("Zoom out"), this);
 zoomOutAction->setShortcut(Qt::CTRL+Qt::Key_Minus);
 connect(zoomOutAction, SIGNAL(triggered()), SLOT(zoomOut()));
 
 zoomOriginAction = new QAction(tr("Original size"), this);
 zoomOriginAction->setShortcut(Qt::CTRL+Qt::Key_1);
 connect(zoomOriginAction, SIGNAL(triggered()), SLOT(zoomOrigin()));
 
 zoomFitPageHeightAction = new QAction(tr("Fit to height"), this);
 connect(zoomFitPageHeightAction, SIGNAL(triggered()), SLOT(zoomFitPageHeight()));
 
 zoomFitPageWidthAction = new QAction(tr("Fit to width"), this);
 connect(zoomFitPageWidthAction, SIGNAL(triggered()), SLOT(zoomFitPageWidth()));
 
 // Mouse mode actions 
 QActionGroup* mouseModeActionGroup = new QActionGroup(this);
 navigationModeAction = new QAction(tr("Navigation"), this);
 navigationModeAction->setCheckable(true);
 mouseModeActionGroup->addAction(navigationModeAction);
 connect(navigationModeAction, SIGNAL(triggered()), SLOT(enableNavigationMode()));
 
 zoomModeAction = new QAction(tr("Zoom"), this);
 zoomModeAction->setCheckable(true);
 mouseModeActionGroup->addAction(zoomModeAction);
 connect(zoomModeAction, SIGNAL(triggered()), SLOT(enableZoomMode()));
 
 selectionModeAction = new QAction(tr("Selection"), this);
 selectionModeAction->setCheckable(true);
 mouseModeActionGroup->addAction(selectionModeAction);
 connect(selectionModeAction, SIGNAL(triggered()), SLOT(enableSelectionMode()));
 
 //Layout actions
 QActionGroup* layoutActionGroup = new QActionGroup(this);
 
 verticalLayoutAction = new QAction(tr("Vertical layout"), this);
 verticalLayoutAction->setObjectName("verticalLayoutAction");
 verticalLayoutAction->setCheckable(true);
 layoutActionGroup->addAction(verticalLayoutAction);
 
 horizontalLayoutAction = new QAction(tr("Horizontal layout"), this);
 horizontalLayoutAction->setObjectName("horizontalLayoutAction");
 horizontalLayoutAction->setCheckable(true);
 layoutActionGroup->addAction(horizontalLayoutAction);
 
 gridLayoutAction = new QAction(tr("Grid layout"), this);
 gridLayoutAction->setObjectName("gridLayoutAction");
 gridLayoutAction->setCheckable(true);
 layoutActionGroup->addAction(gridLayoutAction);
 
 breakLayoutAction = new QAction(tr("Break layout"), this);
 breakLayoutAction->setObjectName("breakLayoutAction");
 breakLayoutAction->setEnabled(false);
 layoutActionGroup->addAction(breakLayoutAction);
#else
  //Zoom actions
  zoomInAction = new KAction(KIcon("zoom-in"), i18n("Zoom in"), this);
  zoomInAction->setShortcut(Qt::CTRL+Qt::Key_Plus);
  connect(zoomInAction, SIGNAL(triggered()), SLOT(zoomIn()));

  zoomOutAction = new KAction(KIcon("zoom-out"), i18n("Zoom out"), this);
  zoomOutAction->setShortcut(Qt::CTRL+Qt::Key_Minus);
  connect(zoomOutAction, SIGNAL(triggered()), SLOT(zoomOut()));

  zoomOriginAction = new KAction(KIcon("zoom-original"), i18n("Original size"), this);
  zoomOriginAction->setShortcut(Qt::CTRL+Qt::Key_1);
  connect(zoomOriginAction, SIGNAL(triggered()), SLOT(zoomOrigin()));

  zoomFitPageHeightAction = new KAction(KIcon("zoom-fit-height"), i18n("Fit to height"), this);
  connect(zoomFitPageHeightAction, SIGNAL(triggered()), SLOT(zoomFitPageHeight()));

  zoomFitPageWidthAction = new KAction(KIcon("zoom-fit-width"), i18n("Fit to width"), this);
  connect(zoomFitPageWidthAction, SIGNAL(triggered()), SLOT(zoomFitPageWidth()));

  // Mouse mode actions 
  QActionGroup* mouseModeActionGroup = new QActionGroup(this);
  navigationModeAction = new KAction(KIcon("input-mouse"), i18n("Navigation"), this);
  navigationModeAction->setCheckable(true);
  mouseModeActionGroup->addAction(navigationModeAction);
  connect(navigationModeAction, SIGNAL(triggered()), SLOT(enableNavigationMode()));

  zoomModeAction = new KAction(KIcon("page-zoom"), i18n("Zoom"), this);
  zoomModeAction->setCheckable(true);
  mouseModeActionGroup->addAction(zoomModeAction);
  connect(zoomModeAction, SIGNAL(triggered()), SLOT(enableZoomMode()));

  selectionModeAction = new KAction(KIcon("select-rectangular"), i18n("Selection"), this);
  selectionModeAction->setCheckable(true);
  mouseModeActionGroup->addAction(selectionModeAction);
  connect(selectionModeAction, SIGNAL(triggered()), SLOT(enableSelectionMode()));

  //Layout actions
  QActionGroup* layoutActionGroup = new QActionGroup(this);

  verticalLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Vertical layout"), this);
  verticalLayoutAction->setObjectName("verticalLayoutAction");
  verticalLayoutAction->setCheckable(true);
  layoutActionGroup->addAction(verticalLayoutAction);

  horizontalLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Horizontal layout"), this);
  horizontalLayoutAction->setObjectName("horizontalLayoutAction");
  horizontalLayoutAction->setCheckable(true);
  layoutActionGroup->addAction(horizontalLayoutAction);

  gridLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Grid layout"), this);
  gridLayoutAction->setObjectName("gridLayoutAction");
  gridLayoutAction->setCheckable(true);
  layoutActionGroup->addAction(gridLayoutAction);

  breakLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Break layout"), this);
  breakLayoutAction->setObjectName("breakLayoutAction");
  breakLayoutAction->setEnabled(false);
  layoutActionGroup->addAction(breakLayoutAction);
  #endif

  connect(layoutActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(layout(QAction*)));
}

void WorksheetView::createMenu(QMenu *menu) const{
  if (!menu)
	menu=new QMenu();
  
  //Zoom actions
  menu->addAction(zoomInAction);
  menu->addAction(zoomOutAction);
  menu->addAction(zoomOriginAction);
  menu->addAction(zoomFitPageHeightAction);
  menu->addAction(zoomFitPageWidthAction);
  
  //Mouse mode actions
  menu->addSeparator();//->setText( i18n("Mouse mode") );
  menu->addAction(navigationModeAction);
  menu->addAction(zoomModeAction);
  menu->addAction(selectionModeAction);
  
  //Layout actions
  menu->addSeparator();//->setText( i18n("Layout") );
  menu->addAction(verticalLayoutAction);
  menu->addAction(horizontalLayoutAction);
  menu->addAction(gridLayoutAction);  
  menu->addAction(breakLayoutAction);
}

void WorksheetView::createContextMenu(QMenu *menu) {
// TODO
//   menu->addItem("Zoom");
}

void WorksheetView::fillProjectMenu(QMenu *menu, bool *rc) {
// TODO
}


void WorksheetView::startTestCode() {
  QRectF pageRect = m_model->scene()->sceneRect();
  
  #define SCALE_MIN CartesianCoordinateSystem::Scale::LIMIT_MIN
  #define SCALE_MAX CartesianCoordinateSystem::Scale::LIMIT_MAX
  
  const double pw = 210/25.4*QApplication::desktop()->physicalDpiX();
  const double ph = 297/25.4*QApplication::desktop()->physicalDpiY();
  
  {
	DecorationPlot *plot = new DecorationPlot("plot1");
	m_worksheet->addChild(plot);
	CartesianCoordinateSystem *coordSys = new CartesianCoordinateSystem("coords1");
	
	QList<CartesianCoordinateSystem::Scale *> scales;
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, 2), pw * 0.3, pw * 0.4, -2, 2);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(3, 6), pw * 0.4 + 2, pw * 0.65, 3, 6);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(7, SCALE_MAX), pw * 0.65 + 5, pw * 0.7, 7, 10);
	coordSys->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, 4), ph * 0.8, ph * 0.65, 1, 4);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(8, SCALE_MAX), ph * 0.65 - 5, ph * 0.5, 8, 10);
	coordSys->setYScales(scales);
	
	plot->addChild(coordSys);
	
	PlotArea *plotArea = new PlotArea("plot area");
	plotArea->setRect(QRectF(-2, 0, 12, 10));
	plotArea->setClippingEnabled(true);
	coordSys->addChild(plotArea);
	
	WorksheetRectangleElement *rect = new WorksheetRectangleElement("rect1");
	rect->setRect(QRectF(12, 12, 300, 3));
	coordSys->addChild(rect);
	WorksheetRectangleElement *rect2 = new WorksheetRectangleElement("rect2");
	rect2->setRect(QRectF(0, 0, 40, 30));
	m_worksheet->addChild(rect2);
	WorksheetRectangleElement *rect3 = new WorksheetRectangleElement("rect3");
	rect3->setRect(QRectF(pageRect.width() / 2 - 2, pageRect.height() / 2 - 2, 10 + 4, 120 + 4));
	m_worksheet->addChild(rect3);
	
	WorksheetElementGroup *group1 = new WorksheetElementGroup("some items");
	//	group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(5, 5, 20, 20)));
	//	group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(4, 5, 25, 15)));
	//	group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(5, 3, 26, 25)));
	plotArea->addChild(group1);
	
	LinearAxis *xAxis1 = new LinearAxis("x axis 1", LinearAxis::axisBottom);
	plot->addChild(xAxis1);
	LinearAxis *yAxis1 = new LinearAxis("y axis 1", LinearAxis::axisLeft);
	plot->addChild(yAxis1);
	
	LinearAxis *xAxis2 = new LinearAxis("x axis 1", LinearAxis::axisBottom);
	coordSys->addChild(xAxis2);
	xAxis2->setMajorTicksLength(3);
	xAxis2->setMinorTicksLength(1);
	xAxis2->setMinorTickCount(3);
	xAxis2->setMajorTickCount(13);
	xAxis2->setStart(-2);
	xAxis2->setEnd(10);
	xAxis2->setTickStart(-2);
	xAxis2->setTickEnd(10);
	
	LinearAxis *yAxis2 = new LinearAxis("y axis 1", LinearAxis::axisLeft);
	yAxis2->setMajorTicksLength(3);
	yAxis2->setMinorTicksLength(1);
	yAxis2->setMinorTickCount(4);
	yAxis2->setStart(0);
	yAxis2->setEnd(10);
	yAxis2->setTickStart(0);
	yAxis2->setTickEnd(10);
	yAxis2->setMajorTickCount(11);
	coordSys->addChild(yAxis2);
	
	LinearAxis *xAxis3 = new LinearAxis("x axis 2", LinearAxis::axisTop);
	xAxis3->setOffset(10);
	xAxis3->setStart(-2);
	xAxis3->setEnd(10);
	xAxis3->setTickStart(-2);
	xAxis3->setTickEnd(10);
	xAxis3->setMajorTickCount(13);
	coordSys->addChild(xAxis3);
	LinearAxis *yAxis3 = new LinearAxis("y axis 2", LinearAxis::axisRight);
	yAxis3->setOffset(10);
	yAxis3->setStart(0);
	yAxis3->setEnd(10);
	yAxis3->setTickStart(0);
	yAxis3->setTickEnd(10);
	yAxis3->setMajorTickCount(11);
	yAxis3->setMajorTicksDirection(LinearAxis::ticksBoth);
	yAxis3->setMinorTicksDirection(LinearAxis::ticksBoth);
	#if 0
	yAxis3->setTickStart(0.5);
	yAxis3->setTickEnd(9.5);
	yAxis3->setMajorTickCount(9);
	#endif
	yAxis3->setStart(0);
	yAxis3->setEnd(10);
	yAxis3->setTickStart(0);
	yAxis3->setTickEnd(10);
	coordSys->addChild(yAxis3);
		plotArea->addChild(new WorksheetRectangleElement("rect 1", QRectF(2, 2, 2, 2)));
	
	Column *xc = new Column("xc", SciDAVis::Numeric);
	Column *yc = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<20; i++)	{
	  xc->setValueAt(i, i*0.25);
	  yc->setValueAt(i, i*i*0.01+1);
	}
	
	LineSymbolCurve *curve1 = new LineSymbolCurve("curve 1");
	curve1->setXColumn(xc);
	curve1->setYColumn(yc);
	
	Column *xc2 = new Column("xc", SciDAVis::Numeric);
	Column *yc2 = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<40; i++)	{
	  xc2->setValueAt(i, (i-20)*0.25);
	  yc2->setValueAt(i, (i-20)*(i-20)*0.01/2+2);
	}
	LineSymbolCurve *curve2 = new LineSymbolCurve("curve 2");
	curve2->setXColumn(xc2);
	curve2->setYColumn(yc2);
	
	Column *xc3 = new Column("xc", SciDAVis::Numeric);
	Column *yc3 = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<20; i++)	{
	  xc3->setValueAt(i, i*0.25);
	  yc3->setValueAt(i, i*i*0.01*2+3);
	}
	DropLineCurve *curve3 = new DropLineCurve("curve 3");
	curve3->setXColumn(xc3);
	curve3->setYColumn(yc3);
	
	WorksheetElementContainer *group2 = new WorksheetElementContainer("some more items");
	group2->addChild(curve3);
	plotArea->addChild(group2);
	
	
	plotArea->addChild(curve2);
	plotArea->addChild(curve1);
	
// 		coordSys->addChild(plotArea);
// 		plot->addChild(coordSys);
  }
  
  // ####################
  
  {
	DecorationPlot *plot = new DecorationPlot("plot2");
	m_worksheet->addChild(plot);
	CartesianCoordinateSystem *coordSys = new CartesianCoordinateSystem("coords2");
	
// 	const double pw = 210;
// 	const double ph = 297;
	QList<CartesianCoordinateSystem::Scale *> scales;
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), pw * 0.3, pw * 0.7, -2, 10);
	coordSys->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX), ph * 0.4, ph * 0.2, 1, 1000, 10);
	coordSys->setYScales(scales);
	
	plot->addChild(coordSys);
	PlotArea *plotArea = new PlotArea("plot area");
	plotArea->setRect(QRectF(-2, 1, 12, 1000));
	plotArea->setClippingEnabled(true);
	coordSys->addChild(plotArea);
	
	LinearAxis *xAxis2 = new LinearAxis("x axis 1", LinearAxis::axisBottom);
	xAxis2->setOffset(1);
	coordSys->addChild(xAxis2);
	xAxis2->setMajorTicksLength(3);
	xAxis2->setMinorTicksLength(1);
	xAxis2->setMinorTickCount(3);
	LogAxis *yAxis2 = new LogAxis("y axis 1", LinearAxis::axisLeft, 10);
	yAxis2->setMajorTicksLength(5);
	yAxis2->setMinorTicksLength(3);
	yAxis2->setMinorTickCount(9);
	yAxis2->setMajorTickCount(4);
	yAxis2->setStart(1);
	yAxis2->setEnd(1000);
	yAxis2->setTickStart(1);
	yAxis2->setTickEnd(1000);
	coordSys->addChild(yAxis2);
	
	LinearAxis *xAxis3 = new LinearAxis("x axis 2", LinearAxis::axisTop);
	xAxis3->setOffset(1000);
	coordSys->addChild(xAxis3);
	LinearAxis *yAxis3 = new LinearAxis("y axis 2", LinearAxis::axisRight);
	yAxis3->setOffset(10);
	yAxis3->setMajorTicksLength(5);
	yAxis3->setMinorTicksLength(3);
	yAxis3->setMajorTicksDirection(LinearAxis::ticksBoth);
	yAxis3->setMinorTicksDirection(LinearAxis::ticksBoth);
	yAxis3->setStart(1);
	yAxis3->setEnd(1000);
	yAxis3->setTickStart(1);
	yAxis3->setTickEnd(1000);
	yAxis3->setMinorTickCount(9);
	yAxis3->setMajorTickCount(4);
	coordSys->addChild(yAxis3);
	
	Column *xc = new Column("xc", SciDAVis::Numeric);
	Column *yc = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<20; i++)	{
	  xc->setValueAt(i, i*0.25);
	  yc->setValueAt(i, i*i*2+1);
  }
  
  LineSymbolCurve *curve1 = new LineSymbolCurve("curve 1");
  curve1->setXColumn(xc);
  curve1->setYColumn(yc);
  
  Column *xc2 = new Column("xc", SciDAVis::Numeric);
  Column *yc2 = new Column("yc", SciDAVis::Numeric);
  for (int i=0; i<40; i++)	{
	  xc2->setValueAt(i, (i-20)*0.25);
	  yc2->setValueAt(i, (i-20)*(i-20)+2);
  }
  LineSymbolCurve *curve2 = new LineSymbolCurve("curve 2");
  curve2->setXColumn(xc2);
  curve2->setYColumn(yc2);
  
  Column *xc3 = new Column("xc", SciDAVis::Numeric);
  Column *yc3 = new Column("yc", SciDAVis::Numeric);
  for (int i=0; i<20; i++)	{
	  xc3->setValueAt(i, i*0.25);
	  yc3->setValueAt(i, i*i*6+3);
  }
  LineSymbolCurve *curve3 = new LineSymbolCurve("curve 3");
  curve3->setXColumn(xc3);
  curve3->setYColumn(yc3);
  
  WorksheetElementContainer *group2 = new WorksheetElementContainer("some more items");
  group2->addChild(curve3);
  plotArea->addChild(group2);
  
  
  plotArea->addChild(curve2);
  plotArea->addChild(curve1);
  }
  
}

void WorksheetView::handleScaleFactorChange(qreal factor) {
	qreal percent = factor*100.0;
	emit statusInfo(tr("Scale: %1%").arg(qRound(percent)));
}

/* ========================= static methods ======================= */
ActionManager *WorksheetView::action_manager = 0;

ActionManager *WorksheetView::actionManager(){
	if (!action_manager)
		initActionManager();

	return action_manager;
}

void WorksheetView::initActionManager(){
	if (!action_manager)
		action_manager = new ActionManager();

	action_manager->setTitle(tr("Worksheet"));
	volatile WorksheetView *action_creator = new WorksheetView(); // initialize the action texts
	delete action_creator;
}

void WorksheetView::setScene(QGraphicsScene * scene) {
  QGraphicsView::setScene(scene);
  setSceneRect(scene->sceneRect());
  setScaleFactor(1.0);
  
//   connect(scene, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(itemSelected(QGraphicsItem*)));
}

void WorksheetView::drawBackground(QPainter * painter, const QRectF & rect) {
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);
  QRectF scene_rect = sceneRect();

  // background
  if (!scene_rect.contains(rect))
	painter->fillRect(rect, Qt::lightGray);

  //shadow
  int shadowSize = scene_rect.width()*0.01; 	
  QRectF rightShadowRect(scene_rect.right(), scene_rect.top() + shadowSize,
									  shadowSize, scene_rect.height());
  QRectF bottomShadowRect(scene_rect.left() + shadowSize, scene_rect.bottom(),
									  scene_rect.width(), shadowSize);
												  
  painter->fillRect(rightShadowRect.intersected(rect), Qt::darkGray);
  painter->fillRect(bottomShadowRect.intersected(rect), Qt::darkGray);

  // canvas
  painter->fillRect(scene_rect.intersected(rect), Qt::green);
  painter->restore();
}


void WorksheetView::setScaleFactor(qreal factor) {
  if (m_scaleFactor < m_scaleFactorLowerLimit){
	m_scaleFactor = m_scaleFactorLowerLimit;
	return;
  }

  if (m_scaleFactor > m_scaleFactorUpperLimit){
	m_scaleFactor = m_scaleFactorUpperLimit;
	return;
  }
	
  m_scaleFactor = factor;
  setTransform(QTransform(QMatrix().scale(m_scaleFactor, m_scaleFactor)));
  emit scaleFactorChanged(factor);
}


//#################### EVENTS #################
void WorksheetView::wheelEvent(QWheelEvent *event) {
  if (m_currentMouseMode == ZoomMode){
	if (event->delta() > 0)
	  setScaleFactor(m_scaleFactor * 1.2);
	else if (event->delta() < 0)
	  setScaleFactor(m_scaleFactor / 1.2);
  }else{
	QGraphicsView::wheelEvent(event);
  }
}

void WorksheetView::mouseReleaseEvent (QMouseEvent * event){
  if (m_currentMouseMode == ZoomMode){
	fitInView(scene()->selectionArea().boundingRect(),Qt::KeepAspectRatio);
  }
  QGraphicsView::mouseReleaseEvent(event);
}

void WorksheetView::contextMenuEvent(QContextMenuEvent* e) {
  Q_UNUSED(e)
  QMenu *menu = new QMenu(this);
  this->createMenu(menu);
  menu->exec(QCursor::pos());
}


//###################### SLOTS #################
void WorksheetView::zoomIn(){
  setScaleFactor(m_scaleFactor*1.2);
  
  if (m_scaleFactor == m_scaleFactorUpperLimit){
	zoomInAction->setEnabled(false);
  }else{
	zoomInAction->setEnabled(true);
	zoomOutAction->setEnabled(true);
  }
}

void WorksheetView::zoomOut(){
  setScaleFactor(m_scaleFactor/1.2);
  
  if (m_scaleFactor == m_scaleFactorLowerLimit){
	zoomOutAction->setEnabled(false);
  }else{
	zoomOutAction->setEnabled(true);
	zoomInAction->setEnabled(true);
  }
}

void WorksheetView::zoomOrigin(){
  setScaleFactor(1.0);
  zoomInAction->setEnabled(true);
  zoomOutAction->setEnabled(true);
}

void WorksheetView::zoomFitPageWidth(){
  float scaleFactor = viewport()->width()/m_model->scene()->sceneRect().width();
  setScaleFactor(scaleFactor);
}

void WorksheetView::zoomFitPageHeight(){
  float scaleFactor = viewport()->height()/m_model->scene()->sceneRect().height();
  setScaleFactor(scaleFactor);
}

void WorksheetView::enableNavigationMode(){
  m_currentMouseMode = NavigationMode;
  setDragMode(QGraphicsView::ScrollHandDrag);
}

void WorksheetView::enableZoomMode(){
  m_currentMouseMode = ZoomMode;
  setDragMode(QGraphicsView::RubberBandDrag);
}

void WorksheetView::enableSelectionMode(){
  m_currentMouseMode = SelectionMode;
  setDragMode(QGraphicsView::RubberBandDrag);
}

void WorksheetView::layout(QAction* action){
  QString name = action->objectName();
  if (name == "breakLayoutAction"){
	qDebug()<<"breakLayoutAction";
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
	
	
	// 	QGraphicsLayout* layout;
	if (name == "verticalLayoutAction"){
// 	  QGraphicsLinearLayout* layout=new QGraphicsLinearLayout(Qt::Vertical);
	  
	  // 	  QList<QGraphicsItem *>   items=m_viewWidget->scene()->items();
	  // 	  for (int i=0; i<items.size(); i++){
 //  		layout->addItem(items[1]);
 // 	  }
 
// 	QGraphicsWidget *textEdit = scene()->addWidget(new QTextEdit);
// 	QGraphicsWidget *pushButton = scene()->addWidget(new QPushButton);
// 	
// 	layout->addItem(textEdit);
// 	layout->addItem(pushButton);
// 	
// 	
// 	QGraphicsWidget *form = new QGraphicsWidget;
// 	form->setLayout(layout);
// 	scene()->addItem(form);
	}else if (name == "horizontalLayoutAction"){
// 	  QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::l);
	}else{
// 	  QGraphicsGridLayout* layout = new QGraphicsGridLayout();
	}
  }
}
