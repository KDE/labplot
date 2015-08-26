/***************************************************************************
    File                 : ImageView.cpp
    Project              : LabPlot
    Description          : Image view for datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)

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

#include "commonfrontend/datapicker/ImageView.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/datapicker/CustomItem.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/Transform.h"
#include "backend/datapicker/DataPickerCurve.h"
#include "backend/datapicker/Image.h"

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
 * \class ImageView
 * \brief Datapicker/Image view
 */

/*!
  Constructur of the class.
  Creates a view for the Image \c image and initializes the internal model.
*/
ImageView::ImageView(Image* image) : QGraphicsView(),
    m_image(image),
    m_transform(new Transform()),
    m_mouseMode(SelectAndEditMode),
    m_selectionBandIsShown(false),
    magnificationFactor(0),
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
    handleActiveCurveChanged();
    changeRotationAngle();

    //signal/slot connections
    //for general actions
    connect( m_image, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)) );
    connect( m_image, SIGNAL(requestUpdate()), this, SLOT(updateBackground()) );
    connect( m_image, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(handleImageActions()));
    connect( m_image, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
            this, SLOT(handleImageActions()) );
    connect( m_image, SIGNAL(rotationAngleChanged(float)), this, SLOT(changeRotationAngle()) );

    //for curve actions
    connect( m_image, SIGNAL(activeCurveChanged(const DataPickerCurve*)),
             this, SLOT(handleActiveCurveChanged()) );
    connect( m_image->parentAspect(), SIGNAL(aspectAdded(const AbstractAspect*)),
             this, SLOT(handleActiveCurveChanged()) );
    connect( m_image->parentAspect(), SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
             this, SLOT(handleActiveCurveChanged()) );
}

void ImageView::initActions() {
    QActionGroup* zoomActionGroup = new QActionGroup(this);
    QActionGroup* mouseModeActionGroup = new QActionGroup(this);
    QActionGroup* plotPointsTypeActionGroup = new QActionGroup(this);
    QActionGroup* navigationActionGroup = new QActionGroup(this);
    QActionGroup* magnificationActionGroup = new QActionGroup(this);
    m_activeCurveActionGroup = new QActionGroup(this);

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
    selectAndEditModeAction = new KAction(KIcon("cursor-arrow"), i18n("Select and Edit"), mouseModeActionGroup);
    selectAndEditModeAction->setCheckable(true);

    navigationModeAction = new KAction(KIcon("input-mouse"), i18n("Navigate"), mouseModeActionGroup);
    navigationModeAction->setCheckable(true);

    zoomSelectionModeAction = new KAction(KIcon("page-zoom"), i18n("Select and Zoom"), mouseModeActionGroup);
    zoomSelectionModeAction->setCheckable(true);

    selectAndMoveModeAction = new KAction(KIcon("cursor-arrow"), i18n("Select and Move"), mouseModeActionGroup);
    selectAndMoveModeAction->setCheckable(true);

    setAxisPointsAction = new KAction(KIcon("plot-axis-points"), i18n("Set Axis Points"), plotPointsTypeActionGroup);
    setAxisPointsAction->setCheckable(true);

    setCurvePointsAction = new KAction(KIcon("xy-curve-points"), i18n("Set Curve Points"), plotPointsTypeActionGroup);
    setCurvePointsAction->setCheckable(true);

    selectSegmentAction = new KAction(KIcon("xy-curve-segments"), i18n("Select Curve Segments"), plotPointsTypeActionGroup);
    selectSegmentAction->setCheckable(true);

    addCurveAction = new KAction(KIcon("xy-curve"), i18n("New Curve"), this);

    shiftLeftAction = new KAction(KIcon("shift-left-x"), i18n("Shift Left"), navigationActionGroup);
    shiftLeftAction->setShortcut(Qt::Key_Right);

    shiftRightAction = new KAction(KIcon("shift-right-x"), i18n("Shift Right"), navigationActionGroup);
    shiftRightAction->setShortcut(Qt::Key_Left);

    shiftUpAction = new KAction(KIcon("shift-down-y"), i18n("Shift Up"), navigationActionGroup);
    shiftUpAction->setShortcut(Qt::Key_Up);

    shiftDownAction = new KAction(KIcon("shift-up-y"), i18n("Shift Down"), navigationActionGroup);
    shiftDownAction->setShortcut(Qt::Key_Down);

    noMagnificationAction = new KAction(KIcon("1-to-1-zoom"), i18n("No Magnification"), magnificationActionGroup);
    noMagnificationAction->setCheckable(true);
    noMagnificationAction->setChecked(true);

    twoTimesMagnificationAction = new KAction(KIcon("1-to-2-zoom"), i18n("2x Magnification"), magnificationActionGroup);
    twoTimesMagnificationAction->setCheckable(true);

    threeTimesMagnificationAction = new KAction(KIcon("1-to-3-zoom"), i18n("3x Magnification"), magnificationActionGroup);
    threeTimesMagnificationAction->setCheckable(true);

    fourTimesMagnificationAction = new KAction(KIcon("1-to-4-zoom"), i18n("4x Magnification"), magnificationActionGroup);
    fourTimesMagnificationAction->setCheckable(true);

    fiveTimesMagnificationAction = new KAction(KIcon("1-to-5-zoom"), i18n("5x Magnification"), magnificationActionGroup);
    fiveTimesMagnificationAction->setCheckable(true);

    connect( mouseModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(mouseModeChanged(QAction*)) );
    connect( zoomActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeZoom(QAction*)) );
    connect( plotPointsTypeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changePointsType(QAction*)) );
    connect( addCurveAction, SIGNAL(triggered()), this, SLOT(addCurve()) );
    connect( navigationActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeSelectedItemsPosition(QAction*)) );
    connect( magnificationActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(magnificationChanged(QAction*)) );
    connect( m_activeCurveActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(activeCurveChanged(QAction*)) );
}

