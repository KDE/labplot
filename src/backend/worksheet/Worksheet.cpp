/***************************************************************************
    File                 : Worksheet.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet (2D visualization) part
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
	Copyright            : (C) 2011-2012 by Alexander Semke (alexander.semke*web.de)
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

#include "Worksheet.h"
#include "WorksheetPrivate.h"
#include "AbstractWorksheetElement.h"
#include "../../commonfrontend/worksheet/WorksheetView.h"
#include "WorksheetGraphicsScene.h"
#include "../lib/commandtemplates.h"
#include "../lib/macros.h"

#include <QWidget>
#include <QDebug>
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#else
#include "KIcon"
#include <KConfig>
#include <KConfigGroup>
#endif
	
/**
 * \class Worksheet
 * \brief The plotting part. 
 *
 * Top-level container for WorksheetElements. 
 *
 * * \ingroup worksheet
 */
	

Worksheet::Worksheet(AbstractScriptingEngine *engine, const QString &name)
		: AbstractPart(name), scripted(engine), d(new WorksheetPrivate(this)){

	connect(this, SIGNAL(aspectAdded(const AbstractAspect*)),
		this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(this, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
		this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	init();
}

Worksheet::~Worksheet() {
	delete d;
}

void Worksheet::init() {
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	KConfig config;
	KConfigGroup group = config.group( "Worksheet" );
	d->backgroundType = (PlotArea::BackgroundType) group.readEntry("BackgroundType", (int) PlotArea::Color);
	d->backgroundColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", (int) PlotArea::SingleColor);
	d->backgroundImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", (int) PlotArea::Scaled);
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());
	d->backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	d->backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	d->backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);
	
	d->layout = (Worksheet::Layout) group.readEntry("Layout", (int) Worksheet::NoLayout);
	d->layoutTopMargin =  group.readEntry("LayoutTopMargin", convertToSceneUnits(1, Centimeter));
	d->layoutBottomMargin = group.readEntry("LayoutBottomMargin", convertToSceneUnits(1, Centimeter));
	d->layoutLeftMargin = group.readEntry("LayoutLeftMargin", convertToSceneUnits(1, Centimeter));
	d->layoutRightMargin = group.readEntry("LayoutRightMargin", convertToSceneUnits(1, Centimeter));
	d->layoutVerticalSpacing = group.readEntry("LayoutVerticalSpacing", convertToSceneUnits(1, Centimeter));
	d->layoutHorizontalSpacing = group.readEntry("LayoutHorizontalSpacing", convertToSceneUnits(1, Centimeter));
	d->layoutRowCount = group.readEntry("LayoutRowCount", 2);
	d->layoutColumnCount = group.readEntry("LayoutColumnCount", 2);
#endif
}

/*!
	converts from \c unit to the scene units. At the moment, 1 scene unit corresponds to 1/10 mm.
 */
float Worksheet::convertToSceneUnits(const float value, const Worksheet::Unit unit){
	switch (unit){
    case Worksheet::Millimeter:
		return value*10.0;
	case Worksheet::Centimeter:
		return value*100.0;
	case Worksheet::Inch:
		return value*25.4*10.;
	case Worksheet::Point:
		return value*25.4/72.*10.;
	}
	return value;
}

/*!
	converts from the scene units to \c unit . At the moment, 1 scene unit corresponds to 1/10 mm.
 */
float Worksheet::convertFromSceneUnits(const float value, const Worksheet::Unit unit){
	switch (unit){
    case Worksheet::Millimeter:
		return value/10.0;
	case Worksheet::Centimeter:
		return value/100.0;
	case Worksheet::Inch:
		return value/25.4/10.;
	case Worksheet::Point:
		return value/25.4/10.*72.;
	}
	return value;
}

//! Return an icon to be used for decorating my views.
QIcon Worksheet::icon() const {
	QIcon ico;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	ico.addPixmap(QPixmap(":/graph.xpm"));
#else
	ico = KIcon("office-chart-area");
#endif
	return ico;
}

