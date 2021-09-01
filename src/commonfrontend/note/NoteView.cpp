/*
    File                 : NotesView.cpp
    Project              : LabPlot
    Description          : Notes View for taking notes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2016 Garvit Khatri (garvitdelhi@gmail.com)
    SPDX-FileCopyrightText: 2016 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "NoteView.h"
#include "backend/note/Note.h"

#include <QHBoxLayout>
#include <QPrinter>
#include <QTextEdit>

NoteView::NoteView(Note* notes) : m_notes(notes) {
	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_textEdit = new QTextEdit(this);

	QPalette palette = m_textEdit->palette();

	palette.setColor(QPalette::Base, m_notes->backgroundColor());
	palette.setColor(QPalette::Text, m_notes->textColor());

	m_textEdit->setPalette(palette);
	m_textEdit->setFont(m_notes->textFont());
	m_textEdit->setText(m_notes->note());

	layout->addWidget(m_textEdit);

	connect(m_notes, &Note::backgroundColorChanged, this, &NoteView::backgroundColorChanged);
	connect(m_notes, &Note::textColorChanged, this, &NoteView::textColorChanged);
	connect(m_notes, &Note::textFontChanged, this, &NoteView::textFontChanged);
	connect(m_textEdit, &QTextEdit::textChanged, this, &NoteView::textChanged);
}

void NoteView::print(QPrinter* printer) const {
	m_textEdit->print(printer);
}

void NoteView::textChanged() {
	m_notes->setNote(m_textEdit->toPlainText());
}

void NoteView::backgroundColorChanged(QColor color) {
	QPalette palette = m_textEdit->palette();
	palette.setColor(QPalette::Base, color);
	m_textEdit->setPalette(palette);
}

void NoteView::textFontChanged(const QFont& font) {
	m_textEdit->setFont(font);
}

void NoteView::textColorChanged(QColor color) {
	QPalette palette = m_textEdit->palette();
	palette.setColor(QPalette::Text, color);
	m_textEdit->setPalette(palette);
}
