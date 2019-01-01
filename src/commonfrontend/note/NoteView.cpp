/***************************************************************************
    File                 : NotesView.cpp
    Project              : LabPlot
    Description          : Notes View for taking notes
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2016 Garvit Khatri (garvitdelhi@gmail.com)
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
