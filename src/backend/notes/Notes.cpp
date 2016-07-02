/***************************************************************************
    File                 : Notes.cpp
    Project              : LabPlot
    Description          : Notes Widget for taking notes
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2015 Garvit Khatri (garvitdelhi@gmail.com)
    Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)

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
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#include <QPalette>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

Notes::Notes(const QString& name) : AbstractPart(name) {
	KConfig config;
	KConfigGroup group = config.group("Notes");

	m_backgroundColor = group.readEntry("BackgroundColor", QColor(Qt::yellow));
	m_textColor = group.readEntry("TextColor", QColor(Qt::black));
	m_textFont = group.readEntry("TextFont", QFont());
}

QIcon Notes::icon() const {
	return QIcon::fromTheme("document-new");
}

bool Notes::printView() {
	QPrinter printer;
	QPrintDialog* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18n("Print Worksheet"));
	bool ret;
    if ((ret = dlg->exec() == QDialog::Accepted)) {
		NotesView* view = reinterpret_cast<NotesView*>(m_view);
		view->print(&printer);
	}
	delete dlg;
	return ret;
}

bool Notes::printPreview() const {
	const NotesView* view = reinterpret_cast<const NotesView*>(m_view);
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, SIGNAL(paintRequested(QPrinter*)), view, SLOT(print(QPrinter*)));
	return dlg->exec();
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
	if (!m_view)
		m_view = new NotesView(const_cast<Notes*>(this));

	return m_view;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Notes::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("note");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement("background");
	WRITE_QCOLOR(m_backgroundColor);
	writer->writeEndElement();

	writer->writeStartElement("text");
	WRITE_QCOLOR(m_textColor);
	WRITE_QFONT(m_textFont);
	writer->writeAttribute("text", m_note);
	writer->writeEndElement();

	writer->writeEndElement(); // close "note" section
}

bool Notes::load(XmlStreamReader* reader) {
	if (!reader->isStartElement() || reader->name() != "note") {
		reader->raiseError(i18n("no note element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "note")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == "background") {
			attribs = reader->attributes();
			READ_QCOLOR(m_backgroundColor);
		} else if (reader->name() == "text") {
			attribs = reader->attributes();
			READ_QCOLOR(m_textColor);
			READ_QFONT(m_textFont);
			m_note = attribs.value("text").toString();
		}
	}
	
	return true;
}
