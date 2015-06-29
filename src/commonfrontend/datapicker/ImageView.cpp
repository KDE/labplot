#include "commonfrontend/datapicker/ImageView.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetElementGroup.h"
#include "backend/worksheet/CustomItem.h"
#include "backend/core/Datapicker.h"
#include "backend/core/Transform.h"

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

ImageView::ImageView(Image* image) : QGraphicsView(),
    m_image(image),
    m_mouseMode(SelectionMode),
    m_selectionBandIsShown(false),
    tbZoom(0),
    m_transform(new Transform()){

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
    selectionModeAction->setChecked(true);

    changeZoom(zoomOriginAction);
    currentZoomAction=zoomInViewAction;
    handleImageActions();

    //signal/slot connections
    connect(m_image, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
    connect(m_image, SIGNAL(requestUpdate()), this, SLOT(updateBackground()) );
    connect(m_image, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(handleImageActions()));
    connect(m_image, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
            this, SLOT(handleImageActions()));
}

void ImageView::initActions() {
    QActionGroup* zoomActionGroup = new QActionGroup(this);
    QActionGroup* mouseModeActionGroup = new QActionGroup(this);
    QActionGroup* plotPointsTypeActionGroup = new QActionGroup(this);

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

    setAxisPointsAction = new KAction(KIcon(""), i18n("Set Axis Points"), plotPointsTypeActionGroup);
    setAxisPointsAction->setCheckable(true);

    setCurvePointsAction = new KAction(KIcon(""), i18n("Set Curve Points"), plotPointsTypeActionGroup);
    setCurvePointsAction->setCheckable(true);

    selectSegmentAction = new KAction(KIcon(""), i18n("Select Curve Segments"), plotPointsTypeActionGroup);
    selectSegmentAction->setCheckable(true);

    updateDatasheetAction = new KAction(KIcon(""), i18n("Update Datasheet"), this);

    connect(mouseModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(mouseModeChanged(QAction*)));
    connect(zoomActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeZoom(QAction*)));
    connect(plotPointsTypeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changePointsType(QAction*)));
    connect(updateDatasheetAction, SIGNAL(triggered()), this, SLOT(updateDatasheet()) );
}

void ImageView::initMenus() {

    m_viewMouseModeMenu = new QMenu(i18n("Mouse Mode"));
    m_viewMouseModeMenu->setIcon(KIcon("input-mouse"));
    m_viewMouseModeMenu->addAction(selectionModeAction);
    m_viewMouseModeMenu->addAction(navigationModeAction);
    m_viewMouseModeMenu->addAction(zoomSelectionModeAction);

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
    m_zoomMenu->addAction(zoomFitSelectionAction);
}

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
    menu->insertSeparator(firstAction);
    menu->insertMenu(firstAction, m_viewImageMenu);
    menu->insertSeparator(firstAction);
    menu->insertAction(firstAction, updateDatasheetAction);
    menu->insertSeparator(firstAction);
}

