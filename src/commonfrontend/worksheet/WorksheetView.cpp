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
#include <QWheelEvent>
#include <QPrinter>
#include <QSvgGenerator>
#include <QImage>
#include <QDebug>

#include "worksheet/WorksheetView.h"
#include "worksheet/Worksheet.h"
#include "worksheet/WorksheetModel.h"
#include "worksheet/WorksheetElementGroup.h"
#include "worksheet/DecorationPlot.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "worksheet/WorksheetRectangleElement.h"
#include "worksheet/Axis.h"
#include "worksheet/XYCurve.h"
#include "worksheet/PlotArea.h"
#include "lib/ActionManager.h"
#include "core/column/Column.h"

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KAction>
#include <KLocale>
#include "kdefrontend/GridDialog.h"
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
, m_worksheet(worksheet) {
  
  m_model = new WorksheetModel(worksheet);

  	connect(worksheet, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(selectItem(QGraphicsItem*)) ); 
	connect(worksheet, SIGNAL(itemDeselected(QGraphicsItem*)), this, SLOT(deselectItem(QGraphicsItem*)) );
	
	
  setScene(m_model->scene());

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
	QActionGroup* mouseModeActionGroup = new QActionGroup(this);
	QActionGroup* layoutActionGroup = new QActionGroup(this);
	QActionGroup * gridActionGroup = new QActionGroup(this);
	gridActionGroup->setExclusive(true);
	
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
  navigationModeAction = new KAction(KIcon("input-mouse"), i18n("Navigation"), mouseModeActionGroup);
  navigationModeAction->setCheckable(true);
  connect(navigationModeAction, SIGNAL(triggered()), SLOT(enableNavigationMode()));

  zoomModeAction = new KAction(KIcon("page-zoom"), i18n("Zoom"), mouseModeActionGroup);
  zoomModeAction->setCheckable(true);
  connect(zoomModeAction, SIGNAL(triggered()), SLOT(enableZoomMode()));

  selectionModeAction = new KAction(KIcon("select-rectangular"), i18n("Selection"), mouseModeActionGroup);
  selectionModeAction->setCheckable(true);
  connect(selectionModeAction, SIGNAL(triggered()), SLOT(enableSelectionMode()));

  //Layout actions
  verticalLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Vertical layout"), layoutActionGroup);
  verticalLayoutAction->setObjectName("verticalLayoutAction");
  verticalLayoutAction->setCheckable(true);

  horizontalLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Horizontal layout"), layoutActionGroup);
  horizontalLayoutAction->setObjectName("horizontalLayoutAction");
  horizontalLayoutAction->setCheckable(true);

  gridLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Grid layout"), layoutActionGroup);
  gridLayoutAction->setObjectName("gridLayoutAction");
  gridLayoutAction->setCheckable(true);

  breakLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Break layout"), layoutActionGroup);
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

  connect(layoutActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeLayout(QAction*)));
  connect(gridActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeGrid(QAction*)));
}

void WorksheetView::initMenus(){
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	m_zoomMenu = new QMenu(tr("Zoom"));
	m_layoutMenu = new QMenu(tr("Layout"));
	m_gridMenu = new QMenu(tr("Grid"));
#else
	m_zoomMenu = new QMenu(i18n("Zoom"));
	m_layoutMenu = new QMenu(i18n("Layout"));
	m_gridMenu = new QMenu(i18n("Grid"));
	m_gridMenu->setIcon(QIcon(KIcon("view-grid")));
#endif
	
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

void WorksheetView::createMenu(QMenu *menu) const{
  if (!menu)
	menu=new QMenu();
  
  //Mouse mode actions
  menu->addSeparator();//->setText( i18n("Mouse mode") );
  menu->addAction(navigationModeAction);
  menu->addAction(zoomModeAction);
  menu->addAction(selectionModeAction);
  
  menu->addSeparator();
  menu->addMenu(m_zoomMenu);
  menu->addSeparator();
  menu->addMenu(m_layoutMenu);
  menu->addSeparator();
  menu->addMenu(m_gridMenu);
}

QMenu* WorksheetView::createContextMenu() {
	QMenu* menu = new QMenu();
	this->createMenu(menu);
	return menu;
}

void WorksheetView::fillProjectMenu(QMenu *menu, bool *rc) {
  Q_UNUSED(rc);
	this->createMenu(menu);
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
  painter->fillRect(scene_rect.intersected(rect), Qt::white);
  
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

void WorksheetView::changeLayout(QAction* action){
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
	
	
		   	  QList<QGraphicsItem *>   items=this->scene()->items();
		  qDebug()<<"number of items to layout "<< items.size();
	// 	QGraphicsLayout* layout;
	if (name == "verticalLayoutAction"){
// 	  QGraphicsLinearLayout* layout=new QGraphicsLinearLayout(Qt::Vertical);
	  

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
	qDebug()<<"selection changed";
// 	qDebug()<<"selection "<<scene()->selectionArea().boundingRect();
 QList<QGraphicsItem *> items = scene()->selectedItems();
 qDebug()<<items;
	
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

//TODO
void WorksheetView::print() const{

}
