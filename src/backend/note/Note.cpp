/*
	File                 : Notes.cpp
	Project              : LabPlot
	Description          : Notes Widget for taking notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2015 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Note.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "frontend/note/NoteView.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPalette>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QTextStream>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

Note::Note(const QString& name)
	: AbstractPart(name, AspectType::Note) {
	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("Notes"));

	m_backgroundColor = group.readEntry(QStringLiteral("BackgroundColor"), QColor(Qt::yellow));
	m_textColor = group.readEntry(QStringLiteral("TextColor"), QColor(Qt::black));
	m_textFont = group.readEntry(QStringLiteral("TextFont"), QFont());
}

QIcon Note::icon() const {
	return QIcon::fromTheme(QStringLiteral("document-new"));
}

bool Note::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18nc("@title:window", "Print Note"));
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
	KConfigGroup conf = Settings::group(QStringLiteral("ExportNote"));
	QString dir = conf.readEntry("LastDir", "");
	QString extensions = i18n("Text file (*.txt)");

	const QString path = QFileDialog::getSaveFileName(view(), i18nc("@title:window", "Export to File"), dir, extensions);

	if (path.isEmpty())
		return false;

	int pos = path.lastIndexOf(QStringLiteral("/"));
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry(QStringLiteral("LastDir"), newDir);
	}

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(view(), i18n("Export failed"), i18n("Failed to open '%1' for writing.", path));
		return false;
	}

	QTextStream out(&file);
	out << m_text;
	file.close();

	return true;
}

void Note::setText(const QString& text) {
	m_text = text;
	setProjectChanged(true);
	Q_EMIT textChanged(text);
}

const QString& Note::text() const {
	return m_text;
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

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
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
	writer->writeAttribute(QStringLiteral("text"), m_text);
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
			m_text = attribs.value(QStringLiteral("text")).toString();
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}
