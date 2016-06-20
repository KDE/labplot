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

#include <QGraphicsItem>
#include <QColorDialog>

NotesView::NotesView() {
	
	m_textEdit = new QTextEdit(this);
	
// 	QColor color = QColorDialog::getColor(Qt::yellow,this); // can be used to give options
	
	m_palette = m_textEdit->palette();
	
	m_palette.setColor(QPalette::Base, Qt::yellow); // set color "Red" for textedit base
	m_palette.setColor(QPalette::Text, Qt::black);
	
	m_textEdit->setPalette(m_palette);
	
	setCentralWidget(m_textEdit);
}

NotesView::~NotesView() {

}

void NotesView::changeBgColor(QColor color) {
	m_palette.setColor(QPalette::Base, color);
	m_textEdit->setPalette(m_palette);
}

void NotesView::changeTextColor(QColor color) {
	m_palette.setColor(QPalette::Text, color);
	m_textEdit->setPalette(m_palette);
}
