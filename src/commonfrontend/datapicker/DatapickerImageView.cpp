/***************************************************************************
    File                 : DatapickerImageView.cpp
    Project              : LabPlot
    Description          : DatapickerImage view for datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
	Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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

#include "commonfrontend/datapicker/DatapickerImageView.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/Transform.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/datapicker/DatapickerImage.h"

#include <QMenu>
#include <QToolBar>
#include <QDesktopWidget>
#include <QWheelEvent>
#include <QPrinter>
#include <QSvgGenerator>
#include <QImage>
#include <QToolButton>
#include <QApplication>

#include <KAction>
#include <KLocale>

/**
 * \class DatapickerImageView
 * \brief Datapicker/DatapickerImage view
 */

/*!
  Constructur of the class.
  Creates a view for the DatapickerImage \c image and initializes the internal model.
*/
DatapickerImageView::DatapickerImageView(DatapickerImage* image) : QGraphicsView(),
	m_image(image),
	m_datapicker(dynamic_cast<Datapicker*>(m_image->parentAspect())),
	m_transform(new Transform()),
	m_mouseMode(SelectAndEditMode),
	m_selectionBandIsShown(false),
	magnificationFactor(0),
	m_rotationAngle(0),
	tbZoom(0) {

	setScene(m_image->scene());

	setRenderHint(QPainter::Antialiasing);
	setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setMinimumSize(16, 16);
	setFocusPolicy(Qt::StrongFocus);

	viewport()->setAttribute( Qt::WA_OpaquePaintEvent );
	viewport()->setAttribute( Qt::WA_NoSystemBackground );
	setCacheMode(QGraphicsView::CacheBackground);

	initActions();
	initMenus();
	selectAndEditModeAction->setChecked(true);
	setInteractive(true);

	changeZoom(zoomOriginAction);
	currentZoomAction=zoomInViewAction;
	handleImageActions();
	changeRotationAngle();

	//signal/slot connections
	//for general actions
	connect( m_image, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)) );
	connect( m_image, SIGNAL(requestUpdate()), this, SLOT(updateBackground()) );
	connect( m_datapicker, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(handleImageActions()));
	connect( m_datapicker, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
	         this, SLOT(handleImageActions()) );
	connect( m_image, SIGNAL(rotationAngleChanged(float)), this, SLOT(changeRotationAngle()) );
}

