/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2009-2013 Alexander Semke (alexander.semke*web.de)
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
#include <QGraphicsOpacityEffect>
#include <QTimeLine>

#include "commonfrontend/worksheet/WorksheetView.h"
#include "backend/worksheet/WorksheetModel.h"
#include "backend/worksheet/WorksheetElementGroup.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/WorksheetRectangleElement.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/TextLabel.h"

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
WorksheetView::WorksheetView(Worksheet *worksheet) : QGraphicsView(),
	m_worksheet(worksheet),
	m_model(new WorksheetModel(worksheet)),
	m_currentMouseMode(NavigationMode), 
	m_suppressSelectionChangedEvent(false),
	lastAddedWorksheetElement(0),
	m_fadeInTimeLine(0),
	m_fadeOutTimeLine(0),

	tbZoom(0) {
  
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

	changeZoom(zoomOriginAction);
	currentZoomAction=zoomInAction;
	

	connect(m_worksheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_worksheet, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(selectItem(QGraphicsItem*)) ); 
	connect(m_worksheet, SIGNAL(itemDeselected(QGraphicsItem*)), this, SLOT(deselectItem(QGraphicsItem*)) );
	connect(m_worksheet, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(selectItem(QGraphicsItem*)) ); 
	connect(m_worksheet, SIGNAL(itemDeselected(QGraphicsItem*)), this, SLOT(deselectItem(QGraphicsItem*)) );
	connect(m_worksheet, SIGNAL(requestUpdate()), this, SLOT(updateBackground()) );
	connect(m_worksheet, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)), this, SLOT(aspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_worksheet, SIGNAL(layoutChanged(Worksheet::Layout)), this, SLOT(layoutChanged(Worksheet::Layout)) );
	
	connect(scene(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()) );
}

WorksheetView::~WorksheetView(){
	delete m_model;
}

void WorksheetView::initActions(){
	QActionGroup* addNewActionGroup = new QActionGroup(this);
	QActionGroup* zoomActionGroup = new QActionGroup(this);
	QActionGroup* mouseModeActionGroup = new QActionGroup(this);
	QActionGroup* layoutActionGroup = new QActionGroup(this);
	QActionGroup * gridActionGroup = new QActionGroup(this);
	gridActionGroup->setExclusive(true);

	selectAllAction = new KAction(KIcon("edit-select-all"), i18n("Select all"), this);
	selectAllAction->setShortcut(Qt::CTRL+Qt::Key_A);
// 	selectAllAction->setShortcutContext(Qt::WidgetShortcut);
	this->addAction(selectAllAction);
	connect(selectAllAction, SIGNAL(triggered()), SLOT(selectAllElements()));

	deleteAction = new KAction(KIcon("edit-delete"), i18n("Delete"), this);
	deleteAction->setShortcut(Qt::Key_Delete);
// 	deleteAction->setShortcutContext(Qt::WidgetShortcut);
	this->addAction(deleteAction);
	connect(deleteAction, SIGNAL(triggered()), SLOT(deleteElement()));

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
	connect(zoomActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeZoom(QAction*)));
	connect(layoutActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeLayout(QAction*)));
	connect(gridActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeGrid(QAction*)));
	connect(snapToGridAction, SIGNAL(triggered()), this, SLOT(changeSnapToGrid()));
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
  
	//Mouse mode actions
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, navigationModeAction);
	menu->insertAction(firstAction, zoomModeAction);
	menu->insertAction(firstAction, selectionModeAction);

	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_zoomMenu);
	menu->insertMenu(firstAction, m_layoutMenu);
	menu->insertMenu(firstAction, m_gridMenu);
	menu->insertSeparator(firstAction);
}

void WorksheetView::fillToolBar(QToolBar* toolBar){
	toolBar->addSeparator();
	toolBar->addAction(navigationModeAction);
	toolBar->addAction(zoomModeAction);
	toolBar->addAction(selectionModeAction);
	
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	toolBar->addSeparator();
	toolBar->addAction(addTextLabelAction);
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
		if ( backgroundFileName!= "") {
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
	if (m_gridSettings.style == WorksheetView::NoGrid){
		painter->restore();
		return;
	}else{
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

//##############################################################################
//####################################  Events   ###############################
//##############################################################################
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

void WorksheetView::mousePressEvent(QMouseEvent* event) {
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
  if (m_currentMouseMode == ZoomMode){
	fitInView(scene()->selectionArea().boundingRect(),Qt::KeepAspectRatio);
  }
  QGraphicsView::mouseReleaseEvent(event);
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
	AbstractWorksheetElement* aspect = 0;
	if ( action == addPlotAction ){
		aspect = new CartesianPlot("xy-plot");
		dynamic_cast<CartesianPlot*>(aspect)->initDefault();
	}else if ( action == addTextLabelAction ){
		TextLabel* l = new TextLabel("text label");
		l->setText(QString("text label"));
		aspect = l;
	}

	if (!aspect)
		return;

	m_worksheet->addChild(aspect);

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

void WorksheetView::deleteElement() {
	qDebug()<<"WorksheetView::deleteElement";
}


void WorksheetView::aspectAboutToBeRemoved(const AbstractAspect* aspect){
	lastAddedWorksheetElement = dynamic_cast<AbstractWorksheetElement*>(const_cast<AbstractAspect*>(aspect));
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
	lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
}

void WorksheetView::fadeOut(qreal value) {
	QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
	effect->setOpacity(1-value);
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
	qDebug()<<"WorksheetView::selectItem";
	m_suppressSelectionChangedEvent = true;
	item->setSelected(true);
	m_selectedItems<<item;
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
	foreach ( QGraphicsItem* item , m_selectedItems ){
		if ( items.indexOf(item) == -1 ) {
			if (item->isVisible())
				m_worksheet->setItemSelectedInView( item, false );
			else
				invisibleDeselected = true;
		}
	}

	//select new items
	if (items.size() == 0 && invisibleDeselected == false){
		//no items selected -> select the worksheet again.
		m_worksheet->setSelectedInView(true);
	}else{
		foreach ( const QGraphicsItem* item , items )
			m_worksheet->setItemSelectedInView( item, true );
		
		//items selected -> deselect the worksheet in the project explorer
		//prevents unwanted multiple selection with worksheet (if it was selected before)
		m_worksheet->setSelectedInView(false);
	}
	
	m_selectedItems = items;
}

void WorksheetView::exportToFile(const QString& path, const ExportFormat format, const ExportArea area) {
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
		exportPaint(&painter, targetRect, sourceRect);
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
		exportPaint(&painter, targetRect, sourceRect);
		painter.end();
	}else{
		//PNG
		//TODO add all formats supported by Qt in QImage
		//TODO make the size of the image customizable by the user in the ExportWorksheetDialog
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Millimeter);		
		w = w*QApplication::desktop()->physicalDpiX()/25.4;
		h = h*QApplication::desktop()->physicalDpiY()/25.4;
		QImage image(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
		QRectF targetRect(0, 0, w, h);

		QPainter painter;
		painter.begin(&image);
		painter.setRenderHint(QPainter::Antialiasing);
		exportPaint(&painter, targetRect, sourceRect);
		painter.end();

		image.save(path, "png");
	}
}

void WorksheetView::exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect) {
	//draw the background
	painter->save();
	painter->scale(targetRect.width()/sourceRect.width(), targetRect.height()/sourceRect.height());
	drawBackground(painter, sourceRect);
	painter->restore();
	
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
