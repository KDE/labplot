#include "commonfrontend/datapicker/ImageReaderElement.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetElementGroup.h"
#include "backend/worksheet/Image.h"

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
#include <QFileDialog>

#include <KAction>
#include <KLocale>
#include <KMessageBox>



ImageReaderElement::ImageReaderElement(Worksheet* worksheet) : QGraphicsView(),
	m_worksheet(worksheet),
	m_mouseMode(SelectionMode),
	m_selectionBandIsShown(false),
	m_suppressSelectionChangedEvent(false),
	tbZoom(0) {

	setScene(m_worksheet->scene());

	setRenderHint(QPainter::Antialiasing);
	setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setMinimumSize(16, 16);
	setFocusPolicy(Qt::StrongFocus);

	if (m_worksheet->useViewSize()) {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}

	viewport()->setAttribute( Qt::WA_OpaquePaintEvent );
	viewport()->setAttribute( Qt::WA_NoSystemBackground );
	setCacheMode(QGraphicsView::CacheBackground);

	initActions();
	initMenus();
	selectionModeAction->setChecked(true);

	changeZoom(zoomOriginAction);
	currentZoomAction=zoomInViewAction;

    connect(m_worksheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_worksheet, SIGNAL(itemSelected(QGraphicsItem*)), this, SLOT(selectItem(QGraphicsItem*)) );
	connect(m_worksheet, SIGNAL(itemDeselected(QGraphicsItem*)), this, SLOT(deselectItem(QGraphicsItem*)) );
	connect(m_worksheet, SIGNAL(requestUpdate()), this, SLOT(updateBackground()) );
	connect(m_worksheet, SIGNAL(useViewSizeRequested()), this, SLOT(useViewSizeRequested()) );
    connect(m_worksheet, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
            this, SLOT(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)));
	connect(scene(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()) );
}

void ImageReaderElement::initActions(){
	QActionGroup* zoomActionGroup = new QActionGroup(this);
	QActionGroup* mouseModeActionGroup = new QActionGroup(this);

    setReferencePointsAction = new KAction(KIcon(""), i18n("Set Reference Points"), this);
    connect(setReferencePointsAction, SIGNAL(triggered()), SLOT(setImageReferencePoints()));

    setCurvePointsAction = new KAction(KIcon(""), i18n("Set Curve Points"), this);
    connect(setCurvePointsAction, SIGNAL(triggered()), SLOT(setImageCurvePoints()));

	selectAllAction = new KAction(KIcon("edit-select-all"), i18n("Select all"), this);
	selectAllAction->setShortcut(Qt::CTRL+Qt::Key_A);
	this->addAction(selectAllAction);
	connect(selectAllAction, SIGNAL(triggered()), SLOT(selectAllElements()));

	deleteAction = new KAction(KIcon("edit-delete"), i18n("Delete"), this);
	deleteAction->setShortcut(Qt::Key_Delete);
	this->addAction(deleteAction);
	connect(deleteAction, SIGNAL(triggered()), SLOT(deleteElement()));

	backspaceAction = new KAction(this);
	backspaceAction->setShortcut(Qt::Key_Backspace);
	this->addAction(backspaceAction);
	connect(backspaceAction, SIGNAL(triggered()), SLOT(deleteElement()));

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

    addImageAction = new KAction(KIcon(""), i18n("new image"), this);
    connect(addImageAction, SIGNAL(triggered()), this, SLOT(addNewImage()));

    connect(mouseModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(mouseModeChanged(QAction*)));
	connect(zoomActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeZoom(QAction*)));
}

void ImageReaderElement::initMenus(){
	m_viewMouseModeMenu = new QMenu(i18n("Mouse Mode"));
	m_viewMouseModeMenu->setIcon(KIcon("input-mouse"));
	m_viewMouseModeMenu->addAction(selectionModeAction);
	m_viewMouseModeMenu->addAction(navigationModeAction);
	m_viewMouseModeMenu->addAction(zoomSelectionModeAction);

	m_zoomMenu = new QMenu(i18n("Zoom"));
	m_zoomMenu->setIcon(KIcon("zoom-draw"));
	m_zoomMenu->addAction(zoomInViewAction);
	m_zoomMenu->addAction(zoomOutViewAction);
	m_zoomMenu->addAction(zoomOriginAction);
	m_zoomMenu->addAction(zoomFitPageHeightAction);
	m_zoomMenu->addAction(zoomFitPageWidthAction);
	m_zoomMenu->addAction(zoomFitSelectionAction);

    m_imageMenu = new QMenu(i18n("Image Menu"));
    m_imageMenu->addAction(setReferencePointsAction);
    m_imageMenu->addAction(setCurvePointsAction);
}


void ImageReaderElement::createContextMenu(QMenu* menu) const {
	Q_ASSERT(menu);

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QAction* firstAction = menu->actions().first();
#else
	QAction* firstAction = 0;
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);
#endif

    menu->insertAction(firstAction, addImageAction);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_viewMouseModeMenu);
	menu->insertMenu(firstAction, m_zoomMenu);
	menu->insertSeparator(firstAction);
    menu->insertMenu(firstAction, m_imageMenu);
	menu->insertSeparator(firstAction);
}

