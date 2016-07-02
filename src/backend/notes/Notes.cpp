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
#include "backend/lib/macros.h"
#include "commonfrontend/notes/NotesView.h"

#include <QPalette>
#include <KConfig>
#include <KConfigGroup>

Notes::Notes(const QString& name): AbstractPart(name) {
	init();
}

void Notes::init() {
	KConfig config;
	KConfigGroup group = config.group("Notes");

	m_backgroundColor = group.readEntry("BackgroundColor", QColor(Qt::yellow));
	m_textColor = group.readEntry("TextColor", QColor(Qt::black));
	m_textFont = group.readEntry("TextFont", QFont());
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

bool Notes::printPreview() const {
	return false;
}

bool Notes::printView() {
	return false;
}

bool Notes::exportView() const {
	return false;
}

void Notes::setNote(const QString& note) {
	m_note = note;
}

const QString& Notes::note() const {
	return m_note;
}

void Notes::setBackgroundColor(const QColor& color) {
	m_backgroundColor = color;
	emit (backgroundColorChanged(color));
}

const QColor& Notes::backgroundColor() const {
	return m_backgroundColor;
}

void Notes::setTextColor(const QColor& color) {
	m_textColor = color;
	emit (textColorChanged(color));
}

const QColor& Notes::textColor() const{
	return m_textColor;
}

void Notes::setTextFont(const QFont& font) {
	m_textFont = font;
	emit (textFontChanged(font));
}

const QFont& Notes::textFont() const {
	return m_textFont;
}

QWidget* Notes::view() const {
	if (!m_view) {
		m_view = new NotesView(const_cast<Notes*>(this));
// 		m_view->setBaseSize(1500, 1500);
		// 	connect(m_view, SIGNAL(statusInfo(QString)), this, SIGNAL(statusInfo(QString)));
	}
	return m_view;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Notes::save(QXmlStreamWriter*) const {
	
}

bool Notes::load(XmlStreamReader*) {
	return false;
}
