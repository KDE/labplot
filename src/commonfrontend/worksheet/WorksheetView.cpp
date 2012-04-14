/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2009-2012 Alexander Semke (alexander.semke*web.de)
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
#include <QToolBar>
#include <QDesktopWidget>
#include <QWheelEvent>
#include <QPrinter>
#include <QSvgGenerator>
#include <QImage>
#include <QToolButton>
#include <QDebug>
#include <QMessageBox>

#include "worksheet/WorksheetView.h"
#include "worksheet/Worksheet.h"
#include "worksheet/WorksheetModel.h"
#include "worksheet/WorksheetElementGroup.h"
#include "worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "worksheet/WorksheetRectangleElement.h"
#include "worksheet/plots/cartesian/CartesianPlot.h"
#include "worksheet/plots/cartesian/Axis.h"
#include "worksheet/plots/cartesian/XYCurve.h"
#include "lib/ActionManager.h"
#include "core/column/Column.h"
#include "../../backend/worksheet/TextLabel.h"

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KAction>
#include <KLocale>
#include "kdefrontend/worksheet/GridDialog.h"
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
, m_worksheet(worksheet), m_currentMouseMode(NavigationMode) {
  
  m_model = new WorksheetModel(worksheet);
  setScene(m_model->scene());
  
  	connect(worksheet, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(selectItem(QGraphicsItem*)) ); 
	connect(worksheet, SIGNAL(itemDeselected(QGraphicsItem*)), this, SLOT(deselectItem(QGraphicsItem*)) );
	connect(worksheet, SIGNAL(requestUpdate()), this, SLOT(updateBackground()) );
  
  setRenderHint(QPainter::Antialiasing);
  setInteractive(true);
  setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);
  setMinimumSize(16, 16);
  setFocusPolicy(Qt::StrongFocus);
  

  viewport()->setAttribute( Qt::WA_OpaquePaintEvent );
  viewport()->setAttribute( Qt::WA_NoSystemBackground );
  setAcceptDrops( true );
  setCacheMode(QGraphicsView::CacheBackground);
  
  m_gridSettings.style = WorksheetView::NoGrid;
  
  initActions();
  initMenus();
  navigationModeAction->setChecked(true);
  
  tbZoom=0;
  changeZoom(zoomOriginAction);
  currentZoomAction=zoomInAction;

  connect(m_worksheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
  connect(m_worksheet, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(selectItem(QGraphicsItem*)) ); 
  connect(m_worksheet, SIGNAL(itemDeselected(QGraphicsItem*)), this, SLOT(deselectItem(QGraphicsItem*)) ); 
  connect(scene(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()) );
 }

WorksheetView::~WorksheetView(){
	delete m_model;
}

//! Private ctor for initActionManager() only
WorksheetView::WorksheetView(){
	m_model = NULL;
	initActions();
}