void ImageView::initMenus() {

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

    m_activeCurveMenu = new QMenu(i18n("Active Curve"));

    m_curveMenu = new QMenu(i18n("Curves"));
    m_curveMenu->addAction(addCurveAction);
    m_curveMenu->addSeparator();
    m_curveMenu->addMenu(m_activeCurveMenu);
}

/*!
 * Populates the menu \c menu with the image and image-view relevant actions.
 * The menu is used
 *   - as the context menu in ImageView
 *   - as the "datapicker menu" in the main menu-bar (called form MainWin)
 *   - as a part of the image context menu in project explorer
 */
void ImageView::createContextMenu(QMenu* menu) const {
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
    menu->insertMenu(firstAction, m_curveMenu);
    menu->insertSeparator(firstAction);
    menu->insertMenu(firstAction, m_navigationMenu);
    menu->insertSeparator(firstAction);
}

void ImageView::fillToolBar(QToolBar* toolBar) {
    toolBar->addSeparator();
    toolBar->addAction(selectAndEditModeAction);
    toolBar->addAction(navigationModeAction);
    toolBar->addAction(zoomSelectionModeAction);
    toolBar->addAction(selectAndMoveModeAction);
    toolBar->addSeparator();
    toolBar->addAction(setAxisPointsAction);
    toolBar->addAction(setCurvePointsAction);
    toolBar->addAction(selectSegmentAction);
    toolBar->addSeparator();
    toolBar->addAction(addCurveAction);
    tbZoom = new QToolButton(toolBar);
    tbZoom->setPopupMode(QToolButton::MenuButtonPopup);
    tbZoom->setMenu(m_zoomMenu);
    tbZoom->setDefaultAction(currentZoomAction);
    toolBar->addWidget(tbZoom);
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
}

void ImageView::setScene(QGraphicsScene* scene) {
    QGraphicsView::setScene(scene);
    setTransform(QTransform());
}

