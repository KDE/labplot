/***************************************************************************
    File                 : CantorWorksheet.cpp
    Project              : LabPlot
    Description          : Aspect providing a Cantor Worksheets for Multiple backends
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)

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

#include "CantorWorksheet.h"
#include "commonfrontend/cantorWorksheet/CantorWorksheetView.h"
#include "backend/core/column/Column.h"

#include <QDebug>
#include <KLocalizedString>
#include <KMessageBox>
#include <cantor/backend.h>

CantorWorksheet::CantorWorksheet(AbstractScriptingEngine* engine, const QString &name)
		: AbstractPart(name), scripted(engine), backendName(name){
    initialize();
}

void CantorWorksheet::initialize() {
    KPluginFactory* factory = KPluginLoader(QLatin1String("libcantorpart")).factory();
    if (factory) {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        part = factory->create<KParts::ReadWritePart>(this, QVariantList()<<BackendName());
	if (!part) {
            qDebug()<<"error creating part ";
	    return;
        }
	Cantor::PanelPluginHandler* handler=part->findChild<Cantor::PanelPluginHandler*>(QLatin1String("PanelPluginHandler"));
	if(!handler) {
	    KMessageBox::error(view(), i18n("no PanelPluginHandle found for the Cantor Part."));
	    qApp->quit();
	}
	plugins = handler->plugins();
    }
    else {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(view(), i18n("Could not find the Cantor Part."));
        qApp->quit();
        // we return here, cause qApp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }
}

QList<Cantor::PanelPlugin*> CantorWorksheet::getPlugins(){
    return plugins;
}

KParts::ReadWritePart* CantorWorksheet::getPart() {
    if(part) return part;
    else return NULL;
}

QWidget* CantorWorksheet::view() const {
    if (!m_view) {
	m_view = new CantorWorksheetView(const_cast<CantorWorksheet*>(this));
// 	connect(m_view, SIGNAL(statusInfo(QString)), this, SIGNAL(statusInfo(QString)));
    }
    return m_view;
}

//! Return a new context menu.
/**
 * The caller takes ownership of the menu.
 */
QMenu* CantorWorksheet::createContextMenu() {
    QMenu* menu = AbstractPart::createContextMenu();
    Q_ASSERT(menu);
    emit requestProjectContextMenu(menu);
    return menu;
}

QString CantorWorksheet::BackendName() {
    return this->backendName;
}
