/***************************************************************************
    File                 : Worksheet.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
	Copyright            : (C) 2011-2013 by Alexander Semke (alexander.semke*web.de)
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
#include "commonfrontend/worksheet/WorksheetView.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"
#include <math.h>

#include <QGraphicsScene>
#include <QWidget>
#include <QDebug>
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#else
#include "KIcon"
#include <KConfig>
#include <KConfigGroup>
#endif
#include <KLocale>

	
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
	connect(this, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
		this, SLOT(handleAspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)) );
	init();
}

Worksheet::~Worksheet() {
	delete d;
}

void Worksheet::init() {
	KConfig config;
	KConfigGroup group = config.group( "Worksheet" );
	d->backgroundType = (PlotArea::BackgroundType) group.readEntry("BackgroundType", (int) PlotArea::Color);
	d->backgroundColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", (int) PlotArea::SingleColor);
	d->backgroundImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", (int) PlotArea::Scaled);
	d->backgroundBrushStyle = (Qt::BrushStyle) group.readEntry("BackgroundBrushStyle", (int) Qt::SolidPattern);
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());
	d->backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	d->backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	d->backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);
	
	d->layout = (Worksheet::Layout) group.readEntry("Layout", (int) Worksheet::VerticalLayout);
	d->layoutTopMargin =  group.readEntry("LayoutTopMargin", convertToSceneUnits(1, Centimeter));
	d->layoutBottomMargin = group.readEntry("LayoutBottomMargin", convertToSceneUnits(1, Centimeter));
	d->layoutLeftMargin = group.readEntry("LayoutLeftMargin", convertToSceneUnits(1, Centimeter));
	d->layoutRightMargin = group.readEntry("LayoutRightMargin", convertToSceneUnits(1, Centimeter));
	d->layoutVerticalSpacing = group.readEntry("LayoutVerticalSpacing", convertToSceneUnits(1, Centimeter));
	d->layoutHorizontalSpacing = group.readEntry("LayoutHorizontalSpacing", convertToSceneUnits(1, Centimeter));
	d->layoutRowCount = group.readEntry("LayoutRowCount", 2);
	d->layoutColumnCount = group.readEntry("LayoutColumnCount", 2);
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
	if (!m_view) {
		m_view = new WorksheetView(const_cast<Worksheet *>(this));
		connect(m_view, SIGNAL(statusInfo(QString)), this, SIGNAL(statusInfo(QString)));
	}
	return m_view;
}

void Worksheet::handleAspectAdded(const AbstractAspect* aspect) {
	qDebug()<<"Worksheet::handleAspectAdded "<< aspect->name();
	const AbstractWorksheetElement* addedElement = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (addedElement) {
		const_cast<AbstractWorksheetElement*>(addedElement)->retransform();

		if (aspect->parentAspect() == this){
			QGraphicsItem *item = addedElement->graphicsItem();
			Q_ASSERT(item != NULL);
			d->m_scene->addItem(item);

			qreal zVal = 0;
			QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
			foreach(AbstractWorksheetElement *elem, childElements) {
				elem->graphicsItem()->setZValue(zVal++);
			}

			if (d->layout != Worksheet::NoLayout)
				d->updateLayout();
		}
	}
}

void Worksheet::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	qDebug()<<"Worksheet::handleAspectAboutToBeRemoved " << aspect->name();
	const AbstractWorksheetElement *removedElement = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (removedElement) {
		QGraphicsItem *item = removedElement->graphicsItem();
		Q_ASSERT(item != NULL);
		d->m_scene->removeItem(item);
	}
}

void Worksheet::handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child){
	Q_UNUSED(parent);
	Q_UNUSED(before);
	Q_UNUSED(child);

	if (d->layout != Worksheet::NoLayout)
		d->updateLayout();	
}

QGraphicsScene *Worksheet::scene() const {
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
void Worksheet::childSelected(const AbstractAspect* aspect){
	AbstractWorksheetElement* element=qobject_cast<AbstractWorksheetElement*>(const_cast<AbstractAspect*>(aspect));
	if (element)
		emit itemSelected(element->graphicsItem());
}

/*!
	this slot is called when a worksheet element is deselected in the project explorer.
	emits \c itemDeselected() which forwards this event to \c WorksheetView 
	in order to deselect the corresponding \c QGraphicsItem.
 */
