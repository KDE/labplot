/***************************************************************************
    File                 : Worksheet.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet (2D visualization) part
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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
#include <QIcon>
#include <QWidget>
/**
 * \class Worksheet
 * \brief The plotting part. 
 *
 * Top-level container for WorksheetElements. 
 *
 */
		
Worksheet::Worksheet(AbstractScriptingEngine *engine, const QString &name)
		: AbstractPart(name), scripted(engine), m_view(NULL) {
	m_scene = new WorksheetGraphicsScene();
	m_scene->setSceneRect(0, 0, 210, 297); // A4  // TODO make this variable
	connect(this, SIGNAL(aspectAdded(const AbstractAspect*)),
		this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(this, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
		this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
}

Worksheet::~Worksheet() {
	delete m_scene;
}

//! Return an icon to be used for decorating my views.
QIcon Worksheet::icon() const {
	QIcon ico;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	ico.addPixmap(QPixmap(":/graph.xpm"));
#else
	ico = KIcon(QIcon(worksheet_xpm));
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
	// TODO
	return NULL;
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
	const AbstractWorksheetElement *elem = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (elem) {
		elem->retransform();
		QList<QGraphicsItem *> itemList = elem->graphicsItems();
		foreach(QGraphicsItem *item, itemList)
			m_scene->addItem(item);
	}
}

void Worksheet::handleAspectAboutToBeRemoved(const AbstractAspect *aspect) {
	const AbstractWorksheetElement *elem = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (elem) {
		QList<QGraphicsItem *> itemList = elem->graphicsItems();
		foreach(QGraphicsItem *item, itemList)
			m_scene->removeItem(item);
	}
}

WorksheetGraphicsScene *Worksheet::scene() const {
	return m_scene;
}

