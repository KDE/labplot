/*
	File                 : Notes.cpp
	Project              : LabPlot
	Description          : Notes Widget for taking notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2015 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Note.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "commonfrontend/note/NoteView.h"

#include <QPalette>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

Note::Note(const QString& name)
	: AbstractPart(name, AspectType::Note) {
	KConfig config;
	KConfigGroup group = config.group("Notes");

	m_backgroundColor = group.readEntry("BackgroundColor", QColor(Qt::yellow));
	m_textColor = group.readEntry("TextColor", QColor(Qt::black));
	m_textFont = group.readEntry("TextFont", QFont());
}

QIcon Note::icon() const {
	return QIcon::fromTheme(QStringLiteral("document-new"));
}

bool Note::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18nc("@title:window", "Print Worksheet"));
	bool ret;
	if ((ret = (dlg->exec() == QDialog::Accepted)))
		m_view->print(&printer);

	delete dlg;
	return ret;
}

bool Note::printPreview() const {
	auto* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &NoteView::print);
	return dlg->exec();
}

bool Note::exportView() const {
	return false;
}

void Note::setNote(const QString& note) {
	m_note = note;
	project()->setChanged(true);
}

const QString& Note::note() const {
	return m_note;
}

void Note::setBackgroundColor(const QColor& color) {
	m_backgroundColor = color;
	Q_EMIT backgroundColorChanged(color);
}

const QColor& Note::backgroundColor() const {
	return m_backgroundColor;
}

void Note::setTextColor(const QColor& color) {
	m_textColor = color;
	Q_EMIT textColorChanged(color);
}

const QColor& Note::textColor() const {
	return m_textColor;
}

void Note::setTextFont(const QFont& font) {
	m_textFont = font;
	Q_EMIT textFontChanged(font);
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
	writer->writeStartElement(QStringLiteral("note"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement(QStringLiteral("background"));
	WRITE_QCOLOR(m_backgroundColor);
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("text"));
	WRITE_QCOLOR(m_textColor);
	WRITE_QFONT(m_textFont);
	writer->writeAttribute(QStringLiteral("text"), m_note);
	writer->writeEndElement();

	writer->writeEndElement(); // close "note" section
}

bool Note::load(XmlStreamReader* reader, bool preview) {
	if (!reader->isStartElement() || reader->name() != QLatin1String("note")) {
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
		if (reader->isEndElement() && reader->name() == QLatin1String("note"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("background")) {
			attribs = reader->attributes();
			READ_QCOLOR(m_backgroundColor);
		} else if (!preview && reader->name() == QLatin1String("text")) {
			attribs = reader->attributes();
			READ_QCOLOR(m_textColor);
			READ_QFONT(m_textFont);
			m_note = attribs.value(QStringLiteral("text")).toString();
		}
	}

	return true;
}