void DatapickerImageView::initActions() {
	QActionGroup* zoomActionGroup = new QActionGroup(this);
	QActionGroup* mouseModeActionGroup = new QActionGroup(this);
	QActionGroup* plotPointsTypeActionGroup = new QActionGroup(this);
	navigationActionGroup = new QActionGroup(this);
	magnificationActionGroup = new QActionGroup(this);

	//Zoom actions
	zoomInViewAction = new KAction(KIcon("zoom-in"), i18n("Zoom in"), zoomActionGroup);
	zoomInViewAction->setShortcut(Qt::CTRL+Qt::Key_Plus);

	zoomOutViewAction = new KAction(KIcon("zoom-out"), i18n("Zoom out"), zoomActionGroup);
	zoomOutViewAction->setShortcut(Qt::CTRL+Qt::Key_Minus);

	zoomOriginAction = new KAction(KIcon("zoom-original"), i18n("Original size"), zoomActionGroup);
	zoomOriginAction->setShortcut(Qt::CTRL+Qt::Key_1);

	zoomFitPageHeightAction = new KAction(KIcon("zoom-fit-height"), i18n("Fit to height"), zoomActionGroup);
	zoomFitPageWidthAction = new KAction(KIcon("zoom-fit-width"), i18n("Fit to width"), zoomActionGroup);

	// Mouse mode actions
	selectAndEditModeAction = new KAction(KIcon("labplot-cursor-arrow"), i18n("Select and Edit"), mouseModeActionGroup);
	selectAndEditModeAction->setCheckable(true);

	navigationModeAction = new KAction(KIcon("input-mouse"), i18n("Navigate"), mouseModeActionGroup);
	navigationModeAction->setCheckable(true);

	zoomSelectionModeAction = new KAction(KIcon("page-zoom"), i18n("Select and Zoom"), mouseModeActionGroup);
	zoomSelectionModeAction->setCheckable(true);

	selectAndMoveModeAction = new KAction(KIcon("labplot-cursor-arrow"), i18n("Select and Move"), mouseModeActionGroup);
	selectAndMoveModeAction->setCheckable(true);

	setAxisPointsAction = new KAction(KIcon("labplot-plot-axis-points"), i18n("Set Axis Points"), plotPointsTypeActionGroup);
	setAxisPointsAction->setCheckable(true);

	setCurvePointsAction = new KAction(KIcon("labplot-xy-curve-points"), i18n("Set Curve Points"), plotPointsTypeActionGroup);
	setCurvePointsAction->setCheckable(true);

	selectSegmentAction = new KAction(KIcon("labplot-xy-curve-segments"), i18n("Select Curve Segments"), plotPointsTypeActionGroup);
	selectSegmentAction->setCheckable(true);

	addCurveAction = new KAction(KIcon("labplot-xy-curve"), i18n("New Curve"), this);

	shiftLeftAction = new KAction(KIcon("labplot-shift-left-x"), i18n("Shift Left"), navigationActionGroup);
	shiftLeftAction->setShortcut(Qt::Key_Right);

	shiftRightAction = new KAction(KIcon("labplot-shift-right-x"), i18n("Shift Right"), navigationActionGroup);
	shiftRightAction->setShortcut(Qt::Key_Left);

	shiftUpAction = new KAction(KIcon("labplot-shift-down-y"), i18n("Shift Up"), navigationActionGroup);
	shiftUpAction->setShortcut(Qt::Key_Up);

	shiftDownAction = new KAction(KIcon("labplot-shift-up-y"), i18n("Shift Down"), navigationActionGroup);
	shiftDownAction->setShortcut(Qt::Key_Down);

	noMagnificationAction = new KAction(KIcon("labplot-1-to-1-zoom"), i18n("No Magnification"), magnificationActionGroup);
	noMagnificationAction->setCheckable(true);
	noMagnificationAction->setChecked(true);

	twoTimesMagnificationAction = new KAction(KIcon("labplot-1-to-2-zoom"), i18n("2x Magnification"), magnificationActionGroup);
	twoTimesMagnificationAction->setCheckable(true);

	threeTimesMagnificationAction = new KAction(KIcon("labplot-1-to-3-zoom"), i18n("3x Magnification"), magnificationActionGroup);
	threeTimesMagnificationAction->setCheckable(true);

	fourTimesMagnificationAction = new KAction(KIcon("labplot-1-to-4-zoom"), i18n("4x Magnification"), magnificationActionGroup);
	fourTimesMagnificationAction->setCheckable(true);

	fiveTimesMagnificationAction = new KAction(KIcon("labplot-1-to-5-zoom"), i18n("5x Magnification"), magnificationActionGroup);
	fiveTimesMagnificationAction->setCheckable(true);

	connect( mouseModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(mouseModeChanged(QAction*)) );
	connect( zoomActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeZoom(QAction*)) );
	connect( plotPointsTypeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changePointsType(QAction*)) );
	connect( addCurveAction, SIGNAL(triggered()), this, SLOT(addCurve()) );
	connect( navigationActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeSelectedItemsPosition(QAction*)) );
	connect( magnificationActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(magnificationChanged(QAction*)) );
}

