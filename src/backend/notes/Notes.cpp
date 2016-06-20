/***************************************************************************
    File                 : Notes.cpp
    Project              : LabPlot
    Description          : Notes Widget for taking notes
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2015 Garvit Khatri (garvitdelhi@gmail.com)

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

#include "Notes.h"


Notes::Notes(AbstractScriptingEngine* engine, const QString& name, bool loading): AbstractPart(name), scripted(engine) {
	m_notesView = new NotesView();
}

Notes::~Notes() {

}

bool Notes::exportView() const {
	return false;
}

QMenu* Notes::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	emit requestProjectContextMenu(menu);
	return menu;
}

QIcon Notes::icon() const {
	return QIcon::fromTheme("document-new");
}

bool Notes::load(XmlStreamReader*) {
	return false;
}

bool Notes::printPreview() const {
	return false;
}

bool Notes::printView() {
	return false;
}

void Notes::save(QXmlStreamWriter*) const {
	
}

void Notes::changeBgColor(QColor color) {
	m_notesView->changeBgColor(color);
}

void Notes::changeTextColor(QColor color) {
	m_notesView->changeTextColor(color);
}

QWidget* Notes::view() const {
	if (!m_view) {
		m_view = m_notesView;
// 		m_view->setBaseSize(1500, 1500);
		// 	connect(m_view, SIGNAL(statusInfo(QString)), this, SIGNAL(statusInfo(QString)));
	}
	return m_view;
}