void WorksheetView::initActions(){
	QActionGroup* addNewActionGroup = new QActionGroup(this);
	QActionGroup* zoomActionGroup = new QActionGroup(this);
	QActionGroup* mouseModeActionGroup = new QActionGroup(this);
	QActionGroup* layoutActionGroup = new QActionGroup(this);
	QActionGroup * gridActionGroup = new QActionGroup(this);
	gridActionGroup->setExclusive(true);
	
 #ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
 //Zoom actions
 zoomInAction = new QAction(tr("Zoom in"), this);
 zoomInAction->setShortcut(Qt::CTRL+Qt::Key_Plus);
 
 zoomOutAction = new QAction(tr("Zoom out"), this);
 zoomOutAction->setShortcut(Qt::CTRL+Qt::Key_Minus);
 
 zoomOriginAction = new QAction(tr("Original size"), this);
 zoomOriginAction->setShortcut(Qt::CTRL+Qt::Key_1);
 
 zoomFitPageHeightAction = new QAction(tr("Fit to height"), this);
 
 zoomFitPageWidthAction = new QAction(tr("Fit to width"), this);

 zoomFitSelectionAction = new QAction(tr("Fit to selection"), this);
 
 // Mouse mode actions 
 navigationModeAction = new QAction(tr("Navigation"), navigationModeAction);
 navigationModeAction->setCheckable(true);
 connect(navigationModeAction, SIGNAL(triggered()), SLOT(enableNavigationMode()));
 
 zoomModeAction = new QAction(tr("Zoom"), navigationModeAction);
 zoomModeAction->setCheckable(true);
 connect(zoomModeAction, SIGNAL(triggered()), SLOT(enableZoomMode()));
 
 selectionModeAction = new QAction(tr("Selection"), navigationModeAction);
 selectionModeAction->setCheckable(true);
 connect(selectionModeAction, SIGNAL(triggered()), SLOT(enableSelectionMode()));
 
 //Layout actions
 verticalLayoutAction = new QAction(tr("Vertical layout"), layoutActionGroup);
 verticalLayoutAction->setObjectName("verticalLayoutAction");
 verticalLayoutAction->setCheckable(true);
 
 horizontalLayoutAction = new QAction(tr("Horizontal layout"), layoutActionGroup);
 horizontalLayoutAction->setObjectName("horizontalLayoutAction");
 horizontalLayoutAction->setCheckable(true);
 
 gridLayoutAction = new QAction(tr("Grid layout"), layoutActionGroup);
 gridLayoutAction->setObjectName("gridLayoutAction");
 gridLayoutAction->setCheckable(true);
 
 breakLayoutAction = new QAction(tr("Break layout"), layoutActionGroup);
 breakLayoutAction->setObjectName("breakLayoutAction");
 breakLayoutAction->setEnabled(false);
#else
  //Zoom actions
  zoomInAction = new KAction(KIcon("zoom-in"), i18n("Zoom in"), zoomActionGroup);
  zoomInAction->setShortcut(Qt::CTRL+Qt::Key_Plus);

  zoomOutAction = new KAction(KIcon("zoom-out"), i18n("Zoom out"), zoomActionGroup);
  zoomOutAction->setShortcut(Qt::CTRL+Qt::Key_Minus);

  zoomOriginAction = new KAction(KIcon("zoom-original"), i18n("Original size"), zoomActionGroup);
  zoomOriginAction->setShortcut(Qt::CTRL+Qt::Key_1);

  zoomFitPageHeightAction = new KAction(KIcon("zoom-fit-height"), i18n("Fit to height"), zoomActionGroup);

  zoomFitPageWidthAction = new KAction(KIcon("zoom-fit-width"), i18n("Fit to width"), zoomActionGroup);
  
  zoomFitSelectionAction = new KAction(i18n("Fit to selection"), zoomActionGroup);

  // Mouse mode actions 
  navigationModeAction = new KAction(KIcon("input-mouse"), i18n("Navigation"), mouseModeActionGroup);
  navigationModeAction->setCheckable(true);
  connect(navigationModeAction, SIGNAL(triggered()), SLOT(enableNavigationMode()));

  zoomModeAction = new KAction(KIcon("page-zoom"), i18n("Zoom"), mouseModeActionGroup);
  zoomModeAction->setCheckable(true);
  connect(zoomModeAction, SIGNAL(triggered()), SLOT(enableZoomMode()));

  selectionModeAction = new KAction(KIcon("select-rectangular"), i18n("Selection"), mouseModeActionGroup);
  selectionModeAction->setCheckable(true);
  connect(selectionModeAction, SIGNAL(triggered()), SLOT(enableSelectionMode()));

  //"Add new" related actions
  addPlotAction = new KAction(KIcon("office-chart-line"), i18n("xy-plot"), addNewActionGroup);
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
	noGridAction = new KAction(KIcon(""), i18n("no grid"), gridActionGroup);
	noGridAction->setObjectName("noGridAction");
	noGridAction->setCheckable(true);
	noGridAction->setChecked(true);
	noGridAction->setData(WorksheetView::NoGrid);
	
	denseLineGridAction = new KAction(KIcon(""), i18n("dense line grid"), gridActionGroup);
	denseLineGridAction->setObjectName("denseLineGridAction");
	denseLineGridAction->setCheckable(true);
	
	sparseLineGridAction = new KAction(KIcon(""), i18n("sparse line grid"), gridActionGroup);
	sparseLineGridAction->setObjectName("sparseLineGridAction");
	sparseLineGridAction->setCheckable(true);

	denseDotGridAction = new KAction(KIcon(""), i18n("dense dot grid"), gridActionGroup);
	denseDotGridAction->setObjectName("denseDotGridAction");
	denseDotGridAction->setCheckable(true);
	
	sparseDotGridAction = new KAction(KIcon(""), i18n("sparse dot grid"), gridActionGroup);
	sparseDotGridAction->setObjectName("sparseDotGridAction");
	sparseDotGridAction->setCheckable(true);
	
	customGridAction = new KAction(KIcon(""), i18n("custom grid"), gridActionGroup);
	customGridAction->setObjectName("customGridAction");
	customGridAction->setCheckable(true);
	
	snapToGridAction = new KAction(KIcon(""), i18n("snap to grid"), this);
	//TODO slot
	snapToGridAction->setCheckable(true);
#endif
	connect(addNewActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(addNew(QAction*)));
	connect(zoomActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeZoom(QAction*)));
	connect(layoutActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeLayout(QAction*)));
	connect(gridActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeGrid(QAction*)));
	qDebug()<<"actions done";
}