void ImageView::drawForeground(QPainter* painter, const QRectF& rect) {
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

void ImageView::drawBackground(QPainter* painter, const QRectF& rect) {
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
        painter->translate(sceneRect().width()/2, sceneRect().height()/2);
        painter->translate(-sceneRect().width()/2, -sceneRect().height()/2);

        if (m_image->plotImageType == Image::OriginalImage) {
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
void ImageView::wheelEvent(QWheelEvent *event) {
    if (m_mouseMode == ZoomSelectionMode) {
        if (event->delta() > 0)
            scale(1.2, 1.2);
        else if (event->delta() < 0)
            scale(1.0/1.2, 1.0/1.2);
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void ImageView::mousePressEvent(QMouseEvent* event) {
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

    QPointF eventPos = mapToScene(event->pos());
    if ( m_mouseMode == SelectAndEditMode && m_image->isLoaded && sceneRect().contains(eventPos)) {
        if (m_image->plotPointsType() == Image::AxisPoints) {
            addAxisPoint(eventPos);
        } else if (m_image->plotPointsType() == Image::CurvePoints) {
            if (m_image->activeCurve())
                m_image->activeCurve()->addCustomItem(eventPos);
        }
    }

    // select the datapicker/image in the project explorer if the view was clicked
    // and there is no selection currently. We need this for the case when
    // there is a single datapicker/image in the project and we change from the project-node
    // in the project explorer to the datapicker/image-node by clicking the view.
    if ( scene()->selectedItems().empty() )
        m_image->setSelectedInView(true);

    QGraphicsView::mousePressEvent(event);
}

void ImageView::mouseReleaseEvent(QMouseEvent* event) {
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

void ImageView::mouseMoveEvent(QMouseEvent* event) {
    if ( m_mouseMode == SelectAndEditMode || m_mouseMode == ZoomSelectionMode )
        setCursor(Qt::CrossCursor);
    else
        setCursor(Qt::ArrowCursor);

    if (m_selectionBandIsShown) {
        m_selectionEnd = event->pos();
        viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());
    }

    QPointF pos = mapToScene(event->pos());

    if (m_image->plotPointsType() == Image::CurvePoints) {
        QPointF logicalPos = m_transform->mapSceneToLogical(pos, m_image->axisPoints());
        QString xLabel = "x";
        QString yLabel = "y";
        if (m_image->axisPoints().type == Image::PolarInDegree){
            xLabel = "r";
            yLabel = "y(deg)";
        } else if (m_image->axisPoints().type == Image::PolarInRadians) {
            xLabel = "r";
            yLabel = "y(rad)";
        }
        emit statusInfo( xLabel + "=" + QString::number(logicalPos.x()) + ", " + yLabel + "=" + QString::number(logicalPos.y()) );
    }

    if ( magnificationFactor && m_image->isLoaded && sceneRect().contains(pos)
         && m_image->plotPointsType() != Image::SegmentPoints ) {
        if (!m_image->m_magnificationWindow) {
            m_image->m_magnificationWindow = new QGraphicsPixmapItem(0, scene());
            m_image->m_magnificationWindow->setZValue(-1);
        }

        int size = 100;
        QImage imageSection = m_image->originalPlotImage.scaled(scene()->width(), scene()->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        imageSection = imageSection.copy(pos.x() - size/2, pos.y() - size/2, size, size);
        imageSection = imageSection.scaled(size*magnificationFactor, size*magnificationFactor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        imageSection = imageSection.copy(imageSection.width()/2 - size/2, imageSection.height()/2 - size/2, size, size);

        m_image->m_magnificationWindow->setVisible(true);
        m_image->m_magnificationWindow->setPixmap(QPixmap::fromImage(imageSection));
        m_image->m_magnificationWindow->setPos(pos.x()- imageSection.width()/2, pos.y()- imageSection.height()/2);
    } else if (m_image->m_magnificationWindow) {
        m_image->m_magnificationWindow->setVisible(false);
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ImageView::contextMenuEvent(QContextMenuEvent* e) {
    Q_UNUSED(e);
    //no need to propagate the event to the scene and graphics items
    QMenu *menu = new QMenu(this);
    this->createContextMenu(menu);
    menu->exec(QCursor::pos());
}

//##############################################################################
//####################################  SLOTs   ###############################
//##############################################################################
void ImageView::changePointsType(QAction* action) {
    if (action==setAxisPointsAction) {
        //clear image
        int childCount = m_image->childCount<CustomItem>(AbstractAspect::IncludeHidden);
        if (childCount)
            m_image->removeAllChildren();
        m_image->setPlotPointsType(Image::AxisPoints);
        m_image->setSegmentVisible(false);
    } else if (action==setCurvePointsAction) {
        m_image->setPlotPointsType(Image::CurvePoints);
        m_image->setSegmentVisible(false);
    } else if (action==selectSegmentAction) {
        m_image->setPlotPointsType(Image::SegmentPoints);
        m_image->setSegmentVisible(true);
    }
}

void ImageView::activeCurveChanged(QAction* newAction) {
    //find the index of curve from index of action
    int index = 0;
    QList<DataPickerCurve*> curveList = m_image->parentAspect()->children<DataPickerCurve>();
    foreach (QAction* action, m_activeCurveMenu->actions()) {
        if (action == newAction) {
            m_image->setActiveCurve(curveList.at(index));
            break;
        }
        index++;
    }
}

void ImageView::changeZoom(QAction* action) {
    if (action==zoomInViewAction) {
        scale(1.2, 1.2);
    } else if (action==zoomOutViewAction) {
        scale(1.0/1.2, 1.0/1.2);
    } else if (action==zoomOriginAction) {
        static const float hscale = QApplication::desktop()->physicalDpiX()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
        static const float vscale = QApplication::desktop()->physicalDpiY()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
        setTransform(QTransform::fromScale(hscale, vscale));
    } else if (action==zoomFitPageWidthAction) {
        float scaleFactor = viewport()->width()/scene()->sceneRect().width();
        setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
    } else if (action==zoomFitPageHeightAction) {
        float scaleFactor = viewport()->height()/scene()->sceneRect().height();
        setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
    }

    currentZoomAction=action;
    if (tbZoom)
        tbZoom->setDefaultAction(action);
}

void ImageView::changeSelectedItemsPosition(QAction* action) {
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

    m_image->beginMacro(i18n("%1: change position of selected CustomItems.", m_image->name()));
    QList<CustomItem*> axisPointsItems = m_image->children<CustomItem>(AbstractAspect::IncludeHidden);
    foreach (CustomItem* item, axisPointsItems) {
        if (!item->graphicsItem()->isSelected())
            continue;

        item->setPosition(item->position().point + shift);

        int itemIndex = m_image->indexOfChild<CustomItem>(item , AbstractAspect::IncludeHidden);
        Image::ReferencePoints points = m_image->axisPoints();
        points.scenePos[itemIndex].setX(item->position().point.x());
        points.scenePos[itemIndex].setY(item->position().point.y());
        m_image->setUndoAware(false);
        m_image->setAxisPoints(points);
        m_image->setUndoAware(true);
    }

    foreach (DataPickerCurve* curve, m_image->parentAspect()->children<DataPickerCurve>()) {
        foreach (CustomItem* item, curve->children<CustomItem>(AbstractAspect::IncludeHidden)) {
            if (!item->graphicsItem()->isSelected())
                continue;
            item->setPosition(item->position().point + shift);
        }
    }

    m_image->endMacro();
}

void ImageView::mouseModeChanged(QAction* action) {
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

void ImageView::magnificationChanged(QAction* action) {
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

void ImageView::exportToFile(const QString& path, const ExportFormat format, const int resolution) {
    QRectF sourceRect;
    sourceRect = scene()->sceneRect();

    //print
    if (format==ImageView::Pdf || format==ImageView::Eps) {
        QPrinter printer(QPrinter::HighResolution);
        if (format==ImageView::Pdf)
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
    } else if (format==ImageView::Svg) {
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

void ImageView::exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect) {
    painter->save();
    painter->scale(targetRect.width()/sourceRect.width(), targetRect.height()/sourceRect.height());
    drawBackground(painter, sourceRect);
    painter->restore();
    m_image->setPrinting(true);
    scene()->render(painter, QRectF(), sourceRect);
    m_image->setPrinting(false);
}

void ImageView::print(QPrinter* printer) const {
    m_image->setPrinting(true);
    QPainter painter(printer);
    painter.setRenderHint(QPainter::Antialiasing);
    scene()->render(&painter);
    m_image->setPrinting(false);
}

void ImageView::updateBackground() {
    invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
    handleImageActions();
}

void ImageView::addCurve() {
    m_image->beginMacro(i18n("%1: add new curve.", m_image->name()));
    Datapicker* datapicker = dynamic_cast<Datapicker*>(m_image->parentAspect());
    Q_ASSERT(datapicker);
    DataPickerCurve* curve = new DataPickerCurve(i18n("Curve"));
    datapicker->addChild(curve);
    m_image->setActiveCurve(curve);
    curve->setCurveErrorTypes(m_image->plotErrors());
    m_image->endMacro();
}

void ImageView::addAxisPoint(const QPointF& pos) {
    QList<CustomItem*> childItems = m_image->children<CustomItem>(AbstractAspect::IncludeHidden);
    if (childItems.count() > 2)
        return;

    CustomItem* newItem = new CustomItem(i18n("Curve Point"));
    newItem->setPosition(pos);
    newItem->setHidden(true);

    if (!childItems.isEmpty()) {
        CustomItem* m_item = childItems.first();
        newItem->setUndoAware(false);
        newItem->setItemsBrush(m_item->itemsBrush());
        newItem->setItemsOpacity(m_item->itemsOpacity());
        newItem->setItemsPen(m_item->itemsPen());
        newItem->setItemsRotationAngle(m_item->itemsRotationAngle());
        newItem->setItemsSize(m_item->itemsSize());
        newItem->setItemsStyle(m_item->itemsStyle());
        newItem->setUndoAware(true);
    }

    m_image->addChild(newItem);

    Image::ReferencePoints points = m_image->axisPoints();
    points.scenePos[childItems.count()].setX(pos.x());
    points.scenePos[childItems.count()].setY(pos.y());
    m_image->setUndoAware(false);
    m_image->setAxisPoints(points);
    m_image->setUndoAware(true);
}

void ImageView::changeRotationAngle() {
    this->rotate(-m_image->rotationAngle());
    updateBackground();
}

void ImageView::handleActiveCurveChanged() {
    //update m_activeCurveMenu
    QList<DataPickerCurve*> curveList = m_image->parentAspect()->children<DataPickerCurve>();
    if (curveList.isEmpty()) {
        m_activeCurveMenu->setEnabled(false);
        return;
    } else {
        m_activeCurveMenu->setEnabled(true);
    }

    m_activeCurveMenu->clear();
    foreach (DataPickerCurve* curve, curveList) {
        QAction* action = new KAction(KIcon(""), curve->name(), m_activeCurveActionGroup);
        action->setCheckable(true);
        action->setChecked(m_image->activeCurve() == curve);
        m_activeCurveMenu->addAction(action);
    }
}

void ImageView::handleImageActions() {
    int axisPointsCount = m_image->childCount<CustomItem>(AbstractAspect::IncludeHidden);

    if (m_image->isLoaded) {
        setCurvePointsAction->setEnabled(false);
        selectSegmentAction->setEnabled(false);
        setAxisPointsAction->setEnabled(true);
        addCurveAction->setEnabled(true);

        if (axisPointsCount > 2) {
            if (m_image->activeCurve()) {
                setCurvePointsAction->setEnabled(true);
                selectSegmentAction->setEnabled(true);
            }
        } else {
            if (m_image->plotPointsType() != Image::AxisPoints) {
                m_image->setUndoAware(false);
                m_image->setPlotPointsType(Image::AxisPoints);
                m_image->setUndoAware(true);
            }
        }
    } else {
        setAxisPointsAction->setEnabled(false);
        setCurvePointsAction->setEnabled(false);
        selectSegmentAction->setEnabled(false);
        addCurveAction->setEnabled(false);
    }

    setAxisPointsAction->setChecked(m_image->plotPointsType() == Image::AxisPoints);
    setCurvePointsAction->setChecked(m_image->plotPointsType() == Image::CurvePoints);
    selectSegmentAction->setChecked(m_image->plotPointsType() == Image::SegmentPoints);
}