//! Fill the part specific menu for the main window including setting the title
/**
 * \return true on success, otherwise false (e.g. part has no actions).
 */
bool Worksheet::fillProjectMenu(QMenu * menu) {
	Q_UNUSED(menu);
	// TODO
	return false;
}

//! Return a new context menu.
/**
 * The caller takes ownership of the menu.
 */
QMenu *Worksheet::createContextMenu() {
	QMenu *menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	emit requestProjectContextMenu(menu);
	return menu;
}

//! Construct a primary view on me.
/**
 * This method may be called multiple times during the life time of an Aspect, or it might not get
 * called at all. Aspects must not depend on the existence of a view for their operation.
 */
QWidget *Worksheet::view() const {
	if (!d->m_view) {
		d->m_view = new WorksheetView(const_cast<Worksheet *>(this));
		connect(d->m_view, SIGNAL(statusInfo(const QString&)), this, SIGNAL(statusInfo(const QString&)));
	}
	return d->m_view;
}

//! Save as XML
void Worksheet::save(QXmlStreamWriter *) const {
	// TODO
}

//! Load from XML
bool Worksheet::load(XmlStreamReader *) {
	// TODO
	return false;
}

void Worksheet::handleAspectAdded(const AbstractAspect *aspect) {
	const AbstractWorksheetElement *addedElement = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (addedElement) {
		const_cast<AbstractWorksheetElement *>(addedElement)->retransform();

		if (aspect->parentAspect() == this){
			QGraphicsItem *item = addedElement->graphicsItem();
			Q_ASSERT(item != NULL);
			d->m_scene->addItem(item);

			qreal zVal = 0;
			QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
			foreach(AbstractWorksheetElement *elem, childElements) {
				elem->graphicsItem()->setZValue(zVal++);
			}
		}
	}
	if (d->layout != Worksheet::NoLayout)
		d->updateLayout();
}

void Worksheet::handleAspectAboutToBeRemoved(const AbstractAspect *aspect) {
	const AbstractWorksheetElement *removedElement = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (removedElement) {
		QGraphicsItem *item = removedElement->graphicsItem();
		Q_ASSERT(item != NULL);
		d->m_scene->removeItem(item);
	}
	if (d->layout != Worksheet::NoLayout)
		d->updateLayout();
}

WorksheetGraphicsScene *Worksheet::scene() const {
	return d->m_scene;
}

QRectF Worksheet::pageRect() const {
	return d->m_scene->sceneRect();
}

/*!
	this slot is called when a worksheet element is selected in the project explorer.
	emits \c itemSelected() which forwards this event to the \c WorksheetView 
	in order to select the corresponding \c QGraphicsItem.
 */
void Worksheet::childSelected(){
	AbstractWorksheetElement* element=qobject_cast<AbstractWorksheetElement*>(QObject::sender());
	if (element)
		emit itemSelected(element->graphicsItem());
}

/*!
	this slot is called when a worksheet element is deselected in the project explorer.
	emits \c itemDeselected() which forwards this event to \c WorksheetView 
	in order to deselect the corresponding \c QGraphicsItem.
 */
void Worksheet::childDeselected(){
	AbstractWorksheetElement* element=qobject_cast<AbstractWorksheetElement*>(QObject::sender());
	if (element)
		emit itemDeselected(element->graphicsItem());
}


/*!
 *  Emits the signal to select or to deselect the aspect corresponding to \c QGraphicsItem \c item in the project explorer, 
 *  if \c selected=true or \c selected=false, respectively.
 *  The signal is handled in \c AspectTreeModel and forwarded to the tree view in \c ProjectExplorer.
 * This function is called in \c WorksheetView upon selection changes.
 */