void DatapickerImageView::initMenus() {
	m_viewMouseModeMenu = new QMenu(i18n("Mouse Mode"));
	m_viewMouseModeMenu->setIcon(KIcon("input-mouse"));
	m_viewMouseModeMenu->addAction(selectAndEditModeAction);
	m_viewMouseModeMenu->addAction(navigationModeAction);
	m_viewMouseModeMenu->addAction(zoomSelectionModeAction);
	m_viewMouseModeMenu->addAction(selectAndMoveModeAction);

	m_viewImageMenu = new QMenu(i18n("Image Menu"));
	//m_viewImageMenu->setIcon();
	m_viewImageMenu->addAction(setAxisPointsAction);
	m_viewImageMenu->addAction(setCurvePointsAction);
	m_viewImageMenu->addAction(selectSegmentAction);

	m_zoomMenu = new QMenu(i18n("Zoom"));
	m_zoomMenu->setIcon(KIcon("zoom-draw"));
	m_zoomMenu->addAction(zoomInViewAction);
	m_zoomMenu->addAction(zoomOutViewAction);
	m_zoomMenu->addAction(zoomOriginAction);
	m_zoomMenu->addAction(zoomFitPageHeightAction);
	m_zoomMenu->addAction(zoomFitPageWidthAction);

	m_navigationMenu = new QMenu(i18n("Navigate Selected Items"));
	//m_navigationMenu->setIcon();
	m_navigationMenu->addAction(shiftLeftAction);
	m_navigationMenu->addAction(shiftRightAction);
	m_navigationMenu->addAction(shiftUpAction);
	m_navigationMenu->addAction(shiftDownAction);

	m_magnificationMenu = new QMenu(i18n("Magnification"));
	//m_viewImageMenu->setIcon();
	m_magnificationMenu->addAction(noMagnificationAction);
	m_magnificationMenu->addAction(twoTimesMagnificationAction);
	m_magnificationMenu->addAction(threeTimesMagnificationAction);
	m_magnificationMenu->addAction(fourTimesMagnificationAction);
	m_magnificationMenu->addAction(fiveTimesMagnificationAction);
}

/*!
 * Populates the menu \c menu with the image and image-view relevant actions.
 * The menu is used
 *   - as the context menu in DatapickerImageView
 *   - as the "datapicker menu" in the main menu-bar (called form MainWin)
 *   - as a part of the image context menu in project explorer
 */
void DatapickerImageView::createContextMenu(QMenu* menu) const {
	Q_ASSERT(menu);

	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertMenu(firstAction, m_viewMouseModeMenu);
	menu->insertMenu(firstAction, m_zoomMenu);
	menu->insertMenu(firstAction, m_magnificationMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_viewImageMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, addCurveAction);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_navigationMenu);
	menu->insertSeparator(firstAction);
}

void DatapickerImageView::fillToolBar(QToolBar* toolBar) {
	toolBar->addSeparator();
	toolBar->addAction(setAxisPointsAction);
	toolBar->addAction(setCurvePointsAction);
	toolBar->addAction(selectSegmentAction);

	toolBar->addSeparator();
	toolBar->addAction(addCurveAction);

	toolBar->addSeparator();
	toolBar->addAction(noMagnificationAction);
	toolBar->addAction(twoTimesMagnificationAction);
	toolBar->addAction(threeTimesMagnificationAction);
	toolBar->addAction(fourTimesMagnificationAction);
	toolBar->addAction(fiveTimesMagnificationAction);

	toolBar->addSeparator();
	toolBar->addAction(shiftRightAction);
	toolBar->addAction(shiftLeftAction);
	toolBar->addAction(shiftUpAction);
	toolBar->addAction(shiftDownAction);

	toolBar->addSeparator();
	toolBar->addAction(selectAndEditModeAction);
	toolBar->addAction(navigationModeAction);
	toolBar->addAction(zoomSelectionModeAction);
	toolBar->addAction(selectAndMoveModeAction);

	tbZoom = new QToolButton(toolBar);
	tbZoom->setPopupMode(QToolButton::MenuButtonPopup);
	tbZoom->setMenu(m_zoomMenu);
	tbZoom->setDefaultAction(currentZoomAction);
	toolBar->addWidget(tbZoom);
}

void DatapickerImageView::setScene(QGraphicsScene* scene) {
	QGraphicsView::setScene(scene);
	setTransform(QTransform());
}

void DatapickerImageView::drawForeground(QPainter* painter, const QRectF& rect) {
	if (m_mouseMode==ZoomSelectionMode && m_selectionBandIsShown) {
		painter->save();
		const QRectF& selRect = mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect();
		painter->setPen(QPen(Qt::black, 5/transform().m11()));
		painter->drawRect(selRect);
		painter->setBrush(Qt::blue);
		painter->setOpacity(0.2);
		painter->drawRect(selRect);
		painter->restore();
	}
	QGraphicsView::drawForeground(painter, rect);
}

