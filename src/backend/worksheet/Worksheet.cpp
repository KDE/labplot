/***************************************************************************
    File                 : Worksheet.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet (2D visualization) part
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
	Copyright            : (C) 2011 by Alexander Semke (alexander.semke*web.de)
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

#include "worksheet/Worksheet.h"
#include "worksheet/WorksheetPrivate.h"
#include "worksheet/AbstractWorksheetElement.h"
#include "worksheet/WorksheetView.h"
#include "worksheet/WorksheetGraphicsScene.h"
#include "lib/commandtemplates.h"
#include "lib/macros.h"

#include <QWidget>

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#else
#include "KIcon"
#include <kdebug.h>
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
	KConfig config;
	KConfigGroup group = config.group( "Worksheet" );
	d->backgroundType = (PlotArea::BackgroundType) group.readEntry("BackgroundType", 0);
	d->backgroundColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", 0);
	d->backgroundImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", 0);
	d->backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	d->backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());
	d->backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);
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
		return value*25.4*10;
	case Worksheet::Point:
		return value*25.4/72*10;
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
		return value/2.54;
	case Worksheet::Point:
		return value/2.54*72;
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
}

void Worksheet::handleAspectAboutToBeRemoved(const AbstractAspect *aspect) {
	const AbstractWorksheetElement *removedElement = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (removedElement) {
		QGraphicsItem *item = removedElement->graphicsItem();
		Q_ASSERT(item != NULL);
		d->m_scene->removeItem(item);
	}
}

WorksheetGraphicsScene *Worksheet::scene() const {
	return d->m_scene;
}


QRectF Worksheet::pageRect() const {
	return d->m_scene->sceneRect();
}

void Worksheet::childSelected(){
	AbstractWorksheetElement* element=qobject_cast<AbstractWorksheetElement*>(QObject::sender());
	if (element)
		emit itemSelected(element->graphicsItem());
}

void Worksheet::update(){
	emit requestUpdate();
}

/* ============================ getter methods for background options ================= */
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundType, backgroundType, backgroundType);
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle);
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle);
CLASS_D_READER_IMPL(Worksheet, QBrush, backgroundBrush, backgroundBrush);
CLASS_D_READER_IMPL(Worksheet, QColor, backgroundFirstColor, backgroundFirstColor);
CLASS_D_READER_IMPL(Worksheet, QColor, backgroundSecondColor, backgroundSecondColor);
CLASS_D_READER_IMPL(Worksheet, QString, backgroundFileName, backgroundFileName);
BASIC_D_READER_IMPL(Worksheet, qreal, backgroundOpacity, backgroundOpacity);


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


STD_SWAP_METHOD_SETTER_CMD_IMPL(Worksheet, SetPageRect, QRectF, swapPageRect);
void Worksheet::setPageRect(const QRectF &rect, bool scaleContent) {
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
