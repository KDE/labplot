/***************************************************************************
    File                 : DatapickerImageView.cpp
    Project              : LabPlot
    Description          : DatapickerImage view for datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015-2016 by Alexander Semke (alexander.semke@web.de)

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

#include <limits>

#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
#include <QDesktopWidget>
#include <QWheelEvent>
#include <QPrinter>
#include <QSvgGenerator>
#include <QImage>
#include <QToolButton>
#include <QTimeLine>

#include <KLocalizedString>

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
	m_transform(new Transform()) {

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
	m_image->setSegmentsHoverEvent(true);
	setInteractive(true);

	changeZoom(zoomOriginAction);
	currentZoomAction = zoomInViewAction;

	if (m_image->plotPointsType() == DatapickerImage::AxisPoints)
		setAxisPointsAction->setChecked(true);
	else if (m_image->plotPointsType() == DatapickerImage::CurvePoints)
		setCurvePointsAction->setChecked(true);
	else
		selectSegmentAction->setChecked(true);

	handleImageActions();
	changeRotationAngle();

	//signal/slot connections
	//for general actions
	connect(m_image, &DatapickerImage::requestProjectContextMenu, this, &DatapickerImageView::createContextMenu);
	connect(m_image, &DatapickerImage::requestUpdate, this, &DatapickerImageView::updateBackground);
	connect(m_image, &DatapickerImage::requestUpdateActions, this, &DatapickerImageView::handleImageActions);
	connect(m_datapicker, &Datapicker::requestUpdateActions, this, &DatapickerImageView::handleImageActions);
	connect(m_image, &DatapickerImage::rotationAngleChanged, this, &DatapickerImageView::changeRotationAngle);

	//resize the view to make the complete scene visible.
	//no need to resize the view when the project is being opened,
	//all views will be resized to the stored values at the end
	if (!m_image->isLoading()) {
		float w = Worksheet::convertFromSceneUnits(sceneRect().width(), Worksheet::Inch);
		float h = Worksheet::convertFromSceneUnits(sceneRect().height(), Worksheet::Inch);
		w *= QApplication::desktop()->physicalDpiX();
		h *= QApplication::desktop()->physicalDpiY();
		resize(w*1.1, h*1.1);
	}

	//rescale to the original size
	static const float hscale = QApplication::desktop()->physicalDpiX()/(Worksheet::convertToSceneUnits(1,Worksheet::Inch));
	static const float vscale = QApplication::desktop()->physicalDpiY()/(Worksheet::convertToSceneUnits(1,Worksheet::Inch));
	setTransform(QTransform::fromScale(hscale, vscale));
}

DatapickerImageView::~DatapickerImageView() {
	delete m_transform;
}

void DatapickerImageView::initActions() {
	auto* zoomActionGroup = new QActionGroup(this);
	auto* mouseModeActionGroup = new QActionGroup(this);
	navigationActionGroup = new QActionGroup(this);
	magnificationActionGroup = new QActionGroup(this);

	//Zoom actions
	zoomInViewAction = new QAction(QIcon::fromTheme("zoom-in"), i18n("Zoom In"), zoomActionGroup);
	zoomInViewAction->setShortcut(Qt::CTRL+Qt::Key_Plus);

	zoomOutViewAction = new QAction(QIcon::fromTheme("zoom-out"), i18n("Zoom Out"), zoomActionGroup);
	zoomOutViewAction->setShortcut(Qt::CTRL+Qt::Key_Minus);

	zoomOriginAction = new QAction(QIcon::fromTheme("zoom-original"), i18n("Original Size"), zoomActionGroup);
	zoomOriginAction->setShortcut(Qt::CTRL+Qt::Key_1);

	zoomFitPageHeightAction = new QAction(QIcon::fromTheme("zoom-fit-height"), i18n("Fit to Height"), zoomActionGroup);
	zoomFitPageWidthAction = new QAction(QIcon::fromTheme("zoom-fit-width"), i18n("Fit to Width"), zoomActionGroup);

	// Mouse mode actions
	navigationModeAction = new QAction(QIcon::fromTheme("input-mouse"), i18n("Navigate"), mouseModeActionGroup);
	navigationModeAction->setCheckable(true);
	navigationModeAction->setData(NavigationMode);

	zoomSelectionModeAction = new QAction(QIcon::fromTheme("page-zoom"), i18n("Select and Zoom"), mouseModeActionGroup);
	zoomSelectionModeAction->setCheckable(true);
	zoomSelectionModeAction->setData(ZoomSelectionMode);

	setAxisPointsAction = new QAction(QIcon::fromTheme("labplot-plot-axis-points"), i18n("Set Axis Points"), mouseModeActionGroup);
	setAxisPointsAction->setCheckable(true);
	setAxisPointsAction->setData(ReferencePointsEntryMode);

	setCurvePointsAction = new QAction(QIcon::fromTheme("labplot-xy-curve-points"), i18n("Set Curve Points"), mouseModeActionGroup);
	setCurvePointsAction->setCheckable(true);
	setCurvePointsAction->setData(CurvePointsEntryMode);

	selectSegmentAction = new QAction(QIcon::fromTheme("labplot-xy-curve-segments"), i18n("Select Curve Segments"), mouseModeActionGroup);
	selectSegmentAction->setCheckable(true);
	selectSegmentAction->setData(CurveSegmentsEntryMode);

	addCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("New Curve"), this);

	shiftLeftAction = new QAction(QIcon::fromTheme("labplot-shift-left-x"), i18n("Shift Left"), navigationActionGroup);
	shiftLeftAction->setShortcut(Qt::Key_Right);

	shiftRightAction = new QAction(QIcon::fromTheme("labplot-shift-right-x"), i18n("Shift Right"), navigationActionGroup);
	shiftRightAction->setShortcut(Qt::Key_Left);

	shiftUpAction = new QAction(QIcon::fromTheme("labplot-shift-down-y"), i18n("Shift Up"), navigationActionGroup);
	shiftUpAction->setShortcut(Qt::Key_Up);

	shiftDownAction = new QAction(QIcon::fromTheme("labplot-shift-up-y"), i18n("Shift Down"), navigationActionGroup);
	shiftDownAction->setShortcut(Qt::Key_Down);

	noMagnificationAction = new QAction(QIcon::fromTheme("labplot-1x-zoom"), i18n("No Magnification"), magnificationActionGroup);
	noMagnificationAction->setCheckable(true);
	noMagnificationAction->setChecked(true);

	twoTimesMagnificationAction = new QAction(QIcon::fromTheme("labplot-2x-zoom"), i18n("2x Magnification"), magnificationActionGroup);
	twoTimesMagnificationAction->setCheckable(true);

	threeTimesMagnificationAction = new QAction(QIcon::fromTheme("labplot-3x-zoom"), i18n("3x Magnification"), magnificationActionGroup);
	threeTimesMagnificationAction->setCheckable(true);

	fourTimesMagnificationAction = new QAction(QIcon::fromTheme("labplot-4x-zoom"), i18n("4x Magnification"), magnificationActionGroup);
	fourTimesMagnificationAction->setCheckable(true);

	fiveTimesMagnificationAction = new QAction(QIcon::fromTheme("labplot-5x-zoom"), i18n("5x Magnification"), magnificationActionGroup);
	fiveTimesMagnificationAction->setCheckable(true);

	//set some default values
	currentZoomAction = zoomInViewAction;
	currentMagnificationAction = noMagnificationAction;

	switch(m_image->plotPointsType()) {
	case DatapickerImage::AxisPoints:
		currentPlotPointsTypeAction = setAxisPointsAction;
		setAxisPointsAction->setChecked(true);
		mouseModeChanged(setAxisPointsAction);
		break;
	case DatapickerImage::CurvePoints:
		currentPlotPointsTypeAction = setCurvePointsAction;
		setCurvePointsAction->setChecked(true);
		mouseModeChanged(setCurvePointsAction);
		break;
	case DatapickerImage::SegmentPoints:
		currentPlotPointsTypeAction = selectSegmentAction;
		selectSegmentAction->setChecked(true);
		mouseModeChanged(selectSegmentAction);
	}

	//signal-slot connections
	connect(mouseModeActionGroup, &QActionGroup::triggered, this, &DatapickerImageView::mouseModeChanged);
	connect(zoomActionGroup, &QActionGroup::triggered, this, &DatapickerImageView::changeZoom);
	connect(addCurveAction, &QAction::triggered, this, &DatapickerImageView::addCurve);
	connect(navigationActionGroup, &QActionGroup::triggered, this, &DatapickerImageView::changeSelectedItemsPosition);
	connect(magnificationActionGroup, &QActionGroup::triggered, this, &DatapickerImageView::magnificationChanged);
}

void DatapickerImageView::initMenus() {
	m_viewMouseModeMenu = new QMenu(i18n("Mouse Mode"), this);
	m_viewMouseModeMenu->setIcon(QIcon::fromTheme("input-mouse"));
	m_viewMouseModeMenu->addAction(setAxisPointsAction);
	m_viewMouseModeMenu->addAction(setCurvePointsAction);
	m_viewMouseModeMenu->addAction(selectSegmentAction);
	m_viewMouseModeMenu->addSeparator();
	m_viewMouseModeMenu->addAction(navigationModeAction);
	m_viewMouseModeMenu->addAction(zoomSelectionModeAction);

	m_zoomMenu = new QMenu(i18n("Zoom View"), this);
	m_zoomMenu->setIcon(QIcon::fromTheme("zoom-draw"));
	m_zoomMenu->addAction(zoomInViewAction);
	m_zoomMenu->addAction(zoomOutViewAction);
	m_zoomMenu->addAction(zoomOriginAction);
	m_zoomMenu->addAction(zoomFitPageHeightAction);
	m_zoomMenu->addAction(zoomFitPageWidthAction);

	m_navigationMenu = new QMenu(i18n("Move Last Point"), this);
	m_navigationMenu->addAction(shiftLeftAction);
	m_navigationMenu->addAction(shiftRightAction);
	m_navigationMenu->addAction(shiftUpAction);
	m_navigationMenu->addAction(shiftDownAction);

	m_magnificationMenu = new QMenu(i18n("Magnification"), this);
	m_magnificationMenu->setIcon(QIcon::fromTheme("zoom-in"));
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

	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, addCurveAction);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_navigationMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_viewMouseModeMenu);
	menu->insertMenu(firstAction, m_zoomMenu);
	menu->insertMenu(firstAction, m_magnificationMenu);
	menu->insertSeparator(firstAction);
}

void DatapickerImageView::fillToolBar(QToolBar* toolBar) {
	toolBar->addSeparator();
	toolBar->addAction(setAxisPointsAction);
	toolBar->addAction(setCurvePointsAction);
	toolBar->addAction(selectSegmentAction);
	toolBar->addAction(navigationModeAction);
	toolBar->addAction(zoomSelectionModeAction);

	toolBar->addSeparator();
	toolBar->addAction(addCurveAction);

	toolBar->addSeparator();
	toolBar->addAction(shiftRightAction);
	toolBar->addAction(shiftLeftAction);
	toolBar->addAction(shiftUpAction);
	toolBar->addAction(shiftDownAction);

	tbZoom = new QToolButton(toolBar);
	tbZoom->setPopupMode(QToolButton::MenuButtonPopup);
	tbZoom->setMenu(m_zoomMenu);
	tbZoom->setDefaultAction(currentZoomAction);
	toolBar->addSeparator();
	toolBar->addWidget(tbZoom);

	tbMagnification = new QToolButton(toolBar);
	tbMagnification->setPopupMode(QToolButton::MenuButtonPopup);
	tbMagnification->setMenu(m_magnificationMenu);
	tbMagnification->setDefaultAction(currentMagnificationAction);
	toolBar->addWidget(tbMagnification);
}

void DatapickerImageView::setScene(QGraphicsScene* scene) {
	QGraphicsView::setScene(scene);
	setTransform(QTransform());
}

void DatapickerImageView::drawForeground(QPainter* painter, const QRectF& rect) {
	if (m_mouseMode == ZoomSelectionMode && m_selectionBandIsShown) {
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

	// canvas
	if (m_image->isLoaded) {
		if (m_image->plotImageType() == DatapickerImage::OriginalImage) {
			QImage todraw = m_image->originalPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			painter->drawImage(scene_rect.topLeft(), todraw);
		} else if (m_image->plotImageType() == DatapickerImage::ProcessedImage) {
			QImage todraw = m_image->processedPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			painter->drawImage(scene_rect.topLeft(), todraw);
		} else {
			painter->fillRect(scene_rect, Qt::white);
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
void DatapickerImageView::wheelEvent(QWheelEvent* event) {
	//https://wiki.qt.io/Smooth_Zoom_In_QGraphicsView
	if (m_mouseMode == ZoomSelectionMode || (QApplication::keyboardModifiers() & Qt::ControlModifier)) {
		int numDegrees = event->delta() / 8;
		int numSteps = numDegrees / 15; // see QWheelEvent documentation
		zoom(numSteps);
	} else
		QGraphicsView::wheelEvent(event);
}

void DatapickerImageView::zoom(int numSteps) {
	m_numScheduledScalings += numSteps;
	if (m_numScheduledScalings * numSteps < 0) // if user moved the wheel in another direction, we reset previously scheduled scalings
		m_numScheduledScalings = numSteps;

	auto* anim = new QTimeLine(350, this);
	anim->setUpdateInterval(20);

	connect(anim, &QTimeLine::valueChanged, this, &DatapickerImageView::scalingTime);
	connect(anim, &QTimeLine::finished, this, &DatapickerImageView::animFinished);
	anim->start();
}

void DatapickerImageView::scalingTime() {
	qreal factor = 1.0 + qreal(m_numScheduledScalings) / 300.0;
	scale(factor, factor);
}

void DatapickerImageView::animFinished() {
	if (m_numScheduledScalings > 0)
		m_numScheduledScalings--;
	else
		m_numScheduledScalings++;
	sender()->~QObject();
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
	bool entryMode = (m_mouseMode == ReferencePointsEntryMode
					  || m_mouseMode == CurvePointsEntryMode || m_mouseMode == CurveSegmentsEntryMode);

	//check whether there is a point item under the cursor
	bool pointsUnderCursor = false;
	auto items = this->items(event->pos());
	for (auto* item : items) {
		if (item != m_image->m_magnificationWindow) {
			pointsUnderCursor = true;
			break;
		}
	}

	if (entryMode && !pointsUnderCursor && m_image->isLoaded && sceneRect().contains(eventPos)) {
		if ( m_image->plotPointsType() == DatapickerImage::AxisPoints ) {
			int childCount = m_image->childCount<DatapickerPoint>(AbstractAspect::IncludeHidden);
			if (childCount < 3)
				m_datapicker->addNewPoint(eventPos, m_image);
		} else if ( m_image->plotPointsType() == DatapickerImage::CurvePoints && m_datapicker->activeCurve() ) {
			m_datapicker->addNewPoint(eventPos, m_datapicker->activeCurve());
		}

		if (m_image->m_magnificationWindow && m_image->m_magnificationWindow->isVisible())
			updateMagnificationWindow();
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
			QString xLabel('x');
			QString yLabel('y');
			if (m_image->axisPoints().type == DatapickerImage::PolarInDegree) {
				xLabel = 'r';
				yLabel = "y(deg)";
			} else if (m_image->axisPoints().type == DatapickerImage::PolarInRadians) {
				xLabel = 'r';
				yLabel = "y(rad)";
			}

			if (m_datapicker->activeCurve()) {
				QString statusText = i18n("%1, active curve \"%2\": %3=%4, %5=%6", m_datapicker->name(), m_datapicker->activeCurve()->name(), xLabel, QString::number(logicalPos.x()), yLabel, QString::number(logicalPos.y()));
				emit statusInfo(statusText);
			}
		}
	}

	//show the magnification window
	if ( magnificationFactor && m_image->isLoaded && sceneRect().contains(pos)
		&& m_image->plotPointsType() != DatapickerImage::SegmentPoints ) {

		if (!m_image->m_magnificationWindow) {
//            m_image->m_magnificationWindow = new QGraphicsPixmapItem(0, scene());
			m_image->m_magnificationWindow = new QGraphicsPixmapItem;
			scene()->addItem(m_image->m_magnificationWindow);
			m_image->m_magnificationWindow->setZValue(std::numeric_limits<int>::max());
		}

		updateMagnificationWindow();
	} else if (m_image->m_magnificationWindow) {
		m_image->m_magnificationWindow->setVisible(false);
	}

	QGraphicsView::mouseMoveEvent(event);
}

void DatapickerImageView::updateMagnificationWindow() {
	m_image->m_magnificationWindow->setVisible(false);
	QPointF pos = mapToScene(mapFromGlobal(QCursor::pos()));

	//copy the part of the view to be shown magnified
	const int size = Worksheet::convertToSceneUnits(2.0, Worksheet::Centimeter)/transform().m11();
	const QRectF copyRect(pos.x() - size/(2*magnificationFactor), pos.y() - size/(2*magnificationFactor), size/magnificationFactor, size/magnificationFactor);
	QPixmap px = grab(mapFromScene(copyRect).boundingRect());
	px = px.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	//draw the bounding rect
	QPainter painter(&px);
	const QPen pen = QPen(Qt::lightGray, 2/transform().m11());
	painter.setPen(pen);
	QRect rect = px.rect();
	rect.setWidth(rect.width()-pen.widthF()/2);
	rect.setHeight(rect.height()-pen.widthF()/2);
	painter.drawRect(rect);

	//set the pixmap
	m_image->m_magnificationWindow->setPixmap(px);
	m_image->m_magnificationWindow->setPos(pos.x()- px.width()/2, pos.y()- px.height()/2);

	m_image->m_magnificationWindow->setVisible(true);
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
void DatapickerImageView::mouseModeChanged(QAction* action) {
	m_mouseMode = (DatapickerImageView::MouseMode)action->data().toInt();

	if (action == navigationModeAction) {
		setInteractive(false);
		setDragMode(QGraphicsView::ScrollHandDrag);
		m_image->setSegmentsHoverEvent(false);
	} else if (action == zoomSelectionModeAction){
		setInteractive(false);
		setDragMode(QGraphicsView::NoDrag);
		m_image->setSegmentsHoverEvent(false);
		setCursor(Qt::ArrowCursor);
	} else {
		setInteractive(true);
		setDragMode(QGraphicsView::NoDrag);
		m_image->setSegmentsHoverEvent(true);
		setCursor(Qt::CrossCursor);

		if (currentPlotPointsTypeAction != action) {
			if (action == setAxisPointsAction) {
				int count = m_image->childCount<DatapickerPoint>(AbstractAspect::IncludeHidden);
				if (count) {
					auto button = QMessageBox::question(this, i18n("Remove existing reference points?"),
														i18n("All available reference points will be removed. Do you want to continue?"));
					if (button != QMessageBox::Yes) {
						currentPlotPointsTypeAction->setChecked(true);
						return;
					}
				}

				m_image->setPlotPointsType(DatapickerImage::AxisPoints);
			} else if (action == setCurvePointsAction)
				m_image->setPlotPointsType(DatapickerImage::CurvePoints);
			else if (action == selectSegmentAction)
				m_image->setPlotPointsType(DatapickerImage::SegmentPoints);

			currentPlotPointsTypeAction = action;
		}
	}
}

void DatapickerImageView::changeZoom(QAction* action) {
	if (action == zoomInViewAction)
		zoom(1);
	else if (action == zoomOutViewAction)
		zoom(-1);
	else if (action == zoomOriginAction) {
		static const float hscale = QApplication::desktop()->physicalDpiX()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		static const float vscale = QApplication::desktop()->physicalDpiY()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		setTransform(QTransform::fromScale(hscale, vscale));
		m_rotationAngle = 0;
	} else if (action == zoomFitPageWidthAction) {
		float scaleFactor = viewport()->width()/scene()->sceneRect().width();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
		m_rotationAngle = 0;
	} else if (action == zoomFitPageHeightAction) {
		float scaleFactor = viewport()->height()/scene()->sceneRect().height();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
		m_rotationAngle = 0;
	}

	currentZoomAction = action;
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
	const QVector<DatapickerPoint*> axisPoints = m_image->children<DatapickerPoint>(AbstractAspect::IncludeHidden);
	for (auto* point : axisPoints) {
		if (!point->graphicsItem()->isSelected())
			continue;

		QPointF newPos = point->position();
		newPos = newPos + shift;
		point->setPosition(newPos);

		int pointIndex = m_image->indexOfChild<DatapickerPoint>(point, AbstractAspect::IncludeHidden);
		if (pointIndex == -1) continue;
		DatapickerImage::ReferencePoints points = m_image->axisPoints();
		points.scenePos[pointIndex].setX(point->position().x());
		points.scenePos[pointIndex].setY(point->position().y());
		m_image->setUndoAware(false);
		m_image->setAxisPoints(points);
		m_image->setUndoAware(true);
	}

	for (auto* curve : m_image->parentAspect()->children<DatapickerCurve>()) {
		for (auto* point : curve->children<DatapickerPoint>(AbstractAspect::IncludeHidden)) {
			if (!point->graphicsItem()->isSelected())
				continue;

			QPointF newPos = point->position();
			newPos = newPos + shift;
			point->setPosition(newPos);
		}
	}

	m_image->endMacro();
	if (m_image->m_magnificationWindow && m_image->m_magnificationWindow->isVisible())
		updateMagnificationWindow();
}

void DatapickerImageView::magnificationChanged(QAction* action) {
	if (action == noMagnificationAction)
		magnificationFactor = 0;
	else if (action == twoTimesMagnificationAction)
		magnificationFactor = 2;
	else if (action == threeTimesMagnificationAction)
		magnificationFactor = 3;
	else if (action == fourTimesMagnificationAction)
		magnificationFactor = 4;
	else if (action == fiveTimesMagnificationAction)
		magnificationFactor = 5;
}

void DatapickerImageView::addCurve() {
	m_datapicker->beginMacro(i18n("%1: add new curve.", m_datapicker->name()));
	DatapickerCurve* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(m_image->axisPoints().type);
	m_datapicker->addChild(curve);
	m_datapicker->endMacro();

	setCurvePointsAction->setChecked(true);
	mouseModeChanged(setCurvePointsAction);
}

void DatapickerImageView::changeRotationAngle() {
	this->rotate(m_rotationAngle);
	this->rotate(-m_image->rotationAngle());
	m_rotationAngle = m_image->rotationAngle();
	updateBackground();
}

void DatapickerImageView::handleImageActions() {
	if (m_image->isLoaded) {
		magnificationActionGroup->setEnabled(true);
		setAxisPointsAction->setEnabled(true);

		int pointsCount = m_image->childCount<DatapickerPoint>(AbstractAspect::IncludeHidden);
		if (pointsCount > 0)
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
		}
	} else {
		navigationActionGroup->setEnabled(false);
		magnificationActionGroup->setEnabled(false);
		setAxisPointsAction->setEnabled(false);
		addCurveAction->setEnabled(false);
		setCurvePointsAction->setEnabled(false);
		selectSegmentAction->setEnabled(false);
	}
}

void DatapickerImageView::exportToFile(const QString& path, const WorksheetView::ExportFormat format, const int resolution) {
	QRectF sourceRect;
	sourceRect = scene()->sceneRect();

	//print
	if (format == WorksheetView::Pdf) {
		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(path);
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Millimeter);
		printer.setPaperSize( QSizeF(w, h), QPrinter::Millimeter);
		printer.setPageMargins(0,0,0,0, QPrinter::Millimeter);
		printer.setPrintRange(QPrinter::PageRange);
		printer.setCreator( QLatin1String("LabPlot ") + LVERSION );

		QPainter painter(&printer);
		painter.setRenderHint(QPainter::Antialiasing);
		QRectF targetRect(0, 0, painter.device()->width(),painter.device()->height());
		painter.begin(&printer);
		exportPaint(&painter, targetRect, sourceRect);
		painter.end();
	} else if (format == WorksheetView::Svg) {
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

void DatapickerImageView::print(QPrinter* printer) {
	const QRectF scene_rect = sceneRect();
	int w = Worksheet::convertFromSceneUnits(scene_rect.width(), Worksheet::Millimeter);
	int h = Worksheet::convertFromSceneUnits(scene_rect.height(), Worksheet::Millimeter);
	printer->setPaperSize( QSizeF(w, h), QPrinter::Millimeter);
	printer->setPageMargins(0,0,0,0, QPrinter::Millimeter);
	printer->setPrintRange(QPrinter::PageRange);
	printer->setCreator( QString("LabPlot ") + LVERSION );

	QPainter painter(printer);
	QRectF targetRect(0, 0, painter.device()->width(),painter.device()->height());
	painter.setRenderHint(QPainter::Antialiasing);
	painter.begin(printer);
	painter.save();
	painter.scale(targetRect.width()/scene_rect.width(), targetRect.height()/scene_rect.height());

	// canvas
	if (m_image->isLoaded) {
		if (m_image->plotImageType() == DatapickerImage::OriginalImage) {
			QImage todraw = m_image->originalPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			painter.drawImage(scene_rect.topLeft(), todraw);
		} else if (m_image->plotImageType() == DatapickerImage::ProcessedImage) {
			QImage todraw = m_image->processedPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			painter.drawImage(scene_rect.topLeft(), todraw);
		} else {
			painter.fillRect(scene_rect, Qt::white);
		}
	} else {
		painter.setBrush(QBrush(Qt::gray));
		painter.drawRect(scene_rect);
	}

	painter.restore();
	m_image->setPrinting(true);
	scene()->render(&painter, QRectF(), scene_rect);
	m_image->setPrinting(false);
	painter.end();
}

void DatapickerImageView::updateBackground() {
	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}
