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

#include "Worksheet.h"
#include <QIcon>
/**
 * \class Worksheet
 * \brief The plotting part. 
 *
 * Top-level container for WorksheetElements. 
 *
 */
		
Worksheet::Worksheet(AbstractScriptingEngine *engine, const QString &name)
	: AbstractPart(name), scripted(engine)
{
}

Worksheet::~Worksheet() {
}

//! Return an icon to be used for decorating my views.
QIcon Worksheet::icon() const {
	// TODO
}

//! Fill the part specific menu for the main window including setting the title
/**
 * \return true on success, otherwise false (e.g. part has no actions).
 */
bool Worksheet::fillProjectMenu(QMenu * menu) {
	// TODO
}

//! Return a new context menu.
/**
 * The caller takes ownership of the menu.
 */
QMenu *Worksheet::createContextMenu() {
	// TODO
}

//! Construct a primary view on me.
/**
 * This method may be called multiple times during the life time of an Aspect, or it might not get
 * called at all. Aspects must not depend on the existence of a view for their operation.
 */
QWidget *Worksheet::view() const {
	// TODO
}

//! Save as XML
void Worksheet::save(QXmlStreamWriter *) const {
	// TODO
}

//! Load from XML
bool Worksheet::load(XmlStreamReader *) {
	// TODO
}
