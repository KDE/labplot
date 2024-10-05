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
#include "frontend/dockwidgets/BaseDock.h"

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
	m_textEdit->setText(m_note->text());

	layout->addWidget(m_textEdit);

	connect(m_note, &Note::textChanged, this, &NoteView::noteTextChanged);
	connect(m_note, &Note::backgroundColorChanged, this, &NoteView::noteBackgroundColorChanged);
	connect(m_note, &Note::textColorChanged, this, &NoteView::noteTextColorChanged);
	connect(m_note, &Note::textFontChanged, this, &NoteView::noteTextFontChanged);

	connect(m_textEdit, &QTextEdit::textChanged, this, &NoteView::textChanged);
}

void NoteView::print(QPrinter* printer) const {
	m_textEdit->print(printer);
}

void NoteView::textChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_note->setText(m_textEdit->toPlainText());
}

void NoteView::noteTextChanged(const QString& text) {
	m_textEdit->setText(text);
}

void NoteView::noteBackgroundColorChanged(const QColor& color) {
	QString red = QString::number(color.red());
	QString green = QString::number(color.green());
	QString blue = QString::number(color.blue());
	m_textEdit->setStyleSheet(QStringLiteral("QTextEdit{background-color: rgb(%1, %2, %3);}").arg(red, green, blue));
}

void NoteView::noteTextFontChanged(const QFont& font) {
	m_textEdit->setFont(font);
}

void NoteView::noteTextColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	m_textEdit->setTextColor(color);
}