void WorksheetView::initMenus(){
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	m_addNewMenu = new QMenu(tr("Add new"));
	m_zoomMenu = new QMenu(tr("Zoom"));
	m_layoutMenu = new QMenu(tr("Layout"));
	m_gridMenu = new QMenu(tr("Grid"));
#else
	m_addNewMenu = new QMenu(i18n("Add new"));
	m_zoomMenu = new QMenu(i18n("Zoom"));
	m_layoutMenu = new QMenu(i18n("Layout"));
	m_gridMenu = new QMenu(i18n("Grid"));
	m_gridMenu->setIcon(QIcon(KIcon("view-grid")));
#endif

	m_addNewMenu->addAction(addTextLabelAction);
	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(addPlotAction);
	
	m_zoomMenu->addAction(zoomInAction);
	m_zoomMenu->addAction(zoomOutAction);
	m_zoomMenu->addAction(zoomOriginAction);
	m_zoomMenu->addAction(zoomFitPageHeightAction);
	m_zoomMenu->addAction(zoomFitPageWidthAction);
	m_zoomMenu->addAction(zoomFitSelectionAction);

	m_layoutMenu->addAction(verticalLayoutAction);
	m_layoutMenu->addAction(horizontalLayoutAction);
	m_layoutMenu->addAction(gridLayoutAction); 
	m_layoutMenu->addSeparator();
	m_layoutMenu->addAction(breakLayoutAction);

	m_gridMenu->addAction(noGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(sparseLineGridAction);
	m_gridMenu->addAction(denseLineGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(sparseDotGridAction);
	m_gridMenu->addAction(denseDotGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(customGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(snapToGridAction);
}

void WorksheetView::createContextMenu(QMenu* menu){
  if (!menu)
	menu=new QMenu();
  else
	menu->addSeparator()->setText(tr("Mouse mode"));

	QAction* firstAction = menu->actions().first();
	menu->insertMenu(firstAction, m_addNewMenu);
	menu->insertSeparator(firstAction);
  
	//Mouse mode actions
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, navigationModeAction);
	menu->insertAction(firstAction, zoomModeAction);
	menu->insertAction(firstAction, selectionModeAction);

	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_zoomMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_layoutMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_gridMenu);
}

void WorksheetView::fillProjectMenu(QMenu *menu, bool *rc) {
  Q_UNUSED(rc);
	this->createContextMenu(menu);
}

void WorksheetView::fillToolBar(QToolBar* toolBar){
	toolBar->clear();
	
	toolBar->addSeparator();
	toolBar->addAction(navigationModeAction);
	toolBar->addAction(zoomModeAction);
	toolBar->addAction(selectionModeAction);
	
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	toolBar->addSeparator();
	toolBar->addAction(addPlotAction);
#endif

	toolBar->addSeparator();
	toolBar->addAction(verticalLayoutAction);
	toolBar->addAction(horizontalLayoutAction);
	toolBar->addAction(gridLayoutAction);
	toolBar->addAction(breakLayoutAction);
	
	toolBar->addSeparator();
	QToolButton* b = new QToolButton(toolBar);
	b->setPopupMode(QToolButton::MenuButtonPopup);
	b->setMenu(m_zoomMenu);
	b->setDefaultAction(currentZoomAction);
	toolBar->addWidget(b);
	tbZoom=b;
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
//   painter->setRenderHint(QPainter::Antialiasing);
  QRectF scene_rect = sceneRect();

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

  // canvas
  //TODO add the code PlotArea::paint() that handels the different background options
  painter->fillRect(scene_rect.intersected(rect), m_worksheet->backgroundFirstColor());
  
  //grid
	if (m_gridSettings.style == WorksheetView::NoGrid){
		painter->restore();
		return;
	}else{
		QColor c=m_gridSettings.color;
 		c.setAlphaF(m_gridSettings.opacity);
		painter->setPen(c);

		qreal x, y;
// 		qreal left = rect.left();
// 		qreal right = rect.right();
// 		qreal top = rect.top();
// 		qreal bottom = rect.bottom();
		qreal left = scene_rect.left();
		qreal right = scene_rect.right();
		qreal top = scene_rect.top();
		qreal bottom = scene_rect.bottom();
		
		if (m_gridSettings.style==WorksheetView::LineGrid){
			QLineF line;
			
			//horizontal lines
			y = top + m_gridSettings.verticalSpacing;
			while (y < bottom){
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
  if (m_currentMouseMode == ZoomMode){
	fitInView(scene()->selectionArea().boundingRect(),Qt::KeepAspectRatio);
  }
  QGraphicsView::mouseReleaseEvent(event);
}

void WorksheetView::contextMenuEvent(QContextMenuEvent* e) {
  Q_UNUSED(e)
  QMenu *menu = new QMenu(this);
  this->createContextMenu(menu);
  menu->exec(QCursor::pos());
}


//###################### SLOTS #################
void WorksheetView::changeZoom(QAction* action){
	if (action==zoomInAction){
		scale(1.2, 1.2);
	}else if (action==zoomOutAction){
		scale(1.0/1.2, 1.0/1.2);
	}else if (action==zoomOriginAction){
		float hscale = QApplication::desktop()->physicalDpiX()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		float vscale = QApplication::desktop()->physicalDpiY()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		setTransform(QTransform::fromScale(hscale, vscale));
	}else if (action==zoomFitPageWidthAction){
		float scaleFactor = viewport()->width()/m_model->scene()->sceneRect().width();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
	}else if (action==zoomFitPageHeightAction){
		float scaleFactor = viewport()->height()/m_model->scene()->sceneRect().height();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
	}else if (action==zoomFitSelectionAction){
		fitInView(scene()->selectionArea().boundingRect(),Qt::KeepAspectRatio);
	}
	currentZoomAction=action;
	if (tbZoom)
			tbZoom->setDefaultAction(action);
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

//"Add new" related slots
void WorksheetView::addNew(QAction* action){
	AbstractAspect* aspect = 0;
	if ( action == addPlotAction ){
		aspect = new CartesianPlot("xy-plot");
	}else if ( action == addTextLabelAction ){
		TextLabel* l = new TextLabel("text label");
		l->setText("text label");
		m_worksheet->addChild(l);
		return;
	}

	m_worksheet->addChild(aspect);
}


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
 #ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE		
		//TODO
#else
		GridDialog* dlg = new GridDialog(this);
		if (dlg->exec() == QDialog::Accepted)
			dlg->save(m_gridSettings);
		else
			return;
#endif
	}

	if (m_gridSettings.style == WorksheetView::NoGrid)
		snapToGridAction->setEnabled(false);
	else
		snapToGridAction->setEnabled(true);

	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}

void WorksheetView::selectItem(QGraphicsItem* item){
//   qDebug()<<"view slot"<<item;
// 	item->setZValue(100);
	item->setSelected(true);
// 	scene()->update();
}


void WorksheetView::deselectItem(QGraphicsItem* item){
	item->setSelected(false);
}

//TODO
void WorksheetView::selectionChanged(){
// 	qDebug()<<"selection changed";
// 	qDebug()<<"selection "<<scene()->selectionArea().boundingRect();
 QList<QGraphicsItem *> items = scene()->selectedItems();
//  qDebug()<<items;
	
 //trave
//  QGraphicsItem* item = items.first();
// //  m_worksheet->children<AbstractWorksheetElement*>()
//  foreach(const AbstractAspect * child, aspect->children<AbstractAspect>());
//  
//  child->graphicsItem() != item;
//  
//  this->hasGraphicsItem(child, item)
}

void WorksheetView::exportToFile(const QString& path, const ExportFormat format, const ExportArea area) const{
	QRectF rect;

	if (area==WorksheetView::ExportBoundingBox){
//TODO QRectF rect =scene()->itemsBoundingRect();
		rect =scene()->sceneRect();
	}else if (area==WorksheetView::ExportSelection){
		//TODO  	QRectF rect = scene()->selectionArea().boundingRect();
		
		// 	QList<QGraphicsItem *> items = scene()->selectedItems();
		// 	QGraphicsItem* item = items.first();
		// 	QRectF rect = item->boundingRect();
		
		rect =scene()->sceneRect();
	}else{
		rect =scene()->sceneRect();
	}

	if (format==WorksheetView::Pdf || format==WorksheetView::Eps){
		QPrinter printer(QPrinter::HighResolution);
		if (format==WorksheetView::Pdf)
			printer.setOutputFormat(QPrinter::PdfFormat);
		else
			printer.setOutputFormat(QPrinter::PostScriptFormat);
		
		printer.setOutputFileName(path);
		printer.setPaperSize( QSizeF(rect.width(), rect.height()), QPrinter::Millimeter);
		printer.setPageMargins(0,0,0,0, QPrinter::Millimeter);
		printer.setPrintRange(QPrinter::PageRange);
//TODO		printer.setCreator("LabPlot "+LVERSION);

		QPainter painter(&printer);
		painter. setRenderHint(QPainter::Antialiasing);
		
		double xscale = printer.pageRect().width()/double(rect.width());
		double yscale = printer.pageRect().height()/double(rect.height());
		double scale = qMin(xscale, yscale);
		painter.scale(scale, scale);

		painter.begin(&printer);
		scene()->render(&painter, rect);
		painter.end();
	}else if (format==WorksheetView::Svg){
		QSvgGenerator generator;
		generator.setFileName(path);
		generator.setSize(QSize(rect.width(), rect.height()));
		generator.setViewBox(rect);

		QPainter painter;
		painter.begin(&generator);
		scene()->render(&painter, rect);
		painter.end();
	}else{
		//PNG
		//TODO add all formats supported by Qt in QImage
		//TODO make the size of the image customizable by the user in the ExportWorksheetDialog
		int w = rect.width()*QApplication::desktop()->physicalDpiX()/25.4;
		int h = rect.height()*QApplication::desktop()->physicalDpiY()/25.4;
		QImage image(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
		
		QPainter painter;
		painter.begin(&image);
		painter.scale(QApplication::desktop()->physicalDpiX()/25.4,QApplication::desktop()->physicalDpiY()/25.4);
		scene()->render(&painter, rect);
		painter.end();

		 image.save(path, "png");
	}
}

void WorksheetView::print(QPrinter* printer) const{
	QRectF rect =scene()->sceneRect();
		
	QPainter painter(printer);
	painter. setRenderHint(QPainter::Antialiasing);
	
	double xscale = printer->pageRect().width()/double(rect.width());
	double yscale = printer->pageRect().height()/double(rect.height());
	double scale = qMin(xscale, yscale);
	painter.scale(scale, scale);

	scene()->render(&painter, rect);
}

void WorksheetView::updateBackground(){
	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}