void DatapickerImageView::drawBackground(QPainter* painter, const QRectF& rect) {
	painter->save();

	QRectF scene_rect = sceneRect();
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
	if (m_image->isLoaded) {
		if (m_image->plotImageType == DatapickerImage::OriginalImage) {
			QImage todraw = m_image->originalPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			painter->drawImage(scene_rect.topLeft(), todraw);
		} else {
			QImage todraw = m_image->processedPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			painter->drawImage(scene_rect.topLeft(), todraw);
		}
	} else {
		painter->setBrush(QBrush(Qt::gray));
		painter->drawRect(scene_rect);
	}

	invalidateScene(rect, QGraphicsScene::BackgroundLayer);
	painter->restore();
}

//##############################################################################
//####################################  Events   ###############################
//##############################################################################
void DatapickerImageView::wheelEvent(QWheelEvent *event) {
	if (m_mouseMode == ZoomSelectionMode) {
		if (event->delta() > 0)
			scale(1.2, 1.2);
		else if (event->delta() < 0)
			scale(1.0/1.2, 1.0/1.2);
	} else {
		QGraphicsView::wheelEvent(event);
	}
}

void DatapickerImageView::mousePressEvent(QMouseEvent* event) {
	//prevent the deselection of items when context menu event
	//was triggered (right button click)
	if (event->button() == Qt::RightButton) {
		event->accept();
		return;
	}

	if (event->button() == Qt::LeftButton && m_mouseMode == ZoomSelectionMode) {
		m_selectionStart = event->pos();
		m_selectionBandIsShown = true;
		return;
	}

	QPointF eventPos = mapToScene(event->pos());
	if ( m_mouseMode == SelectAndEditMode && m_image->isLoaded && sceneRect().contains(eventPos) ) {
		if ( m_image->plotPointsType() == DatapickerImage::AxisPoints ) {
			int childCount = m_image->childCount<DatapickerPoint>(AbstractAspect::IncludeHidden);
			if (childCount < 3)
				m_datapicker->addNewPoint(eventPos, m_image);
		} else if ( m_image->plotPointsType() == DatapickerImage::CurvePoints && m_datapicker->activeCurve() ) {
			m_datapicker->addNewPoint(eventPos, m_datapicker->activeCurve());
		}
	}

	// make sure the datapicker (or its currently active curve) is selected in the project explorer if the view was clicked.
	// We need this for the case when we change from the project-node in the project explorer to the datapicker node by clicking the view.
    if (m_datapicker->activeCurve() && m_image->plotPointsType() != DatapickerImage::AxisPoints) {
        m_datapicker->setSelectedInView(false);
        m_datapicker->activeCurve()->setSelectedInView(true);
    } else {
        if (m_datapicker->activeCurve())
            m_datapicker->activeCurve()->setSelectedInView(false);
        m_datapicker->setSelectedInView(true);
    }

	QGraphicsView::mousePressEvent(event);
}

void DatapickerImageView::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton && m_mouseMode == ZoomSelectionMode) {
		m_selectionBandIsShown = false;
		viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());

		//don't zoom if very small region was selected, avoid occasional/unwanted zooming
		m_selectionEnd = event->pos();
		if ( abs(m_selectionEnd.x()-m_selectionStart.x())>20 && abs(m_selectionEnd.y()-m_selectionStart.y())>20 )
			fitInView(mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect(), Qt::KeepAspectRatio);
	}

	QGraphicsView::mouseReleaseEvent(event);
}

