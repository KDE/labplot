/*
	File                 : DatapickerImageView.cpp
	Project              : LabPlot
	Description          : DatapickerImage view for datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/datapicker/Transform.h"
#include <frontend/GuiTools.h>
#include "frontend/datapicker/DatapickerImageView.h"
#include "frontend/widgets/toggleactionmenu.h"

#include <KLocalizedString>

#include <QActionGroup>
#include <QClipboard>
#include <QFileInfo>
#include <QImage>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPrinter>
#include <QScreen>
#include <QTimeLine>
#include <QWheelEvent>
#ifdef HAVE_QTSVG
#include <QSvgGenerator>
#endif

#include <gsl/gsl_const_cgs.h>

/**
 * \class DatapickerImageView
 * \brief Datapicker/DatapickerImage view
 */

/*!
  Constructor of the class.
  Creates a view for the DatapickerImage \c image and initializes the internal model.
*/
DatapickerImageView::DatapickerImageView(DatapickerImage* image)
	: QGraphicsView()
	, m_image(image)
	, m_datapicker(dynamic_cast<Datapicker*>(m_image->parentAspect()))
	, m_transform(new Transform()) {
	setScene(m_image->scene());

	setRenderHint(QPainter::Antialiasing);
	setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setMinimumSize(16, 16);
	setFocusPolicy(Qt::StrongFocus);

	viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
	viewport()->setAttribute(Qt::WA_NoSystemBackground);
	setCacheMode(QGraphicsView::CacheBackground);

	initActions();
	initMenus();
	m_image->setSegmentsHoverEvent(true);
	setInteractive(true);

	changeZoom(zoomOriginAction);
	// currentZoomAction = zoomInViewAction;

	if (m_image->plotPointsType() == DatapickerImage::PointsType::AxisPoints)
		setAxisPointsAction->setChecked(true);
	else if (m_image->plotPointsType() == DatapickerImage::PointsType::CurvePoints)
		setCurvePointsAction->setChecked(true);
	else
		selectSegmentAction->setChecked(true);

	handleImageActions();
	changeRotationAngle();

	// signal/slot connections
	// for general actions
	connect(m_image, &DatapickerImage::requestProjectContextMenu, this, &DatapickerImageView::createContextMenu);
	connect(m_image, &DatapickerImage::requestUpdate, this, &DatapickerImageView::updateBackground);
	connect(m_image, &DatapickerImage::requestUpdateActions, this, &DatapickerImageView::handleImageActions);
	connect(m_datapicker, &Datapicker::requestUpdateActions, this, &DatapickerImageView::handleImageActions);
	connect(m_image, &DatapickerImage::rotationAngleChanged, this, &DatapickerImageView::changeRotationAngle);

	// resize the view to make the complete scene visible.
	// no need to resize the view when the project is being opened,
	// all views will be resized to the stored values at the end
	if (!m_image->isLoading()) {
		float w = Worksheet::convertFromSceneUnits(sceneRect().width(), Worksheet::Unit::Inch);
		float h = Worksheet::convertFromSceneUnits(sceneRect().height(), Worksheet::Unit::Inch);
		w *= GuiTools::dpi(this).first;
		h *= GuiTools::dpi(this).second;
		resize(w * 1.1, h * 1.1);
	}

	// rescale to the original size
	static const float hscale = GuiTools::dpi(this).first / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
	static const float vscale = GuiTools::dpi(this).second / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
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

	// Zoom actions
	zoomInViewAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("Zoom In"), zoomActionGroup);
	zoomInViewAction->setShortcut(Qt::CTRL | Qt::Key_Plus);

	zoomOutViewAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Zoom Out"), zoomActionGroup);
	zoomOutViewAction->setShortcut(Qt::CTRL | Qt::Key_Minus);

	zoomOriginAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-original")), i18n("Original Size"), zoomActionGroup);
	zoomOriginAction->setShortcut(Qt::CTRL | Qt::Key_1);

	zoomFitPageHeightAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-fit-height")), i18nc("Zoom", "Fit to Height"), zoomActionGroup);
	zoomFitPageWidthAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-fit-width")), i18nc("Zoom", "Fit to Width"), zoomActionGroup);

	// Mouse mode actions
	navigationModeAction = new QAction(QIcon::fromTheme(QStringLiteral("input-mouse")), i18n("Navigate"), mouseModeActionGroup);
	navigationModeAction->setCheckable(true);
	navigationModeAction->setData(static_cast<int>(MouseMode::Navigation));

	zoomSelectionModeAction = new QAction(QIcon::fromTheme(QStringLiteral("page-zoom")), i18n("Select and Zoom"), mouseModeActionGroup);
	zoomSelectionModeAction->setCheckable(true);
	zoomSelectionModeAction->setData(static_cast<int>(MouseMode::ZoomSelection));

	setAxisPointsAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-plot-axis-points")), i18n("Set Axis Points"), mouseModeActionGroup);
	setAxisPointsAction->setCheckable(true);
	setAxisPointsAction->setData(static_cast<int>(MouseMode::ReferencePointsEntry));

	setCurvePointsAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve-points")), i18n("Set Curve Points"), mouseModeActionGroup);
	setCurvePointsAction->setCheckable(true);
	setCurvePointsAction->setData(static_cast<int>(MouseMode::CurvePointsEntry));

	selectSegmentAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve-segments")), i18n("Select Curve Segments"), mouseModeActionGroup);
	selectSegmentAction->setCheckable(true);
	selectSegmentAction->setData(static_cast<int>(MouseMode::CurveSegmentsEntry));

	// add curve action
	addCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("New Curve"), this);

	// shift actions
	shiftLeftAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-left-x")), i18n("Shift Left"), navigationActionGroup);
	shiftLeftAction->setData(static_cast<int>(ShiftOperation::ShiftLeft));
	shiftLeftAction->setShortcut(Qt::Key_Right);

	shiftRightAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-right-x")), i18n("Shift Right"), navigationActionGroup);
	shiftRightAction->setData(static_cast<int>(ShiftOperation::ShiftRight));
	shiftRightAction->setShortcut(Qt::Key_Left);

	shiftUpAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-down-y")), i18n("Shift Up"), navigationActionGroup);
	shiftUpAction->setData(static_cast<int>(ShiftOperation::ShiftUp));
	shiftUpAction->setShortcut(Qt::Key_Up);

	shiftDownAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-up-y")), i18n("Shift Down"), navigationActionGroup);
	shiftDownAction->setData(static_cast<int>(ShiftOperation::ShiftDown));
	shiftDownAction->setShortcut(Qt::Key_Down);

	// magnification actions
	auto* action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-1x-zoom")), i18n("No Magnification"), magnificationActionGroup);
	action->setData(0);
	action->setCheckable(true);
	action->setChecked(true);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-2x-zoom")), i18n("2x Magnification"), magnificationActionGroup);
	action->setData(2);
	action->setCheckable(true);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-3x-zoom")), i18n("3x Magnification"), magnificationActionGroup);
	action->setData(3);
	action->setCheckable(true);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-4x-zoom")), i18n("4x Magnification"), magnificationActionGroup);
	action->setData(4);
	action->setCheckable(true);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-5x-zoom")), i18n("5x Magnification"), magnificationActionGroup);
	action->setData(5);
	action->setCheckable(true);

	// // set some default values
	switch (m_image->plotPointsType()) {
	case DatapickerImage::PointsType::AxisPoints:
		currentPlotPointsTypeAction = setAxisPointsAction;
		setAxisPointsAction->setChecked(true);
		changeMouseMode(setAxisPointsAction);
		break;
	case DatapickerImage::PointsType::CurvePoints:
		currentPlotPointsTypeAction = setCurvePointsAction;
		setCurvePointsAction->setChecked(true);
		changeMouseMode(setCurvePointsAction);
		break;
	case DatapickerImage::PointsType::SegmentPoints:
		currentPlotPointsTypeAction = selectSegmentAction;
		selectSegmentAction->setChecked(true);
		changeMouseMode(selectSegmentAction);
	}

	// signal-slot connections
	connect(mouseModeActionGroup, &QActionGroup::triggered, this, &DatapickerImageView::changeMouseMode);
	connect(zoomActionGroup, &QActionGroup::triggered, this, &DatapickerImageView::changeZoom);
	connect(addCurveAction, &QAction::triggered, this, &DatapickerImageView::addCurve);
	connect(navigationActionGroup, &QActionGroup::triggered, this, &DatapickerImageView::changeSelectedItemsPosition);
	connect(magnificationActionGroup, &QActionGroup::triggered, this, &DatapickerImageView::changeMagnification);
}

