#include "commonfrontend/datapicker/ImageView.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetElementGroup.h"
#include "backend/worksheet/CustomItem.h"

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
    selectionModeAction->setChecked(true);
    //handleCartesianPlotActions();

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

void ImageView::initActions(){
    QActionGroup* zoomActionGroup = new QActionGroup(this);
    QActionGroup* mouseModeActionGroup = new QActionGroup(this);

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

    setReferencePointsAction = new QAction(i18n("Set Reference Point"), this);

    setCurvePointsAction = new QAction(i18n("Set Curve Points"), this);

    connect(setReferencePointsAction, SIGNAL(triggered()), this, SLOT(setReferencePoints()));
    connect(setCurvePointsAction, SIGNAL(triggered()), this, SLOT(setCurvePoints()));
    connect(mouseModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(mouseModeChanged(QAction*)));
    connect(zoomActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeZoom(QAction*)));
}

void ImageView::initMenus(){

    m_viewMouseModeMenu = new QMenu(i18n("Mouse Mode"));
    m_viewMouseModeMenu->setIcon(KIcon("input-mouse"));
    m_viewMouseModeMenu->addAction(selectionModeAction);
    m_viewMouseModeMenu->addAction(navigationModeAction);
    m_viewMouseModeMenu->addAction(zoomSelectionModeAction);

    m_viewImageMenu = new QMenu(i18n("Image Menu"));
    //m_viewImageMenu->setIcon();
    m_viewImageMenu->addAction(setReferencePointsAction);
    m_viewImageMenu->addAction(setCurvePointsAction);

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
}

void ImageView::fillToolBar(QToolBar* toolBar){
    toolBar->addSeparator();
    toolBar->addAction(selectionModeAction);
    toolBar->addAction(navigationModeAction);
    toolBar->addAction(zoomSelectionModeAction);
    toolBar->addAction(setReferencePointsAction);
    toolBar->addAction(setCurvePointsAction);
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

        if (m_image->plotImageType == Image::OriginalImage)
            painter->drawImage(scene_rect.topLeft(),m_image->originalPlotImage);
        else
            painter->drawImage(scene_rect.topLeft(),m_image->processedPlotImage);
    } else {
        painter->setBrush(QBrush(Qt::white));
        painter->drawRect(scene_rect);
    }

    invalidateScene(rect, QGraphicsScene::BackgroundLayer);
    painter->restore();
}

//##############################################################################
//####################################  Events   ###############################
//##############################################################################
void ImageView::wheelEvent(QWheelEvent *event) {
    if (m_mouseMode == ZoomSelectionMode){
        if (event->delta() > 0)
            scale(1.2, 1.2);
        else if (event->delta() < 0)
            scale(1.0/1.2, 1.0/1.2);
    }else{
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

    if ( m_mouseMode == SelectionMode && m_image->drawPoints() ) {
        CustomItem* item = new CustomItem(i18n("item"));
        item->setPosition(mapToScene(event->pos()));
        item->setHidden(true);
        m_image->addChild(item);
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
        setCursor(Qt::ArrowCursor);
    }else if (m_selectionBandIsShown) {
        m_selectionEnd = event->pos();
        viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ImageView::contextMenuEvent(QContextMenuEvent* e) {
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

//  SLOTs
void ImageView::setReferencePoints() {
    //clear image
    m_image->removeAllChildren();
    m_image->setDrawPoints(true);
}

void ImageView::setCurvePoints() {
    m_image->setDrawPoints(true);
}

void ImageView::changeZoom(QAction* action){
    if (action==zoomInViewAction){
        scale(1.2, 1.2);
    }else if (action==zoomOutViewAction){
        scale(1.0/1.2, 1.0/1.2);
    }else if (action==zoomOriginAction){
        static const float hscale = QApplication::desktop()->physicalDpiX()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
        static const float vscale = QApplication::desktop()->physicalDpiY()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
        setTransform(QTransform::fromScale(hscale, vscale));
    }else if (action==zoomFitPageWidthAction){
        float scaleFactor = viewport()->width()/scene()->sceneRect().width();
        setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
    }else if (action==zoomFitPageHeightAction){
        float scaleFactor = viewport()->height()/scene()->sceneRect().height();
        setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
    }else if (action==zoomFitSelectionAction){
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
    if (format==ImageView::Pdf || format==ImageView::Eps){
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
    }else if (format==ImageView::Svg){
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

void ImageView::handleImageActions() {
    int count = m_image->childCount<CustomItem>(AbstractAspect::IncludeHidden);
    if (m_image->isLoaded){
        if (count > 2)
            setCurvePointsAction->setEnabled(true);
        else
            setCurvePointsAction->setEnabled(false);
        setReferencePointsAction->setEnabled(true);
    }else {
        setReferencePointsAction->setEnabled(false);
        setCurvePointsAction->setEnabled(false);
    }
}
