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
#endif
	
/**
 * \class Worksheet
 * \brief The plotting part. 
 *
 * Top-level container for WorksheetElements. 
 *
 */
	
class Worksheet::Private {
	public:

		Private(Worksheet *owner);
		~Private();

		QString name() const {
			return q->name();
		}

		WorksheetGraphicsScene *m_scene;
		QRectF swapPageRect(const QRectF& rect);

		Worksheet * const q;
};

Worksheet::Private::Private(Worksheet *owner) 
	: q(owner) {

	m_scene = new WorksheetGraphicsScene();
//  	m_scene->setSceneRect(0, 0, 210, 297); // A4
	m_scene->setSceneRect(0, 0, 150, 150); // A4
}
		
Worksheet::Private::~Private() {
	delete m_scene;
}

Worksheet::Worksheet(AbstractScriptingEngine *engine, const QString &name)
		: AbstractPart(name), scripted(engine), d(new Private(this)), m_view(NULL) {

	connect(this, SIGNAL(aspectAdded(const AbstractAspect*)),
		this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(this, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
		this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
}

Worksheet::~Worksheet() {
	delete d;
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
	if (!m_view) {
		m_view = new WorksheetView(const_cast<Worksheet *>(this));
		connect(m_view, SIGNAL(statusInfo(const QString&)), this, SIGNAL(statusInfo(const QString&)));
	}
	return m_view;
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

		if (aspect->parentAspect() == this)
		{
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

QRectF Worksheet::Private::swapPageRect(const QRectF &rect) {
	QRectF oldRect = m_scene->sceneRect();
	m_scene->setSceneRect(rect.normalized());

	return oldRect;
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

void Worksheet::childSelected(){
 AbstractWorksheetElement* element=qobject_cast<AbstractWorksheetElement*>(QObject::sender());
 if (element)
  emit itemSelected(element->graphicsItem());
}