void DatapickerImageView::mouseMoveEvent(QMouseEvent* event) {
	if ( m_mouseMode == SelectAndEditMode || m_mouseMode == ZoomSelectionMode ) {
		if (m_image->isLoaded)
			setCursor(Qt::CrossCursor);
		else
			setCursor(Qt::ArrowCursor);
	} else {
		setCursor(Qt::ArrowCursor);
	}

	//show the selection band
	if (m_selectionBandIsShown) {
		QRect rect = QRect(m_selectionStart, m_selectionEnd).normalized();
		m_selectionEnd = event->pos();
		rect = rect.united(QRect(m_selectionStart, m_selectionEnd).normalized());
		int penWidth = 5/transform().m11();
		rect.setX(rect.x()-penWidth);
		rect.setY(rect.y()-penWidth);
		rect.setHeight(rect.height()+2*penWidth);
		rect.setWidth(rect.width()+2*penWidth);
		viewport()->repaint(rect);
		return;
	}

	QPointF pos = mapToScene(event->pos());

	//show the current coordinates under the mouse cursor in the status bar
	if (m_image->plotPointsType() == DatapickerImage::CurvePoints) {
		QVector3D logicalPos = m_transform->mapSceneToLogical(pos, m_image->axisPoints());
		if (m_image->axisPoints().type == DatapickerImage::Ternary) {
			emit statusInfo( "a =" + QString::number(logicalPos.x()) + ", b =" + QString::number(logicalPos.y()) + ", c =" + QString::number(logicalPos.z()));
		} else {
			QString xLabel = "x";
			QString yLabel = "y";
			if (m_image->axisPoints().type == DatapickerImage::PolarInDegree) {
				xLabel = "r";
				yLabel = "y(deg)";
			} else if (m_image->axisPoints().type == DatapickerImage::PolarInRadians) {
				xLabel = "r";
				yLabel = "y(rad)";
			}

			if (m_datapicker->activeCurve()) {
				QString statusText = m_datapicker->name() + ", " + i18n("active curve") + " \"" + m_datapicker->activeCurve()->name() + "\"";
				statusText += ": " +  xLabel + "=" + QString::number(logicalPos.x()) + ", " + yLabel + "=" + QString::number(logicalPos.y());
				emit statusInfo(statusText);
			}
		}
	}

	//show the magnification window
	if ( magnificationFactor && m_mouseMode == SelectAndEditMode && m_image->isLoaded && sceneRect().contains(pos)
	        && m_image->plotPointsType() != DatapickerImage::SegmentPoints ) {

		if (!m_image->m_magnificationWindow) {
			m_image->m_magnificationWindow = new QGraphicsPixmapItem(0, scene());
			m_image->m_magnificationWindow->setZValue(-1);
		}

		int size = 200/transform().m11();
		QImage imageSection = m_image->originalPlotImage.scaled(scene()->width(), scene()->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		imageSection = imageSection.copy(pos.x() - size/2, pos.y() - size/2, size, size);
		imageSection = imageSection.scaled(size*magnificationFactor, size*magnificationFactor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		imageSection = imageSection.copy(imageSection.width()/2 - size/2, imageSection.height()/2 - size/2, size, size);
		QPainter painter(&imageSection);
		painter.setPen(QPen(Qt::lightGray, 2/transform().m11()));
		painter.drawRect(imageSection.rect());

		m_image->m_magnificationWindow->setVisible(true);
		m_image->m_magnificationWindow->setPixmap(QPixmap::fromImage(imageSection));
		m_image->m_magnificationWindow->setPos(pos.x()- imageSection.width()/2, pos.y()- imageSection.height()/2);
	} else if (m_image->m_magnificationWindow) {
		m_image->m_magnificationWindow->setVisible(false);
	}

	QGraphicsView::mouseMoveEvent(event);
}

void DatapickerImageView::contextMenuEvent(QContextMenuEvent* e) {
	Q_UNUSED(e);
	//no need to propagate the event to the scene and graphics items
	QMenu *menu = new QMenu(this);
	this->createContextMenu(menu);
	menu->exec(QCursor::pos());
}

//##############################################################################
//####################################  SLOTs   ###############################
//##############################################################################
void DatapickerImageView::changePointsType(QAction* action) {
	if (action==setAxisPointsAction) {
		//clear image
		int childCount = m_image->childCount<DatapickerPoint>(AbstractAspect::IncludeHidden);
		if (childCount)
			m_image->removeAllChildren();
		m_image->setPlotPointsType(DatapickerImage::AxisPoints);
		m_image->setSegmentVisible(false);
	} else if (action==setCurvePointsAction) {
		m_image->setPlotPointsType(DatapickerImage::CurvePoints);
		m_image->setSegmentVisible(false);
	} else if (action==selectSegmentAction) {
		m_image->setPlotPointsType(DatapickerImage::SegmentPoints);
		m_image->setSegmentVisible(true);
	}
}

void DatapickerImageView::changeZoom(QAction* action) {
	if (action==zoomInViewAction) {
		scale(1.2, 1.2);
	} else if (action==zoomOutViewAction) {
		scale(1.0/1.2, 1.0/1.2);
	} else if (action==zoomOriginAction) {
		static const float hscale = QApplication::desktop()->physicalDpiX()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		static const float vscale = QApplication::desktop()->physicalDpiY()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		setTransform(QTransform::fromScale(hscale, vscale));
		m_rotationAngle = 0;
	} else if (action==zoomFitPageWidthAction) {
		float scaleFactor = viewport()->width()/scene()->sceneRect().width();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
		m_rotationAngle = 0;
	} else if (action==zoomFitPageHeightAction) {
		float scaleFactor = viewport()->height()/scene()->sceneRect().height();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
		m_rotationAngle = 0;
	}

	currentZoomAction=action;
	if (tbZoom)
		tbZoom->setDefaultAction(action);

	//change and set angle if tranform reset
	changeRotationAngle();
}

void DatapickerImageView::changeSelectedItemsPosition(QAction* action) {
	if (scene()->selectedItems().isEmpty())
		return;

	QPointF shift(0, 0);
	if (action == shiftLeftAction)
		shift.setX(1);
	else if (action == shiftRightAction)
		shift.setX(-1);
	else if (action == shiftUpAction)
		shift.setY(-1);
	else if (action == shiftDownAction)
		shift.setY(1);

	m_image->beginMacro(i18n("%1: change position of selected DatapickerPoints.", m_image->name()));
	QList<DatapickerPoint*> axisPoints = m_image->children<DatapickerPoint>(AbstractAspect::IncludeHidden);
	foreach (DatapickerPoint* point, axisPoints) {
		if (!point->graphicsItem()->isSelected())
			continue;

        QPointF newPos = point->position();
        newPos = newPos + shift;
		point->setPosition(newPos);

		int pointIndex = m_image->indexOfChild<DatapickerPoint>(point , AbstractAspect::IncludeHidden);
		DatapickerImage::ReferencePoints points = m_image->axisPoints();
        points.scenePos[pointIndex].setX(point->position().x());
        points.scenePos[pointIndex].setY(point->position().y());
		m_image->setUndoAware(false);
		m_image->setAxisPoints(points);
		m_image->setUndoAware(true);
	}

	foreach (DatapickerCurve* curve, m_image->parentAspect()->children<DatapickerCurve>()) {
		foreach (DatapickerPoint* point, curve->children<DatapickerPoint>(AbstractAspect::IncludeHidden)) {
			if (!point->graphicsItem()->isSelected())
				continue;

            QPointF newPos = point->position();
            newPos = newPos + shift;
			point->setPosition(newPos);
		}
	}

	m_image->endMacro();
}

void DatapickerImageView::mouseModeChanged(QAction* action) {
	if (action==selectAndEditModeAction) {
		m_mouseMode = SelectAndEditMode;
		setInteractive(true);
		setDragMode(QGraphicsView::NoDrag);
	} else if (action==navigationModeAction) {
		m_mouseMode = NavigationMode;
		setInteractive(false);
		setDragMode(QGraphicsView::ScrollHandDrag);
	} else if (action==zoomSelectionModeAction) {
		m_mouseMode = ZoomSelectionMode;
		setInteractive(false);
		setDragMode(QGraphicsView::NoDrag);
	} else {
		m_mouseMode = SelectAndMoveMode;
		setInteractive(true);
		setDragMode(QGraphicsView::NoDrag);
	}
}

void DatapickerImageView::magnificationChanged(QAction* action) {
	if (action==noMagnificationAction)
		magnificationFactor = 0;
	else if (action==twoTimesMagnificationAction)
		magnificationFactor = 2;
	else if (action==threeTimesMagnificationAction)
		magnificationFactor = 3;
	else if (action==fourTimesMagnificationAction)
		magnificationFactor = 4;
	else if (action==fiveTimesMagnificationAction)
		magnificationFactor = 5;
}

void DatapickerImageView::addCurve() {
	m_datapicker->beginMacro(i18n("%1: add new curve.", m_datapicker->name()));
	DatapickerCurve* curve = new DatapickerCurve(i18n("Curve"));
	m_datapicker->addChild(curve);
	curve->addDatasheet(m_image->axisPoints().type);
	m_datapicker->endMacro();
}

void DatapickerImageView::changeRotationAngle() {
	this->rotate(m_rotationAngle);
	this->rotate(-m_image->rotationAngle());
	m_rotationAngle = m_image->rotationAngle();
	updateBackground();
}

void DatapickerImageView::handleImageActions() {
	if (m_image->isLoaded) {
		navigationActionGroup->setEnabled(true);
		magnificationActionGroup->setEnabled(true);
		setAxisPointsAction->setEnabled(true);

		int pointsCount = m_image->childCount<DatapickerPoint>(AbstractAspect::IncludeHidden);
		if (pointsCount>0)
			navigationActionGroup->setEnabled(true);
		else
			navigationActionGroup->setEnabled(false);

		if (pointsCount > 2) {
			addCurveAction->setEnabled(true);
			if (m_datapicker->activeCurve()) {
				setCurvePointsAction->setEnabled(true);
				selectSegmentAction->setEnabled(true);
			} else {
				setCurvePointsAction->setEnabled(false);
				selectSegmentAction->setEnabled(false);
			}
		} else {
			addCurveAction->setEnabled(false);
			setCurvePointsAction->setEnabled(false);
			selectSegmentAction->setEnabled(false);
			if (m_image->plotPointsType() != DatapickerImage::AxisPoints) {
				m_image->setUndoAware(false);
				m_image->setPlotPointsType(DatapickerImage::AxisPoints);
				m_image->setUndoAware(true);
			}
		}

		setAxisPointsAction->setChecked(m_image->plotPointsType() == DatapickerImage::AxisPoints);
		setCurvePointsAction->setChecked(m_image->plotPointsType() == DatapickerImage::CurvePoints);
		selectSegmentAction->setChecked(m_image->plotPointsType() == DatapickerImage::SegmentPoints);
	} else {
		navigationActionGroup->setEnabled(false);
		magnificationActionGroup->setEnabled(false);
		setAxisPointsAction->setEnabled(false);
		addCurveAction->setEnabled(false);
		setCurvePointsAction->setEnabled(false);
		selectSegmentAction->setEnabled(false);
	}
}

void DatapickerImageView::exportToFile(const QString& path, const ExportFormat format, const int resolution) {
	QRectF sourceRect;
	sourceRect = scene()->sceneRect();

	//print
	if (format==DatapickerImageView::Pdf || format==DatapickerImageView::Eps) {
		QPrinter printer(QPrinter::HighResolution);
		if (format==DatapickerImageView::Pdf)
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
	} else if (format==DatapickerImageView::Svg) {
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
	} else {
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
		exportPaint(&painter, targetRect, sourceRect);
		painter.end();

		image.save(path, "png");
	}
}

void DatapickerImageView::exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect) {
	painter->save();
	painter->scale(targetRect.width()/sourceRect.width(), targetRect.height()/sourceRect.height());
	drawBackground(painter, sourceRect);
	painter->restore();
	m_image->setPrinting(true);
	scene()->render(painter, QRectF(), sourceRect);
	m_image->setPrinting(false);
}

void DatapickerImageView::print(QPrinter* printer) const {
	m_image->setPrinting(true);
	QPainter painter(printer);
	painter.setRenderHint(QPainter::Antialiasing);
	scene()->render(&painter);
	m_image->setPrinting(false);
}

void DatapickerImageView::updateBackground() {
	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}