void DatapickerImageView::initMenus() {
	m_viewMouseModeMenu = new QMenu(i18n("Mouse Mode"), this);
	m_viewMouseModeMenu->setIcon(QIcon::fromTheme(QStringLiteral("input-mouse")));
	m_viewMouseModeMenu->addAction(setAxisPointsAction);
	m_viewMouseModeMenu->addAction(setCurvePointsAction);
	m_viewMouseModeMenu->addAction(selectSegmentAction);
	m_viewMouseModeMenu->addSeparator();
	m_viewMouseModeMenu->addAction(navigationModeAction);
	m_viewMouseModeMenu->addAction(zoomSelectionModeAction);

	m_zoomMenu = new QMenu(i18n("Zoom View"), this);
	m_zoomMenu->setIcon(QIcon::fromTheme(QStringLiteral("zoom-draw")));
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
	m_magnificationMenu->setIcon(QIcon::fromTheme(QStringLiteral("zoom-in")));
	for (auto* action : magnificationActionGroup->actions())
		m_magnificationMenu->addAction(action);
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
	// there're already actions available there. Skip the first title-action
	// and insert the action at the beginning of the menu.
	if (menu->actions().size() > 1)
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

void DatapickerImageView::fillZoomMenu(ToggleActionMenu* menu) const {
	menu->addAction(zoomInViewAction);
	menu->addAction(zoomOutViewAction);
	menu->addAction(zoomOriginAction);
}

void DatapickerImageView::fillMagnificationMenu(ToggleActionMenu* menu) const {
	for (auto* action : magnificationActionGroup->actions())
		menu->addAction(action);
}

void DatapickerImageView::setScene(QGraphicsScene* scene) {
	QGraphicsView::setScene(scene);
	setTransform(QTransform());
}

void DatapickerImageView::drawForeground(QPainter* painter, const QRectF& rect) {
	if (m_mouseMode == MouseMode::ZoomSelection && m_selectionBandIsShown) {
		painter->save();
		const QRectF& selRect = mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect();
		painter->setPen(QPen(Qt::black, 5 / transform().m11()));
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
		if (m_image->plotImageType() == DatapickerImage::PlotImageType::OriginalImage) {
			const QImage& todraw = m_image->originalPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			painter->drawImage(scene_rect.topLeft(), todraw);
		} else if (m_image->plotImageType() == DatapickerImage::PlotImageType::ProcessedImage) {
			const QImage& todraw = m_image->processedPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
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

// ##############################################################################
// ####################################  Events   ###############################
// ##############################################################################
void DatapickerImageView::keyPressEvent(QKeyEvent* event) {
	if (event->matches(QKeySequence::Paste)) {
		const QClipboard* clipboard = QApplication::clipboard();
		const QMimeData* mimeData = clipboard->mimeData();
		if (mimeData->hasImage()) {
			m_image->setImage(qvariant_cast<QImage>(mimeData->imageData()), QStringLiteral(""), true);
			event->accept();
		} else if (mimeData->hasText()) {
			// Check if it is a filepath
			QString text = mimeData->text();
			if (text.startsWith(QStringLiteral("file://")))
				text.replace(QStringLiteral("file://"), QStringLiteral(""));
			QFileInfo fi(text);
			if (fi.exists()) {
				m_image->setImage(fi.absoluteFilePath(), true);
				event->accept();
			}
		}
	}
	QGraphicsView::keyPressEvent(event);
}

void DatapickerImageView::wheelEvent(QWheelEvent* event) {
	// https://wiki.qt.io/Smooth_Zoom_In_QGraphicsView
	if (m_mouseMode == MouseMode::ZoomSelection || (QApplication::keyboardModifiers() & Qt::ControlModifier)) {
		QPoint numDegrees = event->angleDelta() / 8;
		int numSteps = numDegrees.y() / 15; // see QWheelEvent documentation
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
	// prevent the deselection of items when context menu event
	// was triggered (right button click)
	if (event->button() == Qt::RightButton) {
		event->accept();
		return;
	}

	if (event->button() == Qt::LeftButton && m_mouseMode == MouseMode::ZoomSelection) {
		m_selectionStart = event->pos();
		m_selectionBandIsShown = true;
		return;
	}

	const auto eventPos = mapToScene(event->pos());
	const auto type = m_image->plotPointsType();
	bool entryMode =
		(m_mouseMode == MouseMode::ReferencePointsEntry || m_mouseMode == MouseMode::CurvePointsEntry || m_mouseMode == MouseMode::CurveSegmentsEntry);

	// check whether there is a point item under the cursor
	bool pointsUnderCursor = false;
	const auto& items = this->items(event->pos());
	const auto& referencePoints = m_image->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	for (auto* item : items) {
		if (item == m_image->m_magnificationWindow)
			continue;

		// when entering curve points, ignore the reference points under the cursor,
		// it should be possible to place curve points close to or over the reference points.
		if (type == DatapickerImage::PointsType::CurvePoints) {
			bool referenceItem = false;
			for (auto* point : referencePoints) {
				if (point->graphicsItem() == item) {
					referenceItem = true;
					break;
				}
			}

			if (referenceItem)
				continue;
		}

		pointsUnderCursor = true;
		break;
	}

	if (entryMode && !pointsUnderCursor && m_image->isLoaded && sceneRect().contains(eventPos)) {
		if (type == DatapickerImage::PointsType::AxisPoints) {
			int childCount = m_image->childCount<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
			if (childCount < 3)
				m_datapicker->addNewPoint(eventPos, m_image);
		} else if (type == DatapickerImage::PointsType::CurvePoints && m_datapicker->activeCurve())
			m_datapicker->addNewPoint(eventPos, m_datapicker->activeCurve());

		if (m_image->m_magnificationWindow && m_image->m_magnificationWindow->isVisible())
			updateMagnificationWindow();
	}

	// make sure the datapicker (or its currently active curve) is selected in the project explorer if the view was clicked.
	// We need this for the case when we change from the project-node in the project explorer to the datapicker node by clicking the view.
	if (m_datapicker->activeCurve() && type != DatapickerImage::PointsType::AxisPoints) {
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
	if (event->button() == Qt::LeftButton && m_mouseMode == MouseMode::ZoomSelection) {
		m_selectionBandIsShown = false;
		viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());

		// don't zoom if very small region was selected, avoid occasional/unwanted zooming
		m_selectionEnd = event->pos();
		if (abs(m_selectionEnd.x() - m_selectionStart.x()) > 20 && abs(m_selectionEnd.y() - m_selectionStart.y()) > 20)
			fitInView(mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect(), Qt::KeepAspectRatio);
	}

	QGraphicsView::mouseReleaseEvent(event);
}

void DatapickerImageView::mouseMoveEvent(QMouseEvent* event) {
	// show the selection band
	if (m_selectionBandIsShown) {
		QRect rect = QRect(m_selectionStart, m_selectionEnd).normalized();
		m_selectionEnd = event->pos();
		rect = rect.united(QRect(m_selectionStart, m_selectionEnd).normalized());
		int penWidth = 5 / transform().m11();
		rect.setX(rect.x() - penWidth);
		rect.setY(rect.y() - penWidth);
		rect.setHeight(rect.height() + 2 * penWidth);
		rect.setWidth(rect.width() + 2 * penWidth);
		viewport()->repaint(rect);
		return;
	}

	QPointF pos = mapToScene(event->pos());

	// show the current coordinates under the mouse cursor in the status bar
	if (m_image->plotPointsType() == DatapickerImage::PointsType::CurvePoints) {
		Vector3D logicalPos = m_transform->mapSceneToLogical(pos, m_image->axisPoints());
		if (m_image->axisPoints().type == DatapickerImage::GraphType::Ternary) {
			Q_EMIT statusInfo(QStringLiteral("a =") + QString::number(logicalPos.x()) + QStringLiteral(", b =") + QString::number(logicalPos.y())
							  + QStringLiteral(", c =") + QString::number(logicalPos.z()));
		} else {
			QString xLabel(QStringLiteral("x"));
			QString yLabel(QStringLiteral("y"));
			if (m_image->axisPoints().type == DatapickerImage::GraphType::PolarInDegree) {
				xLabel = QStringLiteral("r");
				yLabel = QStringLiteral("y(deg)");
			} else if (m_image->axisPoints().type == DatapickerImage::GraphType::PolarInRadians) {
				xLabel = QStringLiteral("r");
				yLabel = QStringLiteral("y(rad)");
			}

			if (m_datapicker->activeCurve()) {
				QString statusText = i18n("%1, active curve \"%2\": %3=%4, %5=%6",
										  m_datapicker->name(),
										  m_datapicker->activeCurve()->name(),
										  xLabel,
										  QString::number(logicalPos.x()),
										  yLabel,
										  QString::number(logicalPos.y()));
				Q_EMIT statusInfo(statusText);
			}
		}
	}

	// show the magnification window
	if (m_magnificationFactor && m_image->isLoaded && sceneRect().contains(pos)) {
		if (!m_image->m_magnificationWindow) {
			m_image->m_magnificationWindow = new QGraphicsPixmapItem;
			scene()->addItem(m_image->m_magnificationWindow);
			m_image->m_magnificationWindow->setZValue(std::numeric_limits<int>::max());
		}

		updateMagnificationWindow();
	} else if (m_image->m_magnificationWindow)
		m_image->m_magnificationWindow->setVisible(false);

	QGraphicsView::mouseMoveEvent(event);
}

void DatapickerImageView::updateMagnificationWindow() {
	m_image->m_magnificationWindow->setVisible(false);
	QPointF pos = mapToScene(mapFromGlobal(QCursor::pos()));

	// copy the part of the view to be shown magnified
	const int size = Worksheet::convertToSceneUnits(2.0, Worksheet::Unit::Centimeter) / transform().m11();
	const QRectF copyRect(pos.x() - size / (2 * m_magnificationFactor),
						  pos.y() - size / (2 * m_magnificationFactor),
						  size / m_magnificationFactor,
						  size / m_magnificationFactor);
	QPixmap px = grab(mapFromScene(copyRect).boundingRect());
	px = px.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	// draw the bounding rect
	QPainter painter(&px);
	const QPen pen = QPen(Qt::lightGray, 2 / transform().m11());
	painter.setPen(pen);
	QRect rect = px.rect();
	rect.setWidth(rect.width() - pen.widthF() / 2);
	rect.setHeight(rect.height() - pen.widthF() / 2);
	painter.drawRect(rect);

	// set the pixmap
	m_image->m_magnificationWindow->setPixmap(px);
	m_image->m_magnificationWindow->setPos(pos.x() - px.width() / 2, pos.y() - px.height() / 2);

	m_image->m_magnificationWindow->setVisible(true);
}

void DatapickerImageView::contextMenuEvent(QContextMenuEvent*) {
	// no need to propagate the event to the scene and graphics items
	QMenu* menu = new QMenu(this);
	this->createContextMenu(menu);
	menu->exec(QCursor::pos());
}

// ##############################################################################
// ####################################  SLOTs   ###############################
// ##############################################################################
void DatapickerImageView::changeMouseMode(QAction* action) {
	m_mouseMode = (DatapickerImageView::MouseMode)action->data().toInt();

	if (action == navigationModeAction) {
		setInteractive(false);
		setDragMode(QGraphicsView::ScrollHandDrag);
		m_image->setSegmentsHoverEvent(false);
	} else if (action == zoomSelectionModeAction) {
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
				int count = m_image->childCount<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
				if (count) {
					auto button = QMessageBox::question(this,
														i18n("Remove existing reference points?"),
														i18n("All available reference points will be removed. Do you want to continue?"));
					if (button != QMessageBox::Yes) {
						currentPlotPointsTypeAction->setChecked(true);
						return;
					}
				}

				m_image->setPlotPointsType(DatapickerImage::PointsType::AxisPoints);
			} else if (action == setCurvePointsAction)
				m_image->setPlotPointsType(DatapickerImage::PointsType::CurvePoints);
			else if (action == selectSegmentAction)
				m_image->setPlotPointsType(DatapickerImage::PointsType::SegmentPoints);

			currentPlotPointsTypeAction = action;
		}
	}
}

DatapickerImageView::MouseMode DatapickerImageView::mouseMode() const {
	return m_mouseMode;
}

void DatapickerImageView::changeZoom(QAction* action) {
	m_zoomMode = static_cast<ZoomMode>(action->data().toInt());

	switch (m_zoomMode) {
	case ZoomMode::ZoomIn:
		zoom(1);
		break;
	case ZoomMode::ZoomOut:
		zoom(-1);
		break;
	case ZoomMode::ZoomOrigin:
		static const float hscale = GuiTools::dpi(this).first / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));
		static const float vscale = GuiTools::dpi(this).second / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));
		setTransform(QTransform::fromScale(hscale, vscale));
		m_rotationAngle = 0;
		break;
	case ZoomMode::ZoomFitPageWidth: {
		float scaleFactor = viewport()->width() / scene()->sceneRect().width();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
		m_rotationAngle = 0;
		break;
	}
	case ZoomMode::ZoomFitPageHeight: {
		float scaleFactor = viewport()->height() / scene()->sceneRect().height();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
		m_rotationAngle = 0;
	}
	}

	// change and set angle if tranform reset
	changeRotationAngle();
}

