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
	m_textEdit->setText(m_note->text());

	layout->addWidget(m_textEdit);

	connect(m_note, &Note::textChanged, this, &NoteView::noteTextChanged);
	connect(m_note, &Note::backgroundColorChanged, this, &NoteView::noteBackgroundColorChanged);
	connect(m_note, &Note::textColorChanged, this, &NoteView::noteTextColorChanged);
	connect(m_note, &Note::textFontChanged, this, &NoteView::noteTextFontChanged);

	// don't put every single character change onto the undo stack, delay it similarly how it's done in TimedLineEdit
	connect(m_textEdit, &QTextEdit::textChanged, [&]() {
		if (m_textChangedTimerId != -1)
			killTimer(m_textChangedTimerId);
		m_textChangedTimerId = startTimer(1000);
	});
}

void NoteView::print(QPrinter* printer) const {
	m_textEdit->print(printer);
}

void NoteView::timerEvent(QTimerEvent* event) {
	if (event->timerId() == m_textChangedTimerId) {
		killTimer(m_textChangedTimerId);
		m_textChangedTimerId = -1;
		CONDITIONAL_LOCK_RETURN;
		m_note->setText(m_textEdit->toPlainText());
	}
}

//*************************************************************
//************ SLOTs for changes triggered in Note ************
//*************************************************************
void NoteView::noteTextChanged(const QString& text) {
	auto cursor = m_textEdit->textCursor();
	int cursorAnchor = cursor.anchor();
	int cursorPos = cursor.position();

	CONDITIONAL_LOCK_RETURN;
	m_textEdit->setText(text);

	cursor = m_textEdit->textCursor();
	cursor.setPosition(cursorAnchor);
	cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, cursorPos - cursorAnchor);
	m_textEdit->setTextCursor(cursor);
}

void NoteView::noteBackgroundColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	QString red = QString::number(color.red());
	QString green = QString::number(color.green());
	QString blue = QString::number(color.blue());
	m_textEdit->setStyleSheet(QStringLiteral("QTextEdit{background-color: rgb(%1, %2, %3);}").arg(red, green, blue));
}

void NoteView::noteTextFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	m_textEdit->setFont(font);
}

void NoteView::noteTextColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	m_textEdit->selectAll();
	m_textEdit->setTextColor(color);
	auto cursor = m_textEdit->textCursor();
	cursor.clearSelection();
	m_textEdit->setTextCursor(cursor);
}
