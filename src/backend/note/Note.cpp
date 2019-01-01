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

#include "Note.h"
#include "commonfrontend/note/NoteView.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#include <QPalette>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

Note::Note(const QString& name) : AbstractPart(name) {
	KConfig config;
	KConfigGroup group = config.group("Notes");

	m_backgroundColor = group.readEntry("BackgroundColor", QColor(Qt::yellow));
	m_textColor = group.readEntry("TextColor", QColor(Qt::black));
	m_textFont = group.readEntry("TextFont", QFont());
}

QIcon Note::icon() const {
	return QIcon::fromTheme("document-new");
}

bool Note::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18nc("@title:window", "Print Worksheet"));
	bool ret;
	if ( (ret = (dlg->exec() == QDialog::Accepted)) )
		m_view->print(&printer);

	delete dlg;
	return ret;
}

bool Note::printPreview() const {
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &NoteView::print);
	return dlg->exec();
}

bool Note::exportView() const {
	return false;
}

void Note::setNote(const QString& note) {
	m_note = note;
}

const QString& Note::note() const {
	return m_note;
}

void Note::setBackgroundColor(const QColor& color) {
	m_backgroundColor = color;
	emit backgroundColorChanged(color);
}

const QColor& Note::backgroundColor() const {
	return m_backgroundColor;
}

void Note::setTextColor(const QColor& color) {
	m_textColor = color;
	emit textColorChanged(color);
}

const QColor& Note::textColor() const{
	return m_textColor;
}

void Note::setTextFont(const QFont& font) {
	m_textFont = font;
	emit textFontChanged(font);
}

const QFont& Note::textFont() const {
	return m_textFont;
}

QWidget* Note::view() const {
	if (!m_partView) {
		m_view = new NoteView(const_cast<Note*>(this));
		m_partView = m_view;
	}
	return m_partView;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Note::save(QXmlStreamWriter* writer) const {
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

bool Note::load(XmlStreamReader* reader, bool preview) {
	if (!reader->isStartElement() || reader->name() != "note") {
		reader->raiseError(i18n("no note element found"));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
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
		} else if (!preview && reader->name() == "background") {
			attribs = reader->attributes();
			READ_QCOLOR(m_backgroundColor);
		} else if (!preview && reader->name() == "text") {
			attribs = reader->attributes();
			READ_QCOLOR(m_textColor);
			READ_QFONT(m_textFont);
			m_note = attribs.value("text").toString();
		}
	}

	return true;
}