DatapickerImageView::ZoomMode DatapickerImageView::zoomMode() const {
	return m_zoomMode;
}

void DatapickerImageView::changeSelectedItemsPosition(QAction* action) {
	if (scene()->selectedItems().isEmpty())
		return;

	const auto shiftOp = static_cast<ShiftOperation>(action->data().toInt());
	QPointF shift(0, 0);
	switch (shiftOp) {
	case ShiftOperation::ShiftLeft:
		shift.setX(1);
		break;
	case ShiftOperation::ShiftRight:
		shift.setX(-1);
		break;
	case ShiftOperation::ShiftUp:
		shift.setY(-1);
		break;
	case ShiftOperation::ShiftDown:
		shift.setY(1);
		break;
	}

	m_image->beginMacro(i18n("%1: change position of selected points", m_image->name()));
	const QVector<DatapickerPoint*> axisPoints = m_image->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	for (auto* point : axisPoints) {
		if (!point->graphicsItem()->isSelected())
			continue;

		QPointF newPos = point->position();
		newPos = newPos + shift;
		point->setPosition(newPos);

		int pointIndex = m_image->indexOfChild<DatapickerPoint>(point, AbstractAspect::ChildIndexFlag::IncludeHidden);
		if (pointIndex == -1)
			continue;
		DatapickerImage::ReferencePoints points = m_image->axisPoints();
		points.scenePos[pointIndex].setX(point->position().x());
		points.scenePos[pointIndex].setY(point->position().y());
		m_image->setUndoAware(false);
		m_image->setAxisPoints(points);
		m_image->setUndoAware(true);
	}

	for (auto* curve : m_image->parentAspect()->children<DatapickerCurve>()) {
		for (auto* point : curve->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden)) {
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

void DatapickerImageView::changeMagnification(QAction* action) {
	m_magnificationFactor = action->data().toInt();

	if (m_magnificationFactor == 0 && m_image->m_magnificationWindow)
		m_image->m_magnificationWindow->setVisible(false);
}

int DatapickerImageView::magnification() const {
	return m_magnificationFactor;
}

void DatapickerImageView::addCurve() {
	m_datapicker->beginMacro(i18n("%1: add new curve.", m_datapicker->name()));
	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(m_image->axisPoints().type);
	m_datapicker->addChild(curve);
	m_datapicker->endMacro();

	setCurvePointsAction->setChecked(true);
	changeMouseMode(setCurvePointsAction);
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

		int pointsCount = m_image->childCount<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
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
	QRectF sourceRect = scene()->sceneRect();

	// print
	if (format == WorksheetView::ExportFormat::PDF) {
		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(path);
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
		printer.setPageSize(QPageSize(QSizeF(w, h), QPageSize::Millimeter));
		printer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);
		printer.setPrintRange(QPrinter::PageRange);
		printer.setCreator(QLatin1String("LabPlot ") + QLatin1String(LVERSION));

		QPainter painter(&printer);
		painter.setRenderHint(QPainter::Antialiasing);
		QRectF targetRect(0, 0, painter.device()->width(), painter.device()->height());
		painter.begin(&printer);
		exportPaint(&painter, targetRect, sourceRect);
		painter.end();
	} else if (format == WorksheetView::ExportFormat::SVG) {
#ifdef HAVE_QTSVG
		QSvgGenerator generator;
		generator.setFileName(path);
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
		w = w * GuiTools::dpi(this).first / GSL_CONST_CGS_INCH;
		h = h * GuiTools::dpi(this).second / GSL_CONST_CGS_INCH;

		generator.setSize(QSize(w, h));
		QRectF targetRect(0, 0, w, h);
		generator.setViewBox(targetRect);

		QPainter painter;
		painter.begin(&generator);
		exportPaint(&painter, targetRect, sourceRect);
		painter.end();
#endif
	} else {
		// PNG
		// TODO add all formats supported by Qt in QImage
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
		w = w * resolution / GSL_CONST_CGS_INCH;
		h = h * resolution / GSL_CONST_CGS_INCH;
		QImage image(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
		image.fill(Qt::transparent);
		QRectF targetRect(0, 0, w, h);

		QPainter painter;
		painter.begin(&image);
		painter.setRenderHint(QPainter::Antialiasing);
		exportPaint(&painter, targetRect, sourceRect);
		painter.end();

		if (!path.isEmpty()) {
			bool rc{false};
			switch (format) {
			case WorksheetView::ExportFormat::PNG:
				rc = image.save(path, "PNG");
				break;
			case WorksheetView::ExportFormat::JPG:
				rc = image.save(path, "JPG");
				break;
			case WorksheetView::ExportFormat::BMP:
				rc = image.save(path, "BMP");
				break;
			case WorksheetView::ExportFormat::PPM:
				rc = image.save(path, "PPM");
				break;
			case WorksheetView::ExportFormat::XBM:
				rc = image.save(path, "XBM");
				break;
			case WorksheetView::ExportFormat::XPM:
				rc = image.save(path, "XPM");
				break;
			case WorksheetView::ExportFormat::PDF:
			case WorksheetView::ExportFormat::SVG:
				break;
			}

			if (!rc) {
				RESET_CURSOR;
				QMessageBox::critical(nullptr, i18n("Failed to export"), i18n("Failed to write to '%1'. Please check the path.", path));
			}
		}
	}
}

void DatapickerImageView::exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect) {
	bool magnificationActive = false;
	if (m_image->m_magnificationWindow && m_image->m_magnificationWindow->isVisible()) {
		magnificationActive = true;
		m_image->m_magnificationWindow->setVisible(false);
	}

	painter->save();
	painter->scale(targetRect.width() / sourceRect.width(), targetRect.height() / sourceRect.height());
	drawBackground(painter, sourceRect);
	painter->restore();
	m_image->setPrinting(true);
	scene()->render(painter, QRectF(), sourceRect);
	m_image->setPrinting(false);

	if (magnificationActive)
		m_image->m_magnificationWindow->setVisible(true);
}

void DatapickerImageView::print(QPrinter* printer) {
	const QRectF& scene_rect = sceneRect();
	const int w = Worksheet::convertFromSceneUnits(scene_rect.width(), Worksheet::Unit::Millimeter);
	const int h = Worksheet::convertFromSceneUnits(scene_rect.height(), Worksheet::Unit::Millimeter);
	printer->setPageSize(QPageSize(QSizeF(w, h), QPageSize::Millimeter));
	printer->setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);
	printer->setPrintRange(QPrinter::PageRange);
	printer->setCreator(QStringLiteral("LabPlot ") + QLatin1String(LVERSION));

	QPainter painter(printer);
	QRectF targetRect(0, 0, painter.device()->width(), painter.device()->height());
	painter.setRenderHint(QPainter::Antialiasing);
	painter.begin(printer);
	painter.save();
	painter.scale(targetRect.width() / scene_rect.width(), targetRect.height() / scene_rect.height());

	// canvas
	if (m_image->isLoaded) {
		if (m_image->plotImageType() == DatapickerImage::PlotImageType::OriginalImage) {
			QImage todraw = m_image->originalPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			painter.drawImage(scene_rect.topLeft(), todraw);
		} else if (m_image->plotImageType() == DatapickerImage::PlotImageType::ProcessedImage) {
			QImage todraw = m_image->processedPlotImage.scaled(scene_rect.width(), scene_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			painter.drawImage(scene_rect.topLeft(), todraw);
		} else
			painter.fillRect(scene_rect, Qt::white);
	} else {
		painter.setBrush(QBrush(Qt::gray));
		painter.drawRect(scene_rect);
	}

	painter.restore();
	m_image->setPrinting(true);
	bool magnificationActive = false;
	if (m_image->m_magnificationWindow && m_image->m_magnificationWindow->isVisible()) {
		magnificationActive = true;
		m_image->m_magnificationWindow->setVisible(false);
	}
	scene()->render(&painter, QRectF(), scene_rect);
	m_image->setPrinting(false);
	painter.end();

	if (magnificationActive)
		m_image->m_magnificationWindow->setVisible(true);
}

void DatapickerImageView::updateBackground() {
	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}