void Worksheet::setItemSelectedInView(const QGraphicsItem* item, const bool b){
	//determine the corresponding aspect
	const AbstractAspect* aspect = 0;
	foreach( const AbstractWorksheetElement* child, children<AbstractWorksheetElement>() ){
		aspect = this->aspectFromGraphicsItem(child, item);
		if (aspect)
			break;
	}

	//no aspect were found.
	//TODO
	if (!aspect){
		qDebug() << "not an  aspect selected";
		return;
	}

	if (b){
		emit childAspectSelectedInView(aspect);
		//deselect the worksheet in the project explorer, if a child was selected.
		//prevents unwanted multiple selection with worksheet (if it was selected before).
		emit childAspectDeselectedInView(this);
	}else{
		emit childAspectDeselectedInView(aspect);
	}
}

/*!
 * helper function:  checks whether \c aspect or one of its children has the \c GraphicsItem \c item
 * Returns a pointer to \c AbstractWorksheetElement having this item.
 */
AbstractWorksheetElement* Worksheet::aspectFromGraphicsItem(const AbstractWorksheetElement* aspect, const QGraphicsItem* item) const{
	if ( aspect->graphicsItem() == item ){
		return const_cast<AbstractWorksheetElement*>(aspect);
	}else{
		AbstractWorksheetElement* a = 0;
		foreach( const AbstractWorksheetElement* child, aspect->children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden) ){
			a = this->aspectFromGraphicsItem(child, item);
			if (a)
				return a;
		}
		return 0;
	}
}

void Worksheet::update(){
	emit requestUpdate();
}

/* =============================== getter methods for background options ================================= */
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundType, backgroundType, backgroundType);
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle);
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle);
CLASS_D_READER_IMPL(Worksheet, QBrush, backgroundBrush, backgroundBrush);
CLASS_D_READER_IMPL(Worksheet, QColor, backgroundFirstColor, backgroundFirstColor);
CLASS_D_READER_IMPL(Worksheet, QColor, backgroundSecondColor, backgroundSecondColor);
CLASS_D_READER_IMPL(Worksheet, QString, backgroundFileName, backgroundFileName);
BASIC_D_READER_IMPL(Worksheet, qreal, backgroundOpacity, backgroundOpacity);

/* =============================== getter methods for layout options ====================================== */
BASIC_D_READER_IMPL(Worksheet, Worksheet::Layout, layout, layout);
BASIC_D_READER_IMPL(Worksheet, float, layoutTopMargin, layoutTopMargin);
BASIC_D_READER_IMPL(Worksheet, float, layoutBottomMargin, layoutBottomMargin);
BASIC_D_READER_IMPL(Worksheet, float, layoutLeftMargin, layoutLeftMargin);
BASIC_D_READER_IMPL(Worksheet, float, layoutRightMargin, layoutRightMargin);
BASIC_D_READER_IMPL(Worksheet, float, layoutHorizontalSpacing, layoutHorizontalSpacing);
BASIC_D_READER_IMPL(Worksheet, float, layoutVerticalSpacing, layoutVerticalSpacing);
BASIC_D_READER_IMPL(Worksheet, int, layoutRowCount, layoutRowCount);
BASIC_D_READER_IMPL(Worksheet, int, layoutColumnCount, layoutColumnCount);


