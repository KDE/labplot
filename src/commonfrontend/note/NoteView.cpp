/*
	File                 : NotesView.cpp
	Project              : LabPlot
	Description          : Notes View for taking notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2016 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NoteView.h"
#include "backend/note/Note.h"

#include <QHBoxLayout>
#include <QPrinter>
#include <QTextEdit>

NoteView::NoteView(Note* note)
	: m_note(note)
	, m_textEdit(new QTextEdit(this)) {
	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	QPalette palette = m_textEdit->palette();
	palette.setColor(QPalette::Base, m_note->backgroundColor());
	palette.setColor(QPalette::Text, m_note->textColor());

	m_textEdit->setPalette(palette);
	m_textEdit->setFont(m_note->textFont());
	m_textEdit->setText(m_note->note());

	layout->addWidget(m_textEdit);

	connect(m_note, &Note::backgroundColorChanged, this, &NoteView::backgroundColorChanged);
	connect(m_note, &Note::textColorChanged, this, &NoteView::textColorChanged);
	connect(m_note, &Note::textFontChanged, this, &NoteView::textFontChanged);
	connect(m_textEdit, &QTextEdit::textChanged, this, &NoteView::textChanged);
}

void NoteView::print(QPrinter* printer) const {
	m_textEdit->print(printer);
}

void NoteView::textChanged() {
	m_note->setNote(m_textEdit->toPlainText());
}

void NoteView::backgroundColorChanged(QColor color) {
	QString red = QString::number(color.red());
	QString green = QString::number(color.green());
	QString blue = QString::number(color.blue());
	m_textEdit->setStyleSheet(QStringLiteral("QTextEdit{background-color: rgb(%1, %2, %3);}").arg(red, green, blue));
}

void NoteView::textFontChanged(const QFont& font) {
	m_textEdit->setFont(font);
}

void NoteView::textColorChanged(QColor color) {
	m_textEdit->setTextColor(color);
}