void ImageReaderElement::fillToolBar(QToolBar* toolBar){
	toolBar->addSeparator();
    toolBar->addAction(addImageAction);
	toolBar->addSeparator();
	toolBar->addAction(selectionModeAction);
	toolBar->addAction(navigationModeAction);
	toolBar->addAction(zoomSelectionModeAction);
	tbZoom = new QToolButton(toolBar);
	tbZoom->setPopupMode(QToolButton::MenuButtonPopup);
	tbZoom->setMenu(m_zoomMenu);
	tbZoom->setDefaultAction(currentZoomAction);
	toolBar->addWidget(tbZoom);
}

void ImageReaderElement::setScene(QGraphicsScene* scene) {
  QGraphicsView::setScene(scene);
  setTransform(QTransform());
}

void ImageReaderElement::drawForeground(QPainter* painter, const QRectF& rect) {
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

void ImageReaderElement::drawBackground(QPainter* painter, const QRectF& rect) {
	painter->save();

	QRectF scene_rect = sceneRect();

	if (!m_worksheet->useViewSize()) {

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
	}
	invalidateScene(rect, QGraphicsScene::BackgroundLayer);
	painter->restore();
}

//Events

void ImageReaderElement::resizeEvent(QResizeEvent *event) {
	if (m_worksheet->useViewSize())
		this->processResize();

	QGraphicsView::resizeEvent(event);
}

void ImageReaderElement::wheelEvent(QWheelEvent *event) {
  if (m_mouseMode == ZoomSelectionMode){
	if (event->delta() > 0)
		scale(1.2, 1.2);
	else if (event->delta() < 0)
		scale(1.0/1.2, 1.0/1.2);
  }else{
	QGraphicsView::wheelEvent(event);
  }
}

void ImageReaderElement::mousePressEvent(QMouseEvent* event) {
	if (m_mouseMode == ZoomSelectionMode) {
		m_selectionStart = event->pos();
		m_selectionBandIsShown = true;
	}

	if (event->button() != Qt::LeftButton) {
        event->accept();
        return;
    }

	if ( scene()->selectedItems().empty() )
		m_worksheet->setSelectedInView(true);

	QGraphicsView::mousePressEvent(event);
}

void ImageReaderElement::mouseReleaseEvent(QMouseEvent* event) {
	if (m_mouseMode == ZoomSelectionMode) {
		m_selectionBandIsShown = false;
		viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());

		m_selectionEnd = event->pos();
		if ( abs(m_selectionEnd.x()-m_selectionStart.x())>20 && abs(m_selectionEnd.y()-m_selectionStart.y())>20 )
			fitInView(mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect(), Qt::KeepAspectRatio);
	}
	QGraphicsView::mouseReleaseEvent(event);
}

void ImageReaderElement::mouseMoveEvent(QMouseEvent* event) {
    if (m_mouseMode == SelectionMode) {
		setCursor(Qt::ArrowCursor);
	} else if (m_selectionBandIsShown) {
		m_selectionEnd = event->pos();
		viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());
    }
	QGraphicsView::mouseMoveEvent(event);
}

void ImageReaderElement::contextMenuEvent(QContextMenuEvent* e) {
	if ( !itemAt(e->pos()) ){
		QMenu *menu = new QMenu(this);
		this->createContextMenu(menu);
		menu->exec(QCursor::pos());
	}else{
		QGraphicsView::contextMenuEvent(e);
	}
}

//SLOTs

void ImageReaderElement::useViewSizeRequested() {
	if (m_worksheet->useViewSize()) {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		zoomFitPageHeightAction->setVisible(false);
		zoomFitPageWidthAction->setVisible(false);
		currentZoomAction = zoomInViewAction;
		if (tbZoom)
			tbZoom->setDefaultAction(zoomInViewAction);

		this->processResize();
	} else {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		zoomFitPageHeightAction->setVisible(true);
		zoomFitPageWidthAction->setVisible(true);
	}
}

void ImageReaderElement::processResize() {
	if (size() != sceneRect().size()) {
		static const float hscale = QApplication::desktop()->physicalDpiX()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		static const float vscale = QApplication::desktop()->physicalDpiY()/(25.4*Worksheet::convertToSceneUnits(1,Worksheet::Millimeter));
		m_worksheet->setUndoAware(false);
		m_worksheet->setPageRect(QRectF(0.0, 0.0, width()/hscale, height()/vscale));
		m_worksheet->setUndoAware(true);
	}
}