void ImageView::fillToolBar(QToolBar* toolBar) {
    toolBar->addSeparator();
    toolBar->addAction(selectionModeAction);
    toolBar->addAction(navigationModeAction);
    toolBar->addAction(zoomSelectionModeAction);
    toolBar->addAction(setAxisPointsAction);
    toolBar->addAction(setCurvePointsAction);
    toolBar->addAction(selectSegmentAction);
    toolBar->addAction(updateDatasheetAction);
    tbZoom = new QToolButton(toolBar);
    tbZoom->setPopupMode(QToolButton::MenuButtonPopup);
    tbZoom->setMenu(m_zoomMenu);
    tbZoom->setDefaultAction(currentZoomAction);
    toolBar->addWidget(tbZoom);
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
        painter->rotate(-m_image->rotationAngle());

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

CustomItem *ImageView::addCustomItem(const QPointF& position) {
    CustomItem* item = new CustomItem(i18n("item"));
    item->setPosition(position);
    item->setHidden(true);
    m_image->addChild(item);
    return item;
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

    if (event->button() != Qt::LeftButton) {
        event->accept();
        return;
    }

    if ( m_mouseMode == SelectionMode && m_image->isLoaded ) {
        QList<CustomItem*> childItems = m_image->children<CustomItem>(AbstractAspect::IncludeHidden);
        CustomItem* lastCurvePoint;
        QPointF eventPos = mapToScene(event->pos());
        if (m_image->plotPointsType() == Image::AxisPoints && childItems.count() < 3) {
            addCustomItem(eventPos);

            Image::ReferencePoints points = m_image->axisPoints();
            points.scenePos[childItems.count() - 1].setX(eventPos.x());
            points.scenePos[childItems.count() - 1].setY(eventPos.y());
            m_image->setAxisPoints(points);
        } else if (m_image->plotPointsType() == Image::CurvePoints) {

            if (childItems.count() == 3) {
                lastCurvePoint = addCustomItem(eventPos);
            } else {
                lastCurvePoint = childItems.last();
                CustomItem::ErrorBar errorBar = lastCurvePoint->itemErrorBar();
                QPoint errorSpan = event->pos() - mapFromScene(lastCurvePoint->position().point);

                if (m_image->plotErrors().x == Image::AsymmetricError && !errorBar.minusDeltaX) {
                    if (!errorBar.plusDeltaX)
                        errorBar.plusDeltaX = abs(errorSpan.x());
                    else
                        errorBar.minusDeltaX = abs(errorSpan.x());

                    lastCurvePoint->setItemErrorBar(errorBar);
                } else if (m_image->plotErrors().x == Image::SymmetricError && !errorBar.plusDeltaX) {
                    errorBar.plusDeltaX = abs(errorSpan.x());
                    errorBar.minusDeltaX = errorBar.plusDeltaX;

                    lastCurvePoint->setItemErrorBar(errorBar);
                } else if (m_image->plotErrors().y == Image::AsymmetricError && !errorBar.minusDeltaY) {
                    if (!errorBar.plusDeltaY)
                        errorBar.plusDeltaY = abs(errorSpan.y());
                    else
                        errorBar.minusDeltaY = abs(errorSpan.y());

                    lastCurvePoint->setItemErrorBar(errorBar);
                } else if (m_image->plotErrors().y == Image::SymmetricError && !errorBar.plusDeltaY) {
                    errorBar.plusDeltaY = abs(errorSpan.y());
                    errorBar.minusDeltaY = errorBar.plusDeltaY;

                    lastCurvePoint->setItemErrorBar(errorBar);
                } else {
                    lastCurvePoint = addCustomItem(eventPos);
                }
            }

            updateData(lastCurvePoint);
        } else if (m_image->plotPointsType() == Image::SegmentPoints) {
            //Todo
        }
    }

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
    if (m_mouseMode == SelectionMode ) {
        setCursor(Qt::CrossCursor);
    } else if (m_selectionBandIsShown) {
        m_selectionEnd = event->pos();
        viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ImageView::contextMenuEvent(QContextMenuEvent* e) {
    Q_UNUSED(e);
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
        m_image->removeAllChildren();
        m_image->setPlotPointsType(Image::AxisPoints);
    } else if (action==setCurvePointsAction){
        m_image->setPlotPointsType(Image::CurvePoints);
    } else if (action==selectSegmentAction){
        m_image->setPlotPointsType(Image::SegmentPoints);
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
    } else if (action==zoomFitSelectionAction) {
        fitInView(scene()->selectionArea().boundingRect(),Qt::KeepAspectRatio);
    }
    currentZoomAction=action;
    if (tbZoom)
        tbZoom->setDefaultAction(action);
}

void ImageView::mouseModeChanged(QAction* action) {
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

void ImageView::updateDatasheet() {
    QList<CustomItem*> childElements = m_image->children<CustomItem>(AbstractAspect::IncludeHidden);
    if (childElements.count() > 3) {
        //remove axis points
        childElements.removeAt(0);
        childElements.removeAt(1);
        childElements.removeAt(2);

        foreach(CustomItem* elem, childElements)
            updateData(elem);
    }
}

void ImageView::updateData(const CustomItem *item) {
    m_image->updateAxisPoints();

    QPointF data;
    Datapicker* datapicker = dynamic_cast<Datapicker*>(m_image->parentAspect());
    int row = m_image->childCount<CustomItem>(AbstractAspect::IncludeHidden) - 4;

    int colCount = 0;
    if (datapicker) {
        data = m_transform->mapSceneToLogical(item->position().point, m_image->axisPoints());
        datapicker->addDataToSheet(data.x(), colCount++, row, "x");
        datapicker->addDataToSheet(data.y(), colCount++, row, "y");

        if (m_image->plotErrors().x != Image::NoError) {
            data = m_transform->mapSceneToLogical(mapToScene(QPoint(item->itemErrorBar().plusDeltaX, 0)), m_image->axisPoints());
            datapicker->addDataToSheet(data.x(), colCount++, row, "+delta_x");

            if (m_image->plotErrors().x == Image::AsymmetricError) {
                data = m_transform->mapSceneToLogical(mapToScene(QPoint(item->itemErrorBar().minusDeltaX, 0)), m_image->axisPoints());
                datapicker->addDataToSheet(data.x(), colCount++, row, "-delta_x");
            }
        }

        if (m_image->plotErrors().y != Image::NoError) {
            data = m_transform->mapSceneToLogical(mapToScene(QPoint(0, item->itemErrorBar().plusDeltaY)), m_image->axisPoints());
            datapicker->addDataToSheet(data.y(), colCount++, row, "+delta_y");

            if (m_image->plotErrors().y == Image::AsymmetricError) {
                data = m_transform->mapSceneToLogical(mapToScene(QPoint(0, item->itemErrorBar().minusDeltaY)), m_image->axisPoints());
                datapicker->addDataToSheet(data.y(), colCount++, row, "-delta_y");
            }
        }
    }
}

void ImageView::handleImageActions() {
    int count = m_image->childCount<CustomItem>(AbstractAspect::IncludeHidden);
    if (m_image->isLoaded) {
        if (count > 2){
            setCurvePointsAction->setEnabled(true);
            selectSegmentAction->setEnabled(true);
        } else {
            setCurvePointsAction->setEnabled(false);
            selectSegmentAction->setEnabled(false);
        }

        setAxisPointsAction->setEnabled(true);
    } else {
        setAxisPointsAction->setEnabled(false);
        setCurvePointsAction->setEnabled(false);
        selectSegmentAction->setEnabled(false);
    }

    setAxisPointsAction->setChecked(m_image->plotPointsType() == Image::AxisPoints);
    setCurvePointsAction->setChecked(m_image->plotPointsType() == Image::CurvePoints);
    selectSegmentAction->setChecked(m_image->plotPointsType() == Image::SegmentPoints);
}
