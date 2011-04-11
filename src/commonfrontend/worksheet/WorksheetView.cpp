/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2010 Alexander Semke (alexander.semke*web.de)
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
#include <QApplication>
#include <QMenu>
#include <QDesktopWidget>
#include <QTimer> //TODO: remove this later
#include <QWheelEvent>
#include <QDebug>

#include "worksheet/WorksheetView.h"
#include "worksheet/Worksheet.h"
#include "worksheet/WorksheetModel.h"
#include "worksheet/WorksheetElementGroup.h"
#include "worksheet/DecorationPlot.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "worksheet/WorksheetRectangleElement.h"
#include "worksheet/Axis.h"
#include "worksheet/LineSymbolCurve.h"
#include "worksheet/PlotArea.h"
#include "lib/ActionManager.h"
#include "core/column/Column.h"

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KAction>
#include <KLocale>
#endif

#include "worksheettestcode.h"

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
, m_worksheet(worksheet) {
  
  m_model = new WorksheetModel(worksheet);

  setScene(m_model->scene());

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
  
  connect(m_worksheet, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(selectItem(QGraphicsItem*)) ); 
  connect(m_worksheet, SIGNAL(itemDeselected(QGraphicsItem*)), this, SLOT(deselectItem(QGraphicsItem*)) ); 
  connect(scene(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()) );
  
  // TODO: remove the test code later
//    QTimer::singleShot(0, this, SLOT(startTestCode()));
startTestCode();
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

 zoomFitSelectionAction = new QAction(tr("Fit to selection"), this);
 connect(zoomFitSelectionAction, SIGNAL(triggered()), SLOT(zoomFitSelection()));
 
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
  
  zoomFitSelectionAction = new KAction(i18n("Fit to selection"), this);
  connect(zoomFitSelectionAction, SIGNAL(triggered()), SLOT(zoomFitSelection()));

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
  menu->addAction(zoomFitSelectionAction);
  
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
  Q_UNUSED(menu);
// TODO
//   menu->addItem("Zoom");
}

void WorksheetView::fillProjectMenu(QMenu *menu, bool *rc) {
  Q_UNUSED(menu);
  Q_UNUSED(rc);
// TODO
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
  setTransform(QTransform());
}

void WorksheetView::drawBackground(QPainter * painter, const QRectF & rect) {
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);
//   QRectF scene_rect = mapFromScene(sceneRect()).boundingRect();
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
  painter->fillRect(scene_rect.intersected(rect), Qt::white);
  painter->restore();
}


//#################### EVENTS #################
void WorksheetView::wheelEvent(QWheelEvent *event) {
  if (m_currentMouseMode == ZoomMode){
	if (event->delta() > 0)
		scale(1.2, 1.2);
	else if (event->delta() < 0)
		scale(1.0/1.2, 1.0/1.2);
  }else{
	QGraphicsView::wheelEvent(event);
  }
}

void WorksheetView::mouseReleaseEvent (QMouseEvent * event){
//   qDebug()<<scene()->selectedItems();
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
	scale(1.2, 1.2);
}

void WorksheetView::zoomOut(){
	scale(1.0/1.2, 1.0/1.2);
}

void WorksheetView::zoomOrigin(){
  setTransform(QTransform::fromScale(QApplication::desktop()->physicalDpiX()/25.4,
			  QApplication::desktop()->physicalDpiY()/25.4));
}

void WorksheetView::zoomFitPageWidth(){
  float scaleFactor = viewport()->width()/m_model->scene()->sceneRect().width();
  setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
}

void WorksheetView::zoomFitPageHeight(){
  float scaleFactor = viewport()->height()/m_model->scene()->sceneRect().height();
  setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
}

void WorksheetView::zoomFitSelection(){
  qDebug()<<scene()->selectionArea().boundingRect();
	qDebug()<<scene()->selectedItems();
	fitInView(scene()->selectionArea().boundingRect(),Qt::KeepAspectRatio);
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


void WorksheetView::selectItem(QGraphicsItem* item){
  qDebug()<<"view slot"<<item;
	scene()->clearSelection();
	item->setSelected(true);
// QPainterPath path;
// path.addRect(item->boundingRect());
//   scene()->setSelectionArea(path, QTransform());
}


void WorksheetView::deselectItem(QGraphicsItem* item){
	item->setSelected(false);
}

//TODO
void WorksheetView::selectionChanged(){
//  QList<QGraphicsItem *> selected= scene()->selectedItems();
}

