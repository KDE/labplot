/*
	File                 : Notes.cpp
	Project              : LabPlot
	Description          : Notes Widget for taking notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2015 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Note.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#ifndef SDK
#include "frontend/note/NoteView.h"
#endif

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

class NotePrivate {
public:
	explicit NotePrivate(Note* owner)
		: q(owner) {
	}

	QString name() const {
		return q->name();
	}

	QColor backgroundColor;
	QColor textColor;
	QFont textFont;
	QString text;
	Note* const q{nullptr};
};

Note::Note(const QString& name)
	: AbstractPart(name, AspectType::Note)
	, d_ptr(new NotePrivate(this)) {
	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("Notes"));

	Q_D(Note);
	d->backgroundColor = group.readEntry(QStringLiteral("BackgroundColor"), QColor(Qt::yellow));
	d->textColor = group.readEntry(QStringLiteral("TextColor"), QColor(Qt::black));
	d->textFont = group.readEntry(QStringLiteral("TextFont"), QFont());
}

QIcon Note::icon() const {
	return QIcon::fromTheme(QStringLiteral("document-new"));
}

bool Note::printView() {
#ifndef SDK
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18nc("@title:window", "Print Note"));
	bool ret;
	if ((ret = (dlg->exec() == QDialog::Accepted)))
		m_view->print(&printer);

	delete dlg;
	return ret;
#else
	return false;
#endif
}

bool Note::printPreview() const {
#ifndef SDK
	auto* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &NoteView::print);
	return dlg->exec();
#else
	return false;
#endif
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

	Q_D(const Note);
	QTextStream out(&file);
	out << d->text;
	file.close();

	return true;
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(Note, QString, text, text)
BASIC_SHARED_D_READER_IMPL(Note, QColor, backgroundColor, backgroundColor)
BASIC_SHARED_D_READER_IMPL(Note, QColor, textColor, textColor)
BASIC_SHARED_D_READER_IMPL(Note, QFont, textFont, textFont)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_S(Note, SetText, QString, text)
void Note::setText(const QString& text) {
	Q_D(Note);
	if (text != d->text)
		exec(new NoteSetTextCmd(d, text, ki18n("%1: set text")));
}

STD_SETTER_CMD_IMPL_S(Note, SetBackgroundColor, QColor, backgroundColor)
void Note::setBackgroundColor(const QColor& color) {
	Q_D(Note);
	if (color != d->backgroundColor)
		exec(new NoteSetBackgroundColorCmd(d, color, ki18n("%1: set background color")));
}

STD_SETTER_CMD_IMPL_S(Note, SetTextColor, QColor, textColor)
void Note::setTextColor(const QColor& color) {
	Q_D(Note);
	if (color != d->textColor)
		exec(new NoteSetTextColorCmd(d, color, ki18n("%1: set text color")));
}

STD_SETTER_CMD_IMPL_S(Note, SetTextFont, QFont, textFont)
void Note::setTextFont(const QFont& font) {
	Q_D(Note);
	if (font != d->textFont)
		exec(new NoteSetTextFontCmd(d, font, ki18n("%1: set text font")));
}

QWidget* Note::view() const {
#ifndef SDK
	if (!m_partView) {
		m_view = new NoteView(const_cast<Note*>(this));
		m_partView = m_view;
	}
	return m_partView;
#else
	return nullptr;
#endif
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Note::save(QXmlStreamWriter* writer) const {
	Q_D(const Note);
	writer->writeStartElement(QStringLiteral("note"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement(QStringLiteral("background"));
	WRITE_QCOLOR(d->backgroundColor);
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("text"));
	WRITE_QCOLOR(d->textColor);
	WRITE_QFONT(d->textFont);
	writer->writeAttribute(QStringLiteral("text"), d->text);
	writer->writeEndElement();

	writer->writeEndElement(); // close "note" section
}

bool Note::load(XmlStreamReader* reader, bool preview) {
	Q_D(Note);

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
			READ_QCOLOR(d->backgroundColor);
		} else if (!preview && reader->name() == QLatin1String("text")) {
			attribs = reader->attributes();
			READ_QCOLOR(d->textColor);
			READ_QFONT(d->textFont);
			d->text = attribs.value(QStringLiteral("text")).toString();
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}