void Worksheet::childDeselected(const AbstractAspect* aspect){
	AbstractWorksheetElement* element=qobject_cast<AbstractWorksheetElement*>(const_cast<AbstractAspect*>(aspect));
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
	foreach( const AbstractWorksheetElement* child, children<AbstractWorksheetElement>(IncludeHidden) ){
		aspect = this->aspectFromGraphicsItem(child, item);
		if (aspect)
			break;
	}

	if (!aspect)
		return;

	//forward selection/deselection to AbstractTreeModel
	if (b)
		emit childAspectSelectedInView(aspect);
	else
		emit childAspectDeselectedInView(aspect);
}

/*!
 * helper function:  checks whether \c aspect or one of its children has the \c GraphicsItem \c item
 * Returns a pointer to \c AbstractWorksheetElement having this item.
 */
AbstractWorksheetElement* Worksheet::aspectFromGraphicsItem(const AbstractWorksheetElement* aspect, const QGraphicsItem* item) const{
	if ( aspect->graphicsItem() == item ){
		return const_cast<AbstractWorksheetElement*>(aspect);
	}else{
		foreach( const AbstractWorksheetElement* child, aspect->children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden) ){
			AbstractWorksheetElement* a = this->aspectFromGraphicsItem(child, item);
			if (a)
				return a;
		}
		return 0;
	}
}

/*!
	Selects or deselects the worksheet in the project explorer.
	This function is called in \c WorksheetView.
	The worksheet gets deselected if there are selected items in the view,
	and selected if there are no selected items in the view.
*/
void Worksheet::setSelectedInView(const bool b){
	if (b)
		emit childAspectSelectedInView(this);
	else
		emit childAspectDeselectedInView(this);
}

void Worksheet::deleteAspectFromGraphicsItem(const QGraphicsItem* item){
	Q_ASSERT(item);
	//determine the corresponding aspect
	AbstractAspect* aspect = 0;
	foreach( const AbstractWorksheetElement* child, children<AbstractWorksheetElement>(IncludeHidden) ){
		aspect = this->aspectFromGraphicsItem(child, item);
		if (aspect)
			break;
	}

	if (!aspect)
		return;

	if (aspect->parentAspect())
		aspect->parentAspect()->removeChild(aspect);
	else
		this->removeChild(aspect);
}

void Worksheet::update(){
	emit requestUpdate();
}

/* =============================== getter methods for background options ================================= */
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundType, backgroundType, backgroundType)
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle)
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle)
BASIC_D_READER_IMPL(Worksheet, Qt::BrushStyle, backgroundBrushStyle, backgroundBrushStyle)
CLASS_D_READER_IMPL(Worksheet, QColor, backgroundFirstColor, backgroundFirstColor)
CLASS_D_READER_IMPL(Worksheet, QColor, backgroundSecondColor, backgroundSecondColor)
CLASS_D_READER_IMPL(Worksheet, QString, backgroundFileName, backgroundFileName)
BASIC_D_READER_IMPL(Worksheet, float, backgroundOpacity, backgroundOpacity)

/* =============================== getter methods for layout options ====================================== */
BASIC_D_READER_IMPL(Worksheet, Worksheet::Layout, layout, layout)
BASIC_D_READER_IMPL(Worksheet, float, layoutTopMargin, layoutTopMargin)
BASIC_D_READER_IMPL(Worksheet, float, layoutBottomMargin, layoutBottomMargin)
BASIC_D_READER_IMPL(Worksheet, float, layoutLeftMargin, layoutLeftMargin)
BASIC_D_READER_IMPL(Worksheet, float, layoutRightMargin, layoutRightMargin)
BASIC_D_READER_IMPL(Worksheet, float, layoutHorizontalSpacing, layoutHorizontalSpacing)
BASIC_D_READER_IMPL(Worksheet, float, layoutVerticalSpacing, layoutVerticalSpacing)
BASIC_D_READER_IMPL(Worksheet, int, layoutRowCount, layoutRowCount)
BASIC_D_READER_IMPL(Worksheet, int, layoutColumnCount, layoutColumnCount)