void ImageReaderElement::changeZoom(QAction* action){
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

void ImageReaderElement::mouseModeChanged(QAction* action) {
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

void ImageReaderElement::aspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* aspect) {
    Q_UNUSED(parent);
    Q_UNUSED(before);
    Image* image = 0;
    image = dynamic_cast<Image*>(const_cast<AbstractAspect*>(aspect));
    if (!image)
        return;
    handleImageActions();
}

void ImageReaderElement::addNewImage() {
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    QString fileName = QFileDialog::getOpenFileName(this,i18n("Open Images"), "~/");
    if (!fileName.isEmpty()){
        //clear worksheet
        QList<Image *> childElements = m_worksheet->children<Image>();
        foreach(Image *elem, childElements)
            elem->remove();
        Image* image = new Image(i18n("Image"),fileName);
        image->setImage();
        m_worksheet->addChild(image);
        handleImageActions();
    }
}

void ImageReaderElement::setImageReferencePoints() {
    foreach(Image* image, m_worksheet->children<Image>() ){
            image->setReferencePoints();
    }
}

void ImageReaderElement::setImageCurvePoints(){
    foreach(Image* image, m_worksheet->children<Image>() ){
            image->setCurvePoints();
    }
}

void ImageReaderElement::selectAllElements() {
	m_suppressSelectionChangedEvent = true;
	QList<QGraphicsItem*> items = scene()->selectedItems();
	foreach ( QGraphicsItem* item , m_selectedItems ){
		m_worksheet->setItemSelectedInView(item, false);
	}

	items = scene()->items();
	foreach(QGraphicsItem* item, items){
		if (!item->parentItem())
			item->setSelected(true);
	}
	m_suppressSelectionChangedEvent = false;
	this->selectionChanged();
}

void ImageReaderElement::deleteElement() {
	QList<QGraphicsItem*> items = scene()->selectedItems();
	if (items.size()==0)
		return;

	int rc = KMessageBox::warningYesNo( this,
			i18np("Do you really want to delete the selected object?", "Do you really want to delete the selected %1 objects?", items.size()),
			i18n("Delete selected objects"));

	if (rc==KMessageBox::No)
		return;

	m_suppressSelectionChangedEvent = true;
	m_worksheet->beginMacro(i18n("%1: Remove selected worksheet elements.", m_worksheet->name()));
	foreach ( QGraphicsItem* item , m_selectedItems ) {
		m_worksheet->deleteAspectFromGraphicsItem(item);
	}
	m_worksheet->endMacro();
	m_suppressSelectionChangedEvent = false;
}

void ImageReaderElement::selectItem(QGraphicsItem* item){
	m_suppressSelectionChangedEvent = true;
	item->setSelected(true);
	m_selectedItems<<item;
	m_suppressSelectionChangedEvent = false;
}

void ImageReaderElement::deselectItem(QGraphicsItem* item){
	m_suppressSelectionChangedEvent = true;
	item->setSelected(false);
	m_selectedItems.removeOne(item);
	m_suppressSelectionChangedEvent = false;
}

void ImageReaderElement::selectionChanged(){
	if (m_suppressSelectionChangedEvent)
		return;

	QList<QGraphicsItem*> items = scene()->selectedItems();

	bool invisibleDeselected = false;

	foreach ( QGraphicsItem* item, m_selectedItems ){
		if ( items.indexOf(item) == -1 ) {
			if (item->isVisible())
				m_worksheet->setItemSelectedInView(item, false);
			else
				invisibleDeselected = true;
		}
	}

	if (items.size() == 0 && invisibleDeselected == false){
		m_worksheet->setSelectedInView(true);
	}else{
		foreach (const QGraphicsItem* item, items)
			m_worksheet->setItemSelectedInView(item, true);

		m_worksheet->setSelectedInView(false);
	}

	m_selectedItems = items;
}

void ImageReaderElement::handleImageActions() {
    bool image = false;

    image = (m_worksheet->children<Image>().size() != 0);

    setReferencePointsAction->setEnabled(image);
    setCurvePointsAction->setEnabled(image);
}

void ImageReaderElement::exportToFile(const QString& path, const ExportFormat format,
								 const ExportArea area, const bool background, const int resolution) {
	QRectF sourceRect;

    if (area==ImageReaderElement::ExportBoundingBox){
		sourceRect = scene()->itemsBoundingRect();
    }else if (area==ImageReaderElement::ExportSelection){
		foreach(QGraphicsItem* item, m_selectedItems) {
			sourceRect = sourceRect.united( item->mapToScene(item->boundingRect()).boundingRect() );
		}
	}else{
		sourceRect = scene()->sceneRect();
	}

	//print
    if (format==ImageReaderElement::Pdf || format==ImageReaderElement::Eps){
		QPrinter printer(QPrinter::HighResolution);
        if (format==ImageReaderElement::Pdf)
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
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();
    }else if (format==ImageReaderElement::Svg){
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
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();
	}else{
		//PNG
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
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();

		image.save(path, "png");
	}
}

void ImageReaderElement::exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect, const bool background) {
	if (background) {
		painter->save();
		painter->scale(targetRect.width()/sourceRect.width(), targetRect.height()/sourceRect.height());
		drawBackground(painter, sourceRect);
		painter->restore();
	}

	m_worksheet->setPrinting(true);
	scene()->render(painter, QRectF(), sourceRect);
	m_worksheet->setPrinting(false);
}

void ImageReaderElement::print(QPrinter* printer) const{
	m_worksheet->setPrinting(true);
	QPainter painter(printer);
	painter.setRenderHint(QPainter::Antialiasing);
	scene()->render(&painter);
	m_worksheet->setPrinting(false);
}

void ImageReaderElement::updateBackground(){
	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}