/* ============================ setter methods and undo commands  for background options  ================= */
STD_SETTER_CMD_IMPL_F(Worksheet, SetBackgroundType, PlotArea::BackgroundType, backgroundType, update);
void Worksheet::setBackgroundType(PlotArea::BackgroundType type) {
	if (type != d->backgroundType)
		exec(new WorksheetSetBackgroundTypeCmd(d, type, tr("%1: background type changed")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetBackgroundColorStyle, PlotArea::BackgroundColorStyle, backgroundColorStyle, update);
void Worksheet::setBackgroundColorStyle(PlotArea::BackgroundColorStyle style) {
	if (style != d->backgroundColorStyle)
		exec(new WorksheetSetBackgroundColorStyleCmd(d, style, tr("%1: background color style changed")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetBackgroundImageStyle, PlotArea::BackgroundImageStyle, backgroundImageStyle, update);
void Worksheet::setBackgroundImageStyle(PlotArea::BackgroundImageStyle style) {
	if (style != d->backgroundImageStyle)
		exec(new WorksheetSetBackgroundImageStyleCmd(d, style, tr("%1: background image style changed")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetBackgroundFirstColor, QColor, backgroundFirstColor, update);
void Worksheet::setBackgroundFirstColor(const QColor &color) {
	if (color!= d->backgroundFirstColor)
		exec(new WorksheetSetBackgroundFirstColorCmd(d, color, tr("%1: set background first color")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetBackgroundSecondColor, QColor, backgroundSecondColor, update);
void Worksheet::setBackgroundSecondColor(const QColor &color) {
	if (color!= d->backgroundSecondColor)
		exec(new WorksheetSetBackgroundSecondColorCmd(d, color, tr("%1: set background second color")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetBackgroundFileName, QString, backgroundFileName, update);
void Worksheet::setBackgroundFileName(const QString& fileName) {
	if (fileName!= d->backgroundFileName)
		exec(new WorksheetSetBackgroundFileNameCmd(d, fileName, tr("%1: set background image")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetBackgroundOpacity, qreal, backgroundOpacity, update);
void Worksheet::setBackgroundOpacity(qreal opacity) {
	if (opacity != d->backgroundOpacity)
		exec(new WorksheetSetBackgroundOpacityCmd(d, opacity, tr("%1: set opacity")));
}

/* ============================ setter methods and undo commands  for layout options  ================= */
//TODO make this undo/redo-aware
void Worksheet::setLayout(Worksheet::Layout layout){
	if (layout != d->layout){
		d->layout = layout;
		d->updateLayout();
	}
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetLayoutTopMargin, float, layoutTopMargin, updateLayout);
void Worksheet::setLayoutTopMargin(float margin){
	if (margin != d->layoutTopMargin)
		exec(new WorksheetSetLayoutTopMarginCmd(d, margin, tr("%1: set layout top margin")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetLayoutBottomMargin, float, layoutBottomMargin, updateLayout);
void Worksheet::setLayoutBottomMargin(float margin){
	if (margin != d->layoutBottomMargin)
		exec(new WorksheetSetLayoutBottomMarginCmd(d, margin, tr("%1: set layout bottom margin")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetLayoutLeftMargin, float, layoutLeftMargin, updateLayout);
void Worksheet::setLayoutLeftMargin(float margin){
	if (margin != d->layoutLeftMargin)
		exec(new WorksheetSetLayoutLeftMarginCmd(d, margin, tr("%1: set layout left margin")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetLayoutRightMargin, float, layoutRightMargin, updateLayout);
void Worksheet::setLayoutRightMargin(float margin){
	if (margin != d->layoutRightMargin)
		exec(new WorksheetSetLayoutRightMarginCmd(d, margin, tr("%1: set layout right margin")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetLayoutVerticalSpacing, float, layoutVerticalSpacing, updateLayout);
void Worksheet::setLayoutVerticalSpacing(float spacing){
	if (spacing != d->layoutVerticalSpacing)
		exec(new WorksheetSetLayoutVerticalSpacingCmd(d, spacing, tr("%1: set layout vertical spacing")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetLayoutHorizontalSpacing, float, layoutHorizontalSpacing, updateLayout);
void Worksheet::setLayoutHorizontalSpacing(float spacing){
	if (spacing != d->layoutHorizontalSpacing)
		exec(new WorksheetSetLayoutHorizontalSpacingCmd(d, spacing, tr("%1: set layout horizontal spacing")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetLayoutRowCount, int, layoutRowCount, updateLayout);
void Worksheet::setLayoutRowCount(int count){
	if (count != d->layoutRowCount)
		exec(new WorksheetSetLayoutRowCountCmd(d, count, tr("%1: set layout row count")));
}

STD_SETTER_CMD_IMPL_F(Worksheet, SetLayoutColumnCount, int, layoutColumnCount, updateLayout);
void Worksheet::setLayoutColumnCount(int count){
	if (count != d->layoutColumnCount)
		exec(new WorksheetSetLayoutColumnCountCmd(d, count, tr("%1: set layout column count")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(Worksheet, SetPageRect, QRectF, swapPageRect);
void Worksheet::setPageRect(const QRectF &rect, bool scaleContent){
	if (qFuzzyCompare(rect.width() + 1, 1) || qFuzzyCompare(rect.height() + 1, 1))
		return;

	if (rect != d->m_scene->sceneRect()) {
		QString title = tr("%1: set page size");
		QRectF oldRect = d->m_scene->sceneRect();
		beginMacro(title.arg(name()));
		exec(new WorksheetSetPageRectCmd(d, rect, title));

		qreal horizontalRatio = rect.width() / oldRect.normalized().width();
		qreal verticalRatio = rect.height() / oldRect.normalized().height();

		QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
		if (scaleContent){
			foreach(AbstractWorksheetElement *elem, childElements) {
				elem->handlePageResize(horizontalRatio, verticalRatio);
			}
		}
		endMacro();
	}
}

//################################################################
//################### Private implementation ##########################
//################################################################
WorksheetPrivate::WorksheetPrivate(Worksheet *owner):q(owner), m_view(NULL){
	m_scene = new WorksheetGraphicsScene();
	m_scene->setSceneRect(0, 0, 1500, 1500);
}

QString WorksheetPrivate::name() const{
	return q->name();
}

QRectF WorksheetPrivate::swapPageRect(const QRectF &rect) {
	QRectF oldRect = m_scene->sceneRect();
	m_scene->setSceneRect(rect.normalized());

	return oldRect;
}

void WorksheetPrivate::update(){
	q->update();
}

WorksheetPrivate::~WorksheetPrivate(){
	delete m_scene;
}

void WorksheetPrivate::updateLayout(){
	QList<WorksheetElementContainer*> list = q->children<WorksheetElementContainer>();
	if (layout==Worksheet::NoLayout){
		foreach(WorksheetElementContainer* elem, q->children<WorksheetElementContainer>()){
			elem->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
		}
		return;
	}

	float x=layoutLeftMargin;
	float y=layoutTopMargin;
	float w, h;
	int count=list.count();
	if (layout == Worksheet::VerticalLayout){
		w= m_scene->sceneRect().width() - layoutLeftMargin - layoutRightMargin;
		h=(m_scene->sceneRect().height()-layoutTopMargin-layoutBottomMargin- (count-1)*layoutVerticalSpacing)/count;
		foreach(WorksheetElementContainer* elem, list){
			elem->setRect(QRectF(x,y,w,h));
			elem->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
			y+=h + layoutVerticalSpacing;
		}
	}else if (layout == Worksheet::HorizontalLayout){
		w=(m_scene->sceneRect().height()-layoutLeftMargin-layoutRightMargin- (count-1)*layoutHorizontalSpacing)/count;
		h= m_scene->sceneRect().height() - layoutTopMargin-layoutBottomMargin;
		foreach(WorksheetElementContainer* elem, list){
			elem->setRect(QRectF(x,y,w,h));
			elem->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
			x+=w + layoutHorizontalSpacing;
		}		
	}else{ //GridLayout
		//add new rows, if not sufficient
		if (count>layoutRowCount*layoutColumnCount)
			layoutRowCount = count/layoutColumnCount;
		
		w=(m_scene->sceneRect().height()-layoutLeftMargin-layoutRightMargin- (layoutColumnCount-1)*layoutHorizontalSpacing)/layoutColumnCount;
		h=(m_scene->sceneRect().height()-layoutTopMargin-layoutBottomMargin- (layoutRowCount-1)*layoutVerticalSpacing)/layoutRowCount;
		int columnIndex=0; //counts the columns in a row
		foreach(WorksheetElementContainer* elem, list){
			elem->setRect(QRectF(x,y,w,h));
			elem->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
			x+=w + layoutHorizontalSpacing;
			columnIndex++;
			if (columnIndex==layoutColumnCount){
				columnIndex=0;
				x=layoutLeftMargin;
				y+=h + layoutVerticalSpacing;
			}
		}
	}
	q->update();
}