/* ============================ setter methods and undo commands  for background options  ================= */
STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundType, PlotArea::BackgroundType, backgroundType, update)
void Worksheet::setBackgroundType(PlotArea::BackgroundType type) {
	if (type != d->backgroundType)
		exec(new WorksheetSetBackgroundTypeCmd(d, type, i18n("%1: background type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundColorStyle, PlotArea::BackgroundColorStyle, backgroundColorStyle, update)
void Worksheet::setBackgroundColorStyle(PlotArea::BackgroundColorStyle style) {
	if (style != d->backgroundColorStyle)
		exec(new WorksheetSetBackgroundColorStyleCmd(d, style, i18n("%1: background color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundImageStyle, PlotArea::BackgroundImageStyle, backgroundImageStyle, update)
void Worksheet::setBackgroundImageStyle(PlotArea::BackgroundImageStyle style) {
	if (style != d->backgroundImageStyle)
		exec(new WorksheetSetBackgroundImageStyleCmd(d, style, i18n("%1: background image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundBrushStyle, Qt::BrushStyle, backgroundBrushStyle, update)
void Worksheet::setBackgroundBrushStyle(Qt::BrushStyle style) {
	if (style != d->backgroundBrushStyle)
		exec(new WorksheetSetBackgroundBrushStyleCmd(d, style, i18n("%1: background brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundFirstColor, QColor, backgroundFirstColor, update)
void Worksheet::setBackgroundFirstColor(const QColor &color) {
	if (color!= d->backgroundFirstColor)
		exec(new WorksheetSetBackgroundFirstColorCmd(d, color, i18n("%1: set background first color")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundSecondColor, QColor, backgroundSecondColor, update)
void Worksheet::setBackgroundSecondColor(const QColor &color) {
	if (color!= d->backgroundSecondColor)
		exec(new WorksheetSetBackgroundSecondColorCmd(d, color, i18n("%1: set background second color")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundFileName, QString, backgroundFileName, update)
void Worksheet::setBackgroundFileName(const QString& fileName) {
	if (fileName!= d->backgroundFileName)
		exec(new WorksheetSetBackgroundFileNameCmd(d, fileName, i18n("%1: set background image")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundOpacity, float, backgroundOpacity, update)
void Worksheet::setBackgroundOpacity(float opacity) {
	if (opacity != d->backgroundOpacity)
		exec(new WorksheetSetBackgroundOpacityCmd(d, opacity, i18n("%1: set opacity")));
}

/* ============================ setter methods and undo commands  for layout options  ================= */
STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayout, Worksheet::Layout, layout, updateLayout)
void Worksheet::setLayout(Worksheet::Layout layout){
	if (layout != d->layout) {
		beginMacro(i18n("%1: set layout").arg(name()));
		exec(new WorksheetSetLayoutCmd(d, layout, i18n("%1: set layout")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutTopMargin, float, layoutTopMargin, updateLayout)
void Worksheet::setLayoutTopMargin(float margin){
	if (margin != d->layoutTopMargin) {
		beginMacro(i18n("%1: set layout top margin").arg(name()));
		exec(new WorksheetSetLayoutTopMarginCmd(d, margin, i18n("%1: set layout top margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutBottomMargin, float, layoutBottomMargin, updateLayout)
void Worksheet::setLayoutBottomMargin(float margin){
	if (margin != d->layoutBottomMargin) {
		beginMacro(i18n("%1: set layout bottom margin").arg(name()));
		exec(new WorksheetSetLayoutBottomMarginCmd(d, margin, i18n("%1: set layout bottom margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutLeftMargin, float, layoutLeftMargin, updateLayout)
void Worksheet::setLayoutLeftMargin(float margin){
	if (margin != d->layoutLeftMargin) {
		beginMacro(i18n("%1: set layout left margin").arg(name()));
		exec(new WorksheetSetLayoutLeftMarginCmd(d, margin, i18n("%1: set layout left margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutRightMargin, float, layoutRightMargin, updateLayout)
void Worksheet::setLayoutRightMargin(float margin){
	if (margin != d->layoutRightMargin) {
		beginMacro(i18n("%1: set layout right margin").arg(name()));
		exec(new WorksheetSetLayoutRightMarginCmd(d, margin, i18n("%1: set layout right margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutVerticalSpacing, float, layoutVerticalSpacing, updateLayout)
void Worksheet::setLayoutVerticalSpacing(float spacing){
	if (spacing != d->layoutVerticalSpacing) {
		beginMacro(i18n("%1: set layout vertical spacing").arg(name()));
		exec(new WorksheetSetLayoutVerticalSpacingCmd(d, spacing, i18n("%1: set layout vertical spacing")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutHorizontalSpacing, float, layoutHorizontalSpacing, updateLayout)
void Worksheet::setLayoutHorizontalSpacing(float spacing){
	if (spacing != d->layoutHorizontalSpacing) {
		beginMacro(i18n("%1: set layout horizontal spacing").arg(name()));
		exec(new WorksheetSetLayoutHorizontalSpacingCmd(d, spacing, i18n("%1: set layout horizontal spacing")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutRowCount, int, layoutRowCount, updateLayout)
void Worksheet::setLayoutRowCount(int count){
	if (count != d->layoutRowCount) {
		beginMacro(i18n("%1: set layout row count").arg(name()));
		exec(new WorksheetSetLayoutRowCountCmd(d, count, i18n("%1: set layout row count")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutColumnCount, int, layoutColumnCount, updateLayout)
void Worksheet::setLayoutColumnCount(int count){
	if (count != d->layoutColumnCount) {
		beginMacro(i18n("%1: set layout column count").arg(name()));
		exec(new WorksheetSetLayoutColumnCountCmd(d, count, i18n("%1: set layout column count")));
		endMacro();
	}
}

class WorksheetSetPageRectCmd : public StandardMacroSetterCmd<Worksheet::Private, QRectF> {
	public:
		WorksheetSetPageRectCmd(Worksheet::Private* target, Loki::TypeTraits<QRectF>::ParameterType newValue, const QString& description)
		: StandardMacroSetterCmd<Worksheet::Private, QRectF>(target, &Worksheet::Private::pageRect, newValue, description) {}
	virtual void finalize() {
		m_target->updatePageRect();
		m_target->q->pageRectChanged(m_target->*m_field);
	}
	virtual void finalizeUndo() {
		m_target->m_scene->setSceneRect(m_target->*m_field);
		m_target->q->pageRectChanged(m_target->*m_field);
	}
}; 

void Worksheet::setPageRect(const QRectF& rect, bool scaleContent) {
	//don't allow any rectangulars of width/height equal to zero
	if (qFuzzyCompare(rect.width(), 0.) || qFuzzyCompare(rect.height(), 0.)){
		pageRectChanged(d->pageRect);
		return;
	}

	if (rect != d->pageRect) {
		d->scaleContent = scaleContent;
		beginMacro(i18n("%1: set page size").arg(name()));
		exec(new WorksheetSetPageRectCmd(d, rect, i18n("%1: set page size")));
		endMacro();
	}
}

void Worksheet::setPrinting(bool on) const {
	QList<AbstractWorksheetElement*> childElements = children<AbstractWorksheetElement>(AbstractAspect::Recursive | AbstractAspect::IncludeHidden);
	foreach(AbstractWorksheetElement* elem, childElements)
		elem->setPrinting(on);
}


//##############################################################################
//######################  Private implementation ###############################
//##############################################################################
WorksheetPrivate::WorksheetPrivate(Worksheet *owner):q(owner),
	pageRect(0, 0, 1500, 1500),
	m_scene(new QGraphicsScene(pageRect)),
	scaleContent(false) {
}

QString WorksheetPrivate::name() const{
	return q->name();
}

void WorksheetPrivate::updatePageRect() {
	QRectF oldRect = m_scene->sceneRect();
	m_scene->setSceneRect(pageRect);

	if (scaleContent) {
		qreal horizontalRatio = pageRect.width() / oldRect.width();
		qreal verticalRatio = pageRect.height() / oldRect.height();				
		QList<AbstractWorksheetElement*> childElements = q->children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden);
		foreach(AbstractWorksheetElement* elem, childElements)
			elem->handlePageResize(horizontalRatio, verticalRatio);
	}
	
	if (layout != Worksheet::NoLayout)
		updateLayout();
}

void WorksheetPrivate::update(){
	q->update();
}

WorksheetPrivate::~WorksheetPrivate(){
	delete m_scene;
}

void WorksheetPrivate::updateLayout(){
	emit q->layoutChanged(layout);
	QList<WorksheetElementContainer*> list = q->children<WorksheetElementContainer>();
	if (layout==Worksheet::NoLayout){
		foreach(WorksheetElementContainer* elem, list) {
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
		w=(m_scene->sceneRect().width()-layoutLeftMargin-layoutRightMargin- (count-1)*layoutHorizontalSpacing)/count;
		h= m_scene->sceneRect().height() - layoutTopMargin-layoutBottomMargin;
		foreach(WorksheetElementContainer* elem, list){
			elem->setRect(QRectF(x,y,w,h));
			elem->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
			x+=w + layoutHorizontalSpacing;
		}
	}else{ //GridLayout
		//add new rows, if not sufficient
		if (count>layoutRowCount*layoutColumnCount){
			layoutRowCount = floor( (float)count/layoutColumnCount + 0.5);
			emit q->layoutRowCountChanged(layoutRowCount);
		}
		
		w=(m_scene->sceneRect().width()-layoutLeftMargin-layoutRightMargin- (layoutColumnCount-1)*layoutHorizontalSpacing)/layoutColumnCount;
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


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void Worksheet::save(QXmlStreamWriter* writer) const{
    writer->writeStartElement( "worksheet" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);

    //geometry
    writer->writeStartElement( "geometry" );
    QRectF rect = d->m_scene->sceneRect();
    writer->writeAttribute( "x", QString::number(rect.x()) );
    writer->writeAttribute( "y", QString::number(rect.y()) );
    writer->writeAttribute( "width", QString::number(rect.width()) );
    writer->writeAttribute( "height", QString::number(rect.height()) );
    writer->writeEndElement();

    //layout
    writer->writeStartElement( "layout" );
    writer->writeAttribute( "layout", QString::number(d->layout) );
    writer->writeAttribute( "topMargin", QString::number(d->layoutTopMargin) );
    writer->writeAttribute( "bottomMargin", QString::number(d->layoutBottomMargin) );
    writer->writeAttribute( "leftMargin", QString::number(d->layoutLeftMargin) );
    writer->writeAttribute( "rightMargin", QString::number(d->layoutRightMargin) );
    writer->writeAttribute( "verticalSpacing", QString::number(d->layoutVerticalSpacing) );
    writer->writeAttribute( "horizontalSpacing", QString::number(d->layoutHorizontalSpacing) );
    writer->writeAttribute( "columnCount", QString::number(d->layoutColumnCount) );
    writer->writeAttribute( "rowCount", QString::number(d->layoutRowCount) );
    writer->writeEndElement();

    //background properties
    writer->writeStartElement( "background" );
    writer->writeAttribute( "type", QString::number(d->backgroundType) );
    writer->writeAttribute( "colorStyle", QString::number(d->backgroundColorStyle) );
    writer->writeAttribute( "imageStyle", QString::number(d->backgroundImageStyle) );
    writer->writeAttribute( "brushStyle", QString::number(d->backgroundBrushStyle) );
    writer->writeAttribute( "firstColor_r", QString::number(d->backgroundFirstColor.red()) );
    writer->writeAttribute( "firstColor_g", QString::number(d->backgroundFirstColor.green()) );
    writer->writeAttribute( "firstColor_b", QString::number(d->backgroundFirstColor.blue()) );
    writer->writeAttribute( "secondColor_r", QString::number(d->backgroundSecondColor.red()) );
    writer->writeAttribute( "secondColor_g", QString::number(d->backgroundSecondColor.green()) );
    writer->writeAttribute( "secondColor_b", QString::number(d->backgroundSecondColor.blue()) );
    writer->writeAttribute( "fileName", d->backgroundFileName );
    writer->writeAttribute( "opacity", QString::number(d->backgroundOpacity) );
    writer->writeEndElement();

    //serialize all children
    QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
    foreach(AbstractWorksheetElement *elem, childElements)
        elem->save(writer);

    writer->writeEndElement(); // close "worksheet" section
}

//! Load from XML
bool Worksheet::load(XmlStreamReader* reader){
    if(!reader->isStartElement() || reader->name() != "worksheet"){
        reader->raiseError(i18n("no worksheet element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;
    QRectF rect;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "worksheet")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
        }else if (reader->name() == "geometry"){
            attribs = reader->attributes();

            str = attribs.value("x").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'x'"));
            else
                rect.setX(str.toDouble());

            str = attribs.value("y").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'y'"));
            else
                rect.setY(str.toDouble());

            str = attribs.value("width").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'width'"));
            else
                rect.setWidth(str.toDouble());

            str = attribs.value("height").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'height'"));
            else
                rect.setHeight(str.toDouble());
        }else if (reader->name() == "layout"){
            attribs = reader->attributes();

            str = attribs.value("layout").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("layout"));
            else
                d->layout = Worksheet::Layout(str.toInt());

            str = attribs.value("topMargin").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("topMargin"));
            else
                d->layoutTopMargin = str.toDouble();

            str = attribs.value("bottomMargin").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("bottomMargin"));
            else
                d->layoutBottomMargin = str.toDouble();

            str = attribs.value("leftMargin").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("leftMargin"));
            else
                d->layoutLeftMargin = str.toDouble();

            str = attribs.value("rightMargin").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("rightMargin"));
            else
                d->layoutRightMargin = str.toDouble();

            str = attribs.value("verticalSpacing").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("verticalSpacing"));
            else
                d->layoutVerticalSpacing = str.toDouble();

            str = attribs.value("horizontalSpacing").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("horizontalSpacing"));
            else
                d->layoutHorizontalSpacing = str.toDouble();

            str = attribs.value("columnCount").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("columnCount"));
            else
                d->layoutColumnCount = str.toInt();

            str = attribs.value("rowCount").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("rowCount"));
            else
                d->layoutRowCount = str.toInt();
        }else if (reader->name() == "background"){
            attribs = reader->attributes();

            str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("type"));
            else
                d->backgroundType = PlotArea::BackgroundType(str.toInt());

            str = attribs.value("colorStyle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("colorStyle"));
            else
                d->backgroundColorStyle = PlotArea::BackgroundColorStyle(str.toInt());

            str = attribs.value("imageStyle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("imageStyle"));
            else
                d->backgroundImageStyle = PlotArea::BackgroundImageStyle(str.toInt());

            str = attribs.value("brushStyle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("brushStyle"));
            else
                d->backgroundBrushStyle = Qt::BrushStyle(str.toInt());

            str = attribs.value("firstColor_r").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("firstColor_r"));
            else
                d->backgroundFirstColor.setRed(str.toInt());

            str = attribs.value("firstColor_g").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("firstColor_g"));
            else
                d->backgroundFirstColor.setGreen(str.toInt());

            str = attribs.value("firstColor_b").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("firstColor_b"));
            else
                d->backgroundFirstColor.setBlue(str.toInt());

            str = attribs.value("secondColor_r").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("secondColor_r"));
            else
                d->backgroundSecondColor.setRed(str.toInt());

            str = attribs.value("secondColor_g").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("secondColor_g"));
            else
                d->backgroundSecondColor.setGreen(str.toInt());

            str = attribs.value("secondColor_b").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("secondColor_b"));
            else
                d->backgroundSecondColor.setBlue(str.toInt());

            str = attribs.value("fileName").toString();
            d->backgroundFileName = str;

            str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("opacity"));
            else
                d->backgroundOpacity = str.toDouble();
        }else if(reader->name() == "cartesianPlot"){
            CartesianPlot* plot = new CartesianPlot("");
            if (!plot->load(reader)){
                delete plot;
                return false;
            }else{
                addChild(plot);
            }
        }else if(reader->name() == "textLabel"){
            TextLabel* label = new TextLabel("");
            if (!label->load(reader)){
                delete label;
                return false;
            }else{
                addChild(label);
            }
        }else{ // unknown element
            reader->raiseWarning(i18n("unknown element '%1'").arg(reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    d->m_scene->setSceneRect(rect);
	d->updateLayout();

    return true;
}
