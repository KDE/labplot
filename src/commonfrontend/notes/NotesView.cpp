/***************************************************************************
    File                 : NotesView.cpp
    Project              : LabPlot
    Description          : Notes View for taking notes
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2016 Garvit Khatri (garvitdelhi@gmail.com)

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

#include "NotesView.h"

#include <QHBoxLayout>
#include <QColorDialog>

#include "backend/notes/Notes.h"

NotesView::NotesView(Notes* notes) : m_notes(notes) {

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	
	m_textEdit = new QTextEdit(this);

	QPalette palette = m_textEdit->palette();

	palette.setColor(QPalette::Base, m_notes->bgColor());
	palette.setColor(QPalette::Text, m_notes->textColor());

	m_textEdit->setPalette(palette);
	m_textEdit->setText(m_notes->note());

	layout->addWidget(m_textEdit);
	
	connect(m_notes, SIGNAL(bgColorChanged(QColor)), this, SLOT(bgColorChanged(QColor)));
	connect(m_notes, SIGNAL(textColorChanged(QColor)), this, SLOT(textColorChanged(QColor)));
	connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));
}

void NotesView::createContextMenu(QMenu* menu) const {

}

void NotesView::fillToolBar(QToolBar* toolbar) {

}

void NotesView::textChanged() {
	m_notes->setNote(m_textEdit->toPlainText());
}

void NotesView::bgColorChanged(QColor color) {
	QPalette palette = m_textEdit->palette();
	palette.setColor(QPalette::Base, color);
	m_textEdit->setPalette(palette);
}

void NotesView::textColorChanged(QColor color) {
	QPalette palette = m_textEdit->palette();
	palette.setColor(QPalette::Text, color);
	m_textEdit->setPalette(palette);
}

NotesView::~NotesView() {

}
